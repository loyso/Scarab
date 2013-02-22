#include "filecopy.h"

#include "filetree.h"

#include <dung/memoryblock.h>
#include <zlib/minizip.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef fs::path Path_t;

	void BuildTempCopiesFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos, PackageOutput_t& output );
	void BuildTempCopiesFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos, PackageOutput_t& output );
}

void rab::BuildTempCopiesFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos, PackageOutput_t& output )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;

		Path_t fullNew = options.pathToNew / relativePath / fileInfo.name;
		Path_t relativeTemp = relativePath / fileInfo.name;
		Path_t fullTemp = options.pathToTemp / relativeTemp;

		dung::MemoryBlock newFile;
		if( dung::ReadWholeFile( fullNew.wstring(), newFile ) )
		{
			int sha1result = SHA1Compute( newFile.pBlock, newFile.size, fileInfo.newSha1 );
			if( sha1result == shaSuccess )
			{
				fileInfo.newSize = newFile.size;

				fs::create_directories( fullTemp.parent_path() );
				if( !fs::exists(fullTemp) )
					dung::WriteWholeFile( fullTemp.wstring(), newFile );

				output.WriteFile( relativeTemp.generic_wstring(), newFile.pBlock, newFile.size );
			}
		}
	}
}

void rab::BuildTempCopiesFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos, PackageOutput_t& output )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;

		Path_t nextRelativePath = relativePath / folderInfo.name;

		BuildTempCopiesFiles( options, config, nextRelativePath, folderInfo.files_newOnly, output );
		
		BuildTempCopiesFolders( options, config, nextRelativePath, folderInfo.folders_newOnly, output );
		BuildTempCopiesFolders( options, config, nextRelativePath, folderInfo.folders_existInBoth, output );
	}
}

void rab::BuildTempCopies( Options const& options, Config const& config, FolderInfo const& rootFolder, PackageOutput_t& output )
{
	Path_t relativePath;

	BuildTempCopiesFiles( options, config, relativePath, rootFolder.files_newOnly, output );

	BuildTempCopiesFolders( options, config, relativePath, rootFolder.folders_newOnly, output );
	BuildTempCopiesFolders( options, config, relativePath, rootFolder.folders_existInBoth, output );
}
