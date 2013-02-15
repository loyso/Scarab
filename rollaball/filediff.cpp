#include "filediff.h"

#include "filetree.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef fs::path Path_t;

	void BuildDiffFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos );
	void BuildDiffFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos );
}

void rab::BuildDiffFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;
		
		Path_t fullNew = options.pathToNew / relativePath / fileInfo.m_name;
		Path_t fullOld = options.pathToOld / relativePath / fileInfo.m_name;
		Path_t fullTemp = options.pathToTemp / relativePath / ( fileInfo.m_name + _T(".") + config.packedExtension );
	}
}

void rab::BuildDiffFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;
		
		Path_t nextRelativePath = relativePath / folderInfo.m_name;

		BuildDiffFiles( options, config, nextRelativePath, folderInfo.m_files_existInBoth );
		BuildDiffFolders( options, config, nextRelativePath, folderInfo.m_folders_existInBoth );
	}
}

void rab::BuildDiffs( Options const& options, Config const& config, FolderInfo const& rootFolder )
{
	fs::create_directories( options.pathToTemp );

	Path_t relativePath;

	BuildDiffFiles( options, config, relativePath, rootFolder.m_files_existInBoth );
	BuildDiffFolders( options, config, relativePath, rootFolder.m_folders_existInBoth );
}
