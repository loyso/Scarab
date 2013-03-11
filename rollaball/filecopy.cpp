#include "filecopy.h"

#include "filetree.h"

#include <dung/memoryblock.h>
#include <zlib/minizip.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef fs::path Path_t;

	bool BuildTempCopiesFiles( Options const& options, Config const& config, Path_t const& relativePath, 
		FolderInfo::FileInfos_t const& fileInfos, PackageOutput_t& output, LogOutput_t& out );

	bool BuildTempCopiesFolders( Options const& options, Config const& config, Path_t const& relativePath, 
		FolderInfo::FolderInfos_t const& folderInfos, PackageOutput_t& output, LogOutput_t& out );
}

bool rab::BuildTempCopiesFiles( Options const& options, Config const& config, Path_t const& relativePath, 
	FolderInfo::FileInfos_t const& fileInfos, PackageOutput_t& zipOut, LogOutput_t& out )
{
	int errors = 0;

	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;

		Path_t fullNew = options.pathToNew / relativePath / fileInfo.name;
		Path_t relativeTemp = relativePath / fileInfo.name;
		Path_t fullTemp = options.pathToTemp / relativeTemp;

		dung::MemoryBlock newFile;
		if( !dung::ReadWholeFile( fullNew.wstring(), newFile ) )
		{
			out << "Can't read file " << FileString(fullNew) << std::endl;
			errors++;
			continue;
		}

		dung::SHA1Compute( newFile.pBlock, newFile.size, fileInfo.newSha1 );
		fileInfo.newSize = newFile.size;

		if( config.newFileLimit == 0 || fileInfo.newSize < config.newFileLimit )
		{
			if( options.produceTemp )
			{
				fs::create_directories( fullTemp.parent_path() );

				if( !dung::WriteWholeFile( FileString(fullTemp), newFile ) )
				{
					out << "Can't write whole file " << FileString(fullTemp) << std::endl;
					errors++;
				}
			}

			if( !zipOut.WriteFile( GenericString(relativeTemp), newFile.pBlock, newFile.size ) )
			{
				out << "Can't write file to archive " << GenericString(relativeTemp) << " size=" << newFile.size << std::endl;
				errors++;
			}
		}
		else
			out << "Skipping file " << FileString(fullNew) << "size=" << fileInfo.newSize << " because of limit " << config.newFileLimit << std::endl;
	}

	return errors == 0;
}

bool rab::BuildTempCopiesFolders( Options const& options, Config const& config, Path_t const& relativePath, 
	FolderInfo::FolderInfos_t const& folderInfos, PackageOutput_t& zipOut, LogOutput_t& out )
{
	bool result = true;

	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;

		Path_t nextRelativePath = relativePath / folderInfo.name;

		result &= BuildTempCopiesFiles( options, config, nextRelativePath, folderInfo.files_newOnly, zipOut, out );
		
		result &= BuildTempCopiesFolders( options, config, nextRelativePath, folderInfo.folders_newOnly, zipOut, out );
		result &= BuildTempCopiesFolders( options, config, nextRelativePath, folderInfo.folders_existInBoth, zipOut, out );
	}

	return result;
}

bool rab::BuildTempCopies( Options const& options, Config const& config, FolderInfo const& rootFolder, 
	PackageOutput_t& zipOut, LogOutput_t& out )
{
	bool result = true;

	Path_t relativePath;

	out << "Building whole file copies (new files)..." << std::endl;
	result &= BuildTempCopiesFiles( options, config, relativePath, rootFolder.files_newOnly, zipOut, out );

	result &= BuildTempCopiesFolders( options, config, relativePath, rootFolder.folders_newOnly, zipOut, out );
	result &= BuildTempCopiesFolders( options, config, relativePath, rootFolder.folders_existInBoth, zipOut, out );

	return result;
}
