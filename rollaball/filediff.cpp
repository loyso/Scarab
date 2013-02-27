#include "filediff.h"

#include "filetree.h"

#include <dung/memoryblock.h>
#include <zlib/minizip.h>

#include <algorithm>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

extern "C"
{
#include <external/xdelta/xdelta3.h>
}

namespace rab
{
	typedef fs::path Path_t;

	bool EncodeAndWrite( dung::MemoryBlock const& newFile, dung::MemoryBlock const& oldFile, Path_t const& fullTemp,
		Path_t const& relativeTemp, PackageOutput_t& output );

	bool BuildDiffFile( Options const& options, Config const& config, DiffEncoders const& diffEncoders,
		Path_t const& relativePath, PackageOutput_t& output, FileInfo& fileInfo );

	void BuildDiffFiles( Options const& options, Config const& config, DiffEncoders const& diffEncoders,
		Path_t const& relativePath, PackageOutput_t& output, FolderInfo::FileInfos_t const& fileInfos );
	
	void BuildDiffFolders( Options const& options, Config const& config, DiffEncoders const& diffEncoders, 
		Path_t const& relativePath, PackageOutput_t& output, FolderInfo::FolderInfos_t const& folderInfos );
}

bool rab::EncodeAndWrite( dung::MemoryBlock const& newFile, dung::MemoryBlock const& oldFile, Path_t const& fullTemp,
	Path_t const& relativeTemp, PackageOutput_t& output )
{
	const size_t reservedSize = max( newFile.size, 1024 );
	dung::MemoryBlock deltaFile( reservedSize );
	deltaFile.size = 0;

	int ret = xd3_encode_memory( newFile.pBlock, newFile.size, oldFile.pBlock, oldFile.size, deltaFile.pBlock, &deltaFile.size, reservedSize, 0 );
	if( ret != 0 )
		return false;

	WriteWholeFile( fullTemp.wstring(), deltaFile );

	output.WriteFile( relativeTemp.generic_wstring(), deltaFile.pBlock, deltaFile.size );

	return true;
}

bool rab::BuildDiffFile( Options const& options, Config const& config, DiffEncoders const& diffEncoders,
	Path_t const& relativePath, PackageOutput_t& output, FileInfo& fileInfo )
{
	Path_t fullNew = options.pathToNew / relativePath / fileInfo.name;
	Path_t fullOld = options.pathToOld / relativePath / fileInfo.name;
	Path_t relativeTemp = relativePath / DiffFileName(fileInfo.name, config);
	Path_t fullTemp = options.pathToTemp / relativeTemp;

	dung::MemoryBlock oldFile;
	if( !ReadWholeFile( fullOld.wstring(), oldFile ) )
		return false;
	dung::SHA1Compute( oldFile.pBlock, oldFile.size, fileInfo.oldSha1 );
	fileInfo.oldSize = oldFile.size;

	if( MatchName( config.oldSkipChanged_regex, fileInfo.name ) )
		return true;

	dung::MemoryBlock newFile;
	if( !ReadWholeFile( fullNew.wstring(), newFile ) )
		return false;
	dung::SHA1Compute( newFile.pBlock, newFile.size, fileInfo.newSha1 );
	fileInfo.newSize = newFile.size;

	if( fileInfo.newSha1 != fileInfo.oldSha1 )
	{
		fileInfo.isDifferent = true;
		fs::create_directories( fullTemp.parent_path() );

 		dung::DiffEncoder_i* pEncoder = diffEncoders.FindEncoder( fileInfo.name );
		if( pEncoder != NULL )
		{
		}
		else
		{
			dung::DiffEncoderExternal_i* pExternalEncoder = diffEncoders.FindExternalEncoder( fileInfo.name );
			if( pExternalEncoder != NULL )
			{
				if( !pExternalEncoder->EncodeDiffFile( fullNew.generic_string().c_str(), fullOld.generic_string().c_str(), fullTemp.generic_string().c_str() ) )
				{
					char errorMessage[ 256 ];
					pExternalEncoder->GetErrorMessage( errorMessage, sizeof( errorMessage ) );
					return false;
				}

				dung::MemoryBlock deltaFile;
				if( !ReadWholeFile( fullTemp.generic_string(), deltaFile ) )
					return false;

				output.WriteFile( relativeTemp.generic_wstring(), deltaFile.pBlock, deltaFile.size );
			}
			else if( !EncodeAndWrite( newFile, oldFile, fullTemp, relativeTemp, output ) )
				return false;
		}
	}
	else
		fileInfo.isDifferent = false;

	return true;
}

void rab::BuildDiffFiles( Options const& options, Config const& config, DiffEncoders const& diffEncoders,
	Path_t const& relativePath, PackageOutput_t& output, FolderInfo::FileInfos_t const& fileInfos )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;

		BuildDiffFile( options, config, diffEncoders, relativePath, output, fileInfo );
	}
}

void rab::BuildDiffFolders( Options const& options, Config const& config, DiffEncoders const& diffEncoders, 
	Path_t const& relativePath, PackageOutput_t& output, FolderInfo::FolderInfos_t const& folderInfos )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;
		
		Path_t nextRelativePath = relativePath / folderInfo.name;

		BuildDiffFiles( options, config, diffEncoders, nextRelativePath, output, folderInfo.files_existInBoth );
		BuildDiffFolders( options, config, diffEncoders, nextRelativePath, output, folderInfo.folders_existInBoth );
	}
}

void rab::BuildDiffs( Options const& options, Config const& config, DiffEncoders const& diffEncoders, FolderInfo const& rootFolder, PackageOutput_t& output )
{
	Path_t relativePath;

	BuildDiffFiles( options, config, diffEncoders, relativePath, output, rootFolder.files_existInBoth );
	BuildDiffFolders( options, config, diffEncoders, relativePath, output, rootFolder.folders_existInBoth );
}


rab::DiffEncoders::DiffEncoders()
{
}

rab::DiffEncoders::~DiffEncoders()
{
	DeleteContainer( m_diffEncoders );
	DeleteContainer( m_diffEncodersExternal );
}

void rab::DiffEncoders::AddEncoder( dung::DiffEncoder_i& diffEncoder, const char* encoderName, Config::StringValues_t const& packFiles )
{
	EncoderEntry* pEncoderEntry = SCARAB_NEW EncoderEntry;

	pEncoderEntry->m_encoderName = encoderName;
	pEncoderEntry->m_pDiffEncoder = &diffEncoder;
	BuildRegexVector( packFiles, pEncoderEntry->m_packFiles );

	m_diffEncoders.push_back( pEncoderEntry );
}

void rab::DiffEncoders::AddExternalEncoder( dung::DiffEncoderExternal_i& diffEncoder, const char* encoderName, Config::StringValues_t const& packFiles )
{
	ExternalEncoderEntry* pEncoderEntry = SCARAB_NEW ExternalEncoderEntry;

	pEncoderEntry->m_encoderName = encoderName;
	pEncoderEntry->m_pDiffEncoder = &diffEncoder;
	BuildRegexVector( packFiles, pEncoderEntry->m_packFiles );

	m_diffEncodersExternal.push_back( pEncoderEntry );
}

dung::DiffEncoder_i* rab::DiffEncoders::FindEncoder( String_t const& fileName ) const
{
	for( DiffEncoders_t::const_iterator i = m_diffEncoders.begin(); i != m_diffEncoders.end(); ++i )
	{
		EncoderEntry const& encoderEntry = **i;
		if( MatchName( encoderEntry.m_packFiles, fileName ) )
			return encoderEntry.m_pDiffEncoder;
	}

	return NULL;
}

dung::DiffEncoderExternal_i* rab::DiffEncoders::FindExternalEncoder( String_t const& fileName ) const
{
	for( DiffExternalEncoders_t::const_iterator i = m_diffEncodersExternal.begin(); i != m_diffEncodersExternal.end(); ++i )
	{
		ExternalEncoderEntry const& encoderEntry = **i;
		if( MatchName( encoderEntry.m_packFiles, fileName ) )
			return encoderEntry.m_pDiffEncoder;
	}

	return NULL;
}
