#include "filesha1.h"

#include "filetree.h"

#include <dung/memoryblock.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef fs::path Path_t;

	void GatherSha1Files( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos );
	void GatherSha1Folders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos );
}

void rab::GatherSha1Files( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;

		Path_t fullOld = options.pathToOld / relativePath / fileInfo.name;

		dung::MemoryBlock oldFile;
		if( dung::ReadWholeFile( fullOld.wstring(), oldFile ) )
		{
			int sha1result = SHA1Compute( oldFile.pBlock, oldFile.size, fileInfo.oldSha1 );
			if( sha1result == shaSuccess )
				fileInfo.oldSize = oldFile.size;
		}
	}
}

void rab::GatherSha1Folders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;

		Path_t nextRelativePath = relativePath / folderInfo.name;

		GatherSha1Files( options, config, nextRelativePath, folderInfo.files_oldOnly );
		
		GatherSha1Folders( options, config, nextRelativePath, folderInfo.folders_oldOnly );
		GatherSha1Folders( options, config, nextRelativePath, folderInfo.folders_existInBoth );
	}
}

void rab::GatherSha1( Options const& options, Config const& config, FolderInfo const& rootFolder )
{
	Path_t relativePath;

	GatherSha1Files( options, config, relativePath, rootFolder.files_oldOnly );

	GatherSha1Folders( options, config, relativePath, rootFolder.folders_oldOnly );
	GatherSha1Folders( options, config, relativePath, rootFolder.folders_existInBoth );
}
