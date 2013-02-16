#include "filecopy.h"

#include "filetree.h"

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

		Path_t fullNew = options.pathToNew / relativePath / fileInfo.m_name;
		Path_t fullTemp = options.pathToTemp / relativePath / fileInfo.m_name;

		fs::copy( fullNew, fullTemp );
	}
}

void rab::BuildTempCopiesFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;

		Path_t nextRelativePath = relativePath / folderInfo.m_name;
		fs::create_directory( options.pathToTemp / nextRelativePath );

		BuildTempCopiesFiles( options, config, nextRelativePath, folderInfo.m_files_newOnly );
		
		BuildTempCopiesFolders( options, config, nextRelativePath, folderInfo.m_folders_newOnly );
		BuildTempCopiesFolders( options, config, nextRelativePath, folderInfo.m_folders_existInBoth );
	}
}

void rab::BuildTempCopies( Options const& options, Config const& config, FolderInfo const& rootFolder )
{
	Path_t relativePath;
	fs::create_directories( options.pathToTemp );

	BuildTempCopiesFiles( options, config, relativePath, rootFolder.m_files_newOnly );

	BuildTempCopiesFolders( options, config, relativePath, rootFolder.m_folders_newOnly );
	BuildTempCopiesFolders( options, config, relativePath, rootFolder.m_folders_existInBoth );
}
