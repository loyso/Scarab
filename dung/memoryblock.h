#pragma once

#include "dung.h"

#include <string>

namespace dung
{
	typedef unsigned char Byte_t;

	struct MemoryBlock
	{
		MemoryBlock();
		MemoryBlock( size_t size );
		~MemoryBlock();

		bool operator==( const MemoryBlock& other ) const;
		bool operator!=( const MemoryBlock& other ) const;

		Byte_t* pBlock;
		size_t size;
	};

	bool ReadWholeFile( std::wstring const& fullPath, MemoryBlock& memoryBlock );
	bool ReadWholeFile( std::string const& fullPath, MemoryBlock& memoryBlock );
	bool ReadWholeFile( std::ifstream& in, MemoryBlock& memoryBlock );
	
	bool WriteWholeFile( std::wstring const& fullPath, MemoryBlock& memoryBlock );
	bool WriteWholeFile( std::string const& fullPath, MemoryBlock& memoryBlock );
	bool WriteWholeFile( std::ofstream& out, MemoryBlock& memoryBlock );
}