#include "filediff.h"

#include "filetree.h"

#include <dung/memoryblock.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

extern "C"
{
#include <external/xdelta/xdelta3.h>
}

namespace rab
{
	typedef fs::path Path_t;

	bool EncodeAndWrite( dung::MemoryBlock const& newFile, dung::MemoryBlock const& oldFile, Path_t const& fullTemp );

	void BuildDiffFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos );
	void BuildDiffFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos );
}

bool rab::EncodeAndWrite( dung::MemoryBlock const& newFile, dung::MemoryBlock const& oldFile, Path_t const& fullTemp )
{
	const size_t reservedSize = 2 * ( newFile.size + oldFile.size );
	dung::MemoryBlock deltaFile( reservedSize );
	deltaFile.size = 0;

	int ret = xd3_encode_memory( newFile.pBlock, newFile.size, oldFile.pBlock, oldFile.size, deltaFile.pBlock, &deltaFile.size, reservedSize, 0 );
	if( ret != 0 )
		return false;

	WriteWholeFile( fullTemp.wstring(), deltaFile );
	return true;
}

void rab::BuildDiffFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;
		
		Path_t fullNew = options.pathToNew / relativePath / fileInfo.name;
		Path_t fullOld = options.pathToOld / relativePath / fileInfo.name;
		Path_t fullTemp = options.pathToTemp / relativePath / DiffFileName(fileInfo.name, config);

		dung::MemoryBlock newFile;
		dung::MemoryBlock oldFile;
		if( ReadWholeFile( fullNew.wstring(), newFile ) && ReadWholeFile( fullOld.wstring(), oldFile ) )
		{
			int sha1resultNew = SHA1Compute( newFile.pBlock, newFile.size, fileInfo.newSha1 );
			int sha1resultOld = SHA1Compute( oldFile.pBlock, oldFile.size, fileInfo.oldSha1 );

			if( sha1resultNew == shaSuccess && sha1resultOld == shaSuccess )
			{
				fileInfo.oldSize = oldFile.size;
				fileInfo.newSize = newFile.size;
				if( Equals( newFile, oldFile ) )
				{
					SCARAB_ASSERT( fileInfo.newSha1 == fileInfo.oldSha1 ); //TODO: rely on this.
					fileInfo.isDifferent = false;
				}
				else
				{
					fileInfo.isDifferent = true;
					EncodeAndWrite(newFile, oldFile, fullTemp);
				}
			}
		}
	}
}

void rab::BuildDiffFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;
		
		Path_t nextRelativePath = relativePath / folderInfo.name;
		fs::create_directory( options.pathToTemp / nextRelativePath );

		BuildDiffFiles( options, config, nextRelativePath, folderInfo.files_existInBoth );
		BuildDiffFolders( options, config, nextRelativePath, folderInfo.folders_existInBoth );
	}
}

void rab::BuildDiffs( Options const& options, Config const& config, FolderInfo const& rootFolder )
{
	Path_t relativePath;
	fs::create_directories( options.pathToTemp );

	BuildDiffFiles( options, config, relativePath, rootFolder.files_existInBoth );
	BuildDiffFolders( options, config, relativePath, rootFolder.folders_existInBoth );
}
