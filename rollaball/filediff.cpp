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

	bool EncodeAndWrite( dung::MemoryBlock const& newFile, dung::MemoryBlock const& oldFile, Path_t const& fullTemp, Path_t const& relativeTemp, PackageOutput_t& output );

	void BuildDiffFiles( Options const& options, Config const& config, Path_t const& relativePath, PackageOutput_t& output, FolderInfo::FileInfos_t const& fileInfos );
	void BuildDiffFolders( Options const& options, Config const& config, Path_t const& relativePath, PackageOutput_t& output, FolderInfo::FolderInfos_t const& folderInfos );
}

bool rab::EncodeAndWrite( dung::MemoryBlock const& newFile, dung::MemoryBlock const& oldFile, Path_t const& fullTemp, Path_t const& relativeTemp, PackageOutput_t& output )
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

void rab::BuildDiffFiles( Options const& options, Config const& config, Path_t const& relativePath, PackageOutput_t& output, FolderInfo::FileInfos_t const& fileInfos )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;

		Path_t fullNew = options.pathToNew / relativePath / fileInfo.name;
		Path_t fullOld = options.pathToOld / relativePath / fileInfo.name;
		Path_t relativeTemp = relativePath / DiffFileName(fileInfo.name, config);
		Path_t fullTemp = options.pathToTemp / relativeTemp;

		dung::MemoryBlock oldFile;
		if( !ReadWholeFile( fullOld.wstring(), oldFile ) )
			continue;
		dung::SHA1Compute( oldFile.pBlock, oldFile.size, fileInfo.oldSha1 );
		fileInfo.oldSize = oldFile.size;

		if( MatchName( config.oldSkipChanged_regex, fileInfo.name ) )
			continue;

		dung::MemoryBlock newFile;
		if( !ReadWholeFile( fullNew.wstring(), newFile ) )
			continue;
		dung::SHA1Compute( newFile.pBlock, newFile.size, fileInfo.newSha1 );
		fileInfo.newSize = newFile.size;

		if( fileInfo.newSha1 != fileInfo.oldSha1 )
		{
			fileInfo.isDifferent = true;
			fs::create_directories( fullTemp.parent_path() );
			if( !EncodeAndWrite( newFile, oldFile, fullTemp, relativeTemp, output ) )
			{
				// TODO: report errors.
			}
		}
		else
			fileInfo.isDifferent = false;
	}
}

void rab::BuildDiffFolders( Options const& options, Config const& config, Path_t const& relativePath, PackageOutput_t& output, FolderInfo::FolderInfos_t const& folderInfos )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;
		
		Path_t nextRelativePath = relativePath / folderInfo.name;

		BuildDiffFiles( options, config, nextRelativePath, output, folderInfo.files_existInBoth );
		BuildDiffFolders( options, config, nextRelativePath, output, folderInfo.folders_existInBoth );
	}
}

void rab::BuildDiffs( Options const& options, Config const& config, FolderInfo const& rootFolder, PackageOutput_t& output )
{
	Path_t relativePath;

	BuildDiffFiles( options, config, relativePath, output, rootFolder.files_existInBoth );
	BuildDiffFolders( options, config, relativePath, output, rootFolder.folders_existInBoth );
}
