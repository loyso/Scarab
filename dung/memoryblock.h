#pragma once

#include "dung.h"

#include <string>

namespace dung
{
	typedef unsigned char Byte_t;
	typedef _tstring String_t;

	struct MemoryBlock
	{
		MemoryBlock();
		MemoryBlock( size_t size );
		~MemoryBlock();

		Byte_t* pBlock;
		size_t size;
	};

	bool ReadWholeFile( String_t const& fullPath, MemoryBlock& memoryBlock );
	bool WriteWholeFile( String_t const& fullPath, MemoryBlock& memoryBlock );
	bool Equals( MemoryBlock const& block1, MemoryBlock const& block2 );
}