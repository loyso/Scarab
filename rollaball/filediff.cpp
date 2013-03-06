#include "filediff.h"

#include "filetree.h"

#include <dung/memoryblock.h>
#include <dung/diffencoder.h>

#include <zlib/minizip.h>

#include <algorithm>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef fs::path Path_t;

	bool CreateDiffFile( DiffEncoders const &diffEncoders, FileInfo& fileInfo, Path_t const& fullNew, Path_t const& fullOld, Path_t const& fullTemp, Path_t const& relativeTemp, 
		dung::MemoryBlock const& newFile, dung::MemoryBlock const& oldFile, PackageOutput_t &package, LogOutput_t& out );

	bool BuildDiffFile( Options const& options, Config const& config, DiffEncoders const& diffEncoders,
		Path_t const& relativePath, PackageOutput_t& package, LogOutput_t& out, FileInfo& fileInfo );

	bool BuildDiffFiles( Options const& options, Config const& config, DiffEncoders const& diffEncoders,
		Path_t const& relativePath, PackageOutput_t& package, LogOutput_t& out, FolderInfo::FileInfos_t const& fileInfos );
	
	bool BuildDiffFolders( Options const& options, Config const& config, DiffEncoders const& diffEncoders, 
		Path_t const& relativePath, PackageOutput_t& package, LogOutput_t& out, FolderInfo::FolderInfos_t const& folderInfos );
}

bool rab::CreateDiffFile( DiffEncoders const &diffEncoders, FileInfo& fileInfo, Path_t const& fullNew, Path_t const& fullOld, Path_t const& fullTemp, Path_t const& relativeTemp, 
	dung::MemoryBlock const& newFile, dung::MemoryBlock const& oldFile, PackageOutput_t &package, LogOutput_t& out )
{
	dung::DiffEncoder_i* pEncoder = diffEncoders.FindEncoder( fileInfo.name, fileInfo.diffMethod );
	if( pEncoder != NULL )
	{		
		out << "Encoding " << fileInfo.diffMethod << " diff file " << fullTemp.generic_wstring() << std::endl;
		
		void* pDiffBlock = NULL;
		size_t diffSize = 0;
		if( !pEncoder->EncodeDiffMemoryBlock( newFile.pBlock, newFile.size, oldFile.pBlock, oldFile.size, pDiffBlock, diffSize ) )
		{
			char errorMessage[ 256 ];
			pEncoder->GetErrorMessage( errorMessage, sizeof( errorMessage ) );
			out << "Encoding error " << errorMessage << std::endl;
			return false;
		}

		dung::MemoryBlock deltaFile( pDiffBlock, diffSize );

		if( !WriteWholeFile( fullTemp.wstring(), deltaFile ) )
		{
			out << "Can't write file " << fullTemp.generic_wstring() << std::endl;
			return false;
		}

		if( !package.WriteFile( relativeTemp.generic_wstring(), deltaFile.pBlock, deltaFile.size ) )
		{
			out << "Can't write file " << relativeTemp.generic_wstring() << " to package. Size=" << deltaFile.size << std::endl;
			return false;
		}
	}
	else
	{
		dung::DiffEncoderExternal_i* pExternalEncoder = diffEncoders.FindExternalEncoder( fileInfo.name, fileInfo.diffMethod );
		if( pExternalEncoder != NULL )
		{
			out << "Encoding " << fileInfo.diffMethod << " diff file " << fullTemp.generic_wstring() << std::endl;
			if( !pExternalEncoder->EncodeDiffFile( fullNew.generic_string().c_str(), fullOld.generic_string().c_str(), fullTemp.generic_string().c_str() ) )
			{
				char errorMessage[ 256 ];
				pExternalEncoder->GetErrorMessage( errorMessage, sizeof( errorMessage ) );
				out << "Encoding error " << errorMessage << std::endl;
				return false;
			}

			dung::MemoryBlock deltaFile;
			if( !ReadWholeFile( fullTemp.generic_string(), deltaFile ) )
			{
				out << "Can't read file " << fullTemp.generic_wstring() << std::endl;
				return false;
			}

			if( !package.WriteFile( relativeTemp.generic_wstring(), deltaFile.pBlock, deltaFile.size ) )
			{
				out << "Can't write file " << relativeTemp.generic_wstring() << " to package. Size=" << deltaFile.size << std::endl;
				return false;
			}
		}
		else
		{
			out << "Can't file encoder for file " << fileInfo.name << std::endl;
			return false;
		}				
	}

	return true;
}

bool rab::BuildDiffFile( Options const& options, Config const& config, DiffEncoders const& diffEncoders,
	Path_t const& relativePath, PackageOutput_t& package, LogOutput_t& out, FileInfo& fileInfo )
{
	Path_t fullNew = options.pathToNew / relativePath / fileInfo.name;
	Path_t fullOld = options.pathToOld / relativePath / fileInfo.name;
	Path_t relativeTemp = relativePath / DiffFileName(fileInfo.name, config);
	Path_t fullTemp = options.pathToTemp / relativeTemp;

	dung::MemoryBlock oldFile;
	if( !ReadWholeFile( fullOld.wstring(), oldFile ) )
	{
		out << "Can't read file " << fullOld.wstring() << std::endl;
		return false;
	}
	
	dung::SHA1Compute( oldFile.pBlock, oldFile.size, fileInfo.oldSha1 );
	fileInfo.oldSize = oldFile.size;

	if( MatchName( config.oldSkipChanged_regex, fileInfo.name ) )
		return true;

	dung::MemoryBlock newFile;
	if( !ReadWholeFile( fullNew.wstring(), newFile ) )
	{
		out << "Can't read file " << fullNew.wstring() << std::endl;
		return false;
	}

	dung::SHA1Compute( newFile.pBlock, newFile.size, fileInfo.newSha1 );
	fileInfo.newSize = newFile.size;

	bool result = true;

	if( fileInfo.newSha1 != fileInfo.oldSha1 )
	{
		fileInfo.isDifferent = true;
		fs::create_directories( fullTemp.parent_path() );

		result &= CreateDiffFile( diffEncoders, fileInfo, fullNew, fullOld, fullTemp, relativeTemp, newFile, oldFile, package, out );
	}
	else
		fileInfo.isDifferent = false;

	return result;
}

bool rab::BuildDiffFiles( Options const& options, Config const& config, DiffEncoders const& diffEncoders,
	Path_t const& relativePath, PackageOutput_t& package, LogOutput_t& out, FolderInfo::FileInfos_t const& fileInfos )
{
	bool result = true;

	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;

		result &= BuildDiffFile( options, config, diffEncoders, relativePath, package, out, fileInfo );
	}

	return result;
}

bool rab::BuildDiffFolders( Options const& options, Config const& config, DiffEncoders const& diffEncoders, 
	Path_t const& relativePath, PackageOutput_t& package, LogOutput_t& out, FolderInfo::FolderInfos_t const& folderInfos )
{
	bool result = true;

	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;
		
		Path_t nextRelativePath = relativePath / folderInfo.name;

		result &= BuildDiffFiles( options, config, diffEncoders, nextRelativePath, package, out, folderInfo.files_existInBoth );
		result &= BuildDiffFolders( options, config, diffEncoders, nextRelativePath, package, out, folderInfo.folders_existInBoth );
	}

	return result;
}

bool rab::BuildDiffs( Options const& options, Config const& config, DiffEncoders const& diffEncoders, FolderInfo const& rootFolder, 
	PackageOutput_t& package, LogOutput_t& out )
{
	bool result = true;

	out << "Building diff files..." << std::endl;

	Path_t relativePath;

	result &= BuildDiffFiles( options, config, diffEncoders, relativePath, package, out, rootFolder.files_existInBoth );
	result &= BuildDiffFolders( options, config, diffEncoders, relativePath, package, out, rootFolder.folders_existInBoth );

	return result;
}


rab::DiffEncoders::DiffEncoders()
{
}

rab::DiffEncoders::~DiffEncoders()
{
	dung::DeleteContainer( m_diffEncoders );
	dung::DeleteContainer( m_diffEncodersExternal );
}

void rab::DiffEncoders::AddEncoder( dung::DiffEncoder_i& diffEncoder, DiffMethod_t const& encoderName, Config::StringValues_t const& packFiles )
{
	EncoderEntry* pEncoderEntry = SCARAB_NEW EncoderEntry;

	pEncoderEntry->m_encoderName = encoderName;
	pEncoderEntry->m_pDiffEncoder = &diffEncoder;
	BuildRegexVector( packFiles, pEncoderEntry->m_packFiles );

	m_diffEncoders.push_back( pEncoderEntry );
}

void rab::DiffEncoders::AddExternalEncoder( dung::DiffEncoderExternal_i& diffEncoder, DiffMethod_t const& encoderName, Config::StringValues_t const& packFiles )
{
	ExternalEncoderEntry* pEncoderEntry = SCARAB_NEW ExternalEncoderEntry;

	pEncoderEntry->m_encoderName = encoderName;
	pEncoderEntry->m_pDiffEncoder = &diffEncoder;
	BuildRegexVector( packFiles, pEncoderEntry->m_packFiles );

	m_diffEncodersExternal.push_back( pEncoderEntry );
}

dung::DiffEncoder_i* rab::DiffEncoders::FindEncoder( String_t const& fileName, DiffMethod_t& encoderName ) const
{
	for( DiffEncoders_t::const_iterator i = m_diffEncoders.begin(); i != m_diffEncoders.end(); ++i )
	{
		EncoderEntry const& encoderEntry = **i;
		if( MatchName( encoderEntry.m_packFiles, fileName ) )
		{
			encoderName = encoderEntry.m_encoderName;
			return encoderEntry.m_pDiffEncoder;
		}
	}

	return NULL;
}

dung::DiffEncoderExternal_i* rab::DiffEncoders::FindExternalEncoder( String_t const& fileName, DiffMethod_t& encoderName ) const
{
	for( DiffExternalEncoders_t::const_iterator i = m_diffEncodersExternal.begin(); i != m_diffEncodersExternal.end(); ++i )
	{
		ExternalEncoderEntry const& encoderEntry = **i;
		if( MatchName( encoderEntry.m_packFiles, fileName ) )
		{
			encoderName = encoderEntry.m_encoderName;
			return encoderEntry.m_pDiffEncoder;
		}
	}

	return NULL;
}
