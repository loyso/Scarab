#include "memoryblock.h"

#include <fstream>

dung::MemoryBlock::MemoryBlock()
	: pBlock()
	, size()
{
}

dung::MemoryBlock::MemoryBlock( size_t size )
	: pBlock()
	, size( size )
{
	pBlock = SCARAB_NEW Byte_t[ size ];
}

dung::MemoryBlock::~MemoryBlock()
{
	delete[] pBlock;
}

bool dung::MemoryBlock::operator==( const MemoryBlock& other ) const
{
	if( other.size != other.size )
		return false;

	return memcmp( other.pBlock, other.pBlock, other.size ) == 0;
}

bool dung::MemoryBlock::operator!=( const MemoryBlock& other ) const
{
	return !operator==( other );
}

bool dung::ReadWholeFile( std::wstring const& fullPath, MemoryBlock& memoryBlock )
{
	std::ifstream file ( fullPath, std::ios::in|std::ios::binary|std::ios::ate );
	return ReadWholeFile( file, memoryBlock );
}

bool dung::ReadWholeFile( std::string const& fullPath, MemoryBlock& memoryBlock )
{
	std::ifstream file ( fullPath, std::ios::in|std::ios::binary|std::ios::ate );
	return ReadWholeFile( file, memoryBlock );
}

bool dung::ReadWholeFile( std::ifstream& file, MemoryBlock& memoryBlock )
{
	if( !file.is_open() )
		return false;

	memoryBlock.size = (size_t)file.tellg();

	memoryBlock.pBlock = SCARAB_NEW Byte_t [memoryBlock.size];

	file.seekg( 0, std::ios::beg );
	file.read( (char*)memoryBlock.pBlock, memoryBlock.size );
	file.close();

	return true;
}

bool dung::WriteWholeFile( std::wstring const& fullPath, MemoryBlock& memoryBlock )
{
	std::ofstream file ( fullPath, std::ios::out|std::ios::binary|std::ios::trunc );
	return WriteWholeFile( file, memoryBlock );
}

bool dung::WriteWholeFile( std::string const& fullPath, MemoryBlock& memoryBlock )
{
	std::ofstream file ( fullPath, std::ios::out|std::ios::binary|std::ios::trunc );
	return WriteWholeFile( file, memoryBlock );
}

bool dung::WriteWholeFile( std::ofstream& file, MemoryBlock& memoryBlock )
{
	if( !file.is_open() )
		return false;

	file.write( (const char*)memoryBlock.pBlock, memoryBlock.size );
	file.close();
	return true;
}
