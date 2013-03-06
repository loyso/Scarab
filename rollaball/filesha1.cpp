#include "filesha1.h"

#include "filetree.h"

#include <dung/memoryblock.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef fs::path Path_t;

	bool GatherSha1Files( Options const& options, Config const& config, Path_t const& relativePath, 
		FolderInfo::FileInfos_t const& fileInfos, LogOutput_t& out );

	bool GatherSha1Folders( Options const& options, Config const& config, Path_t const& relativePath, 
		FolderInfo::FolderInfos_t const& folderInfos, LogOutput_t& out );
}

bool rab::GatherSha1Files( Options const& options, Config const& config, Path_t const& relativePath, 
	FolderInfo::FileInfos_t const& fileInfos, LogOutput_t& out )
{
	int errors = 0;

	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;

		Path_t fullOld = options.pathToOld / relativePath / fileInfo.name;

		dung::MemoryBlock oldFile;
		if( !dung::ReadWholeFile( fullOld.wstring(), oldFile ) )
		{
			out << "Can't read file " << fullOld.wstring() << std::endl;
			errors++;
			continue;
		}
		
		dung::SHA1Compute( oldFile.pBlock, oldFile.size, fileInfo.oldSha1 );
		fileInfo.oldSize = oldFile.size;
	}

	return errors == 0;
}

bool rab::GatherSha1Folders( Options const& options, Config const& config, Path_t const& relativePath, 
	FolderInfo::FolderInfos_t const& folderInfos, LogOutput_t& out )
{
	bool result = true;

	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;

		Path_t nextRelativePath = relativePath / folderInfo.name;

		result &= GatherSha1Files( options, config, nextRelativePath, folderInfo.files_oldOnly, out );
		
		result &= GatherSha1Folders( options, config, nextRelativePath, folderInfo.folders_oldOnly, out );
		result &= GatherSha1Folders( options, config, nextRelativePath, folderInfo.folders_existInBoth, out );
	}

	return result;
}

bool rab::GatherSha1( Options const& options, Config const& config, FolderInfo const& rootFolder, LogOutput_t& out )
{
	bool result = true;

	Path_t relativePath;

	out << "Gathering SHA1 digests for files in old directory..." << std:: endl;
	result &= GatherSha1Files( options, config, relativePath, rootFolder.files_oldOnly, out );

	result &= GatherSha1Folders( options, config, relativePath, rootFolder.folders_oldOnly, out );
	result &= GatherSha1Folders( options, config, relativePath, rootFolder.folders_existInBoth, out );

	return result;
}
