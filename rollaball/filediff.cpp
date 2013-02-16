#include "filediff.h"

#include "filetree.h"

#include <fstream>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

extern "C"
{
#include <external/xdelta/xdelta3.h>
}

namespace rab
{
	typedef fs::path Path_t;
	typedef unsigned char Byte_t;

	struct MemoryBlock
	{
		MemoryBlock();
		~MemoryBlock();

		Byte_t* pBlock;
		size_t size;
	};

	bool ReadWholeFile( Path_t const& fullPath, MemoryBlock& memoryBlock );
	bool WriteWholeFile( Path_t const& fullPath, MemoryBlock& memoryBlock );

	void BuildDiffFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos );
	void BuildDiffFolders( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FolderInfos_t const& folderInfos );
}

rab::MemoryBlock::MemoryBlock()
	: pBlock()
	, size()
{
}

rab::MemoryBlock::~MemoryBlock()
{
	delete[] pBlock;
}

bool rab::ReadWholeFile( Path_t const& fullPath, MemoryBlock& memoryBlock )
{
	std::ifstream file ( fullPath.string(), std::ios::in|std::ios::binary|std::ios::ate );
	if( !file.is_open() )
		return false;

	memoryBlock.size = (size_t)file.tellg();

	memoryBlock.pBlock = SCARAB_NEW Byte_t [memoryBlock.size];

	file.seekg( 0, std::ios::beg );
	file.read( (char*)memoryBlock.pBlock, memoryBlock.size );
	file.close();

	return true;
}

bool rab::WriteWholeFile( Path_t const& fullPath, MemoryBlock& memoryBlock )
{
	std::ofstream file ( fullPath.string(), std::ios::out|std::ios::binary|std::ios::trunc );
	if( !file.is_open() )
		return false;

	file.write( (const char*)memoryBlock.pBlock, memoryBlock.size );
	file.close();
	return true;
}

void rab::BuildDiffFiles( Options const& options, Config const& config, Path_t const& relativePath, FolderInfo::FileInfos_t const& fileInfos )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo& fileInfo = **i;
		
		Path_t fullNew = options.pathToNew / relativePath / fileInfo.name;
		Path_t fullOld = options.pathToOld / relativePath / fileInfo.name;
		Path_t fullTemp = options.pathToTemp / relativePath / ( fileInfo.name + _T(".") + config.packedExtension );

		MemoryBlock newFile;
		MemoryBlock oldFile;
		if( ReadWholeFile( fullNew, newFile ) && ReadWholeFile( fullOld, oldFile ) )
		{
			MemoryBlock deltaFile;
			deltaFile.size = newFile.size + oldFile.size;
			deltaFile.pBlock = SCARAB_NEW Byte_t[ deltaFile.size ];
			size_t size;

			int ret = xd3_encode_memory( newFile.pBlock, newFile.size, oldFile.pBlock, oldFile.size, deltaFile.pBlock, &size, deltaFile.size, 0 );
			if( ret == 0 )
			{
				deltaFile.size = size;
				WriteWholeFile( fullTemp, deltaFile );
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
