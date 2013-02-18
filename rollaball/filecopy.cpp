#include "filecopy.h"

#include "filetree.h"

#include <dung/memoryblock.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef fs::path Path_t;

	void BuildTempCopiesFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos );
	void BuildTempCopiesFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos );
}

void rab::BuildTempCopiesFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;

		Path_t fullNew = options.pathToNew / relativePath / fileInfo.name;
		Path_t fullTemp = options.pathToTemp / relativePath / fileInfo.name;

		dung::MemoryBlock newFile;
		if( dung::ReadWholeFile( fullNew.wstring(), newFile ) )
		{
			int sha1result = SHA1Compute( newFile.pBlock, newFile.size, fileInfo.newSha1 );
			if( sha1result == shaSuccess )
			{
				fileInfo.newSize = newFile.size;
				if( !fs::exists(fullTemp) )
					dung::WriteWholeFile( fullTemp.wstring(), newFile );
			}
		}
	}
}

void rab::BuildTempCopiesFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;

		Path_t nextRelativePath = relativePath / folderInfo.name;
		fs::create_directory( options.pathToTemp / nextRelativePath );

		BuildTempCopiesFiles( options, config, nextRelativePath, folderInfo.files_newOnly );
		
		BuildTempCopiesFolders( options, config, nextRelativePath, folderInfo.folders_newOnly );
		BuildTempCopiesFolders( options, config, nextRelativePath, folderInfo.folders_existInBoth );
	}
}

void rab::BuildTempCopies( Options const& options, Config const& config, FolderInfo const& rootFolder )
{
	Path_t relativePath;
	fs::create_directories( options.pathToTemp );

	BuildTempCopiesFiles( options, config, relativePath, rootFolder.files_newOnly );

	BuildTempCopiesFolders( options, config, relativePath, rootFolder.folders_newOnly );
	BuildTempCopiesFolders( options, config, relativePath, rootFolder.folders_existInBoth );
}
