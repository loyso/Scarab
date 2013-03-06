#pragma once

#include "dung.h"

namespace dung
{
	class DiffEncoder_i
	{
	public:
		virtual bool EncodeDiffMemoryBlock( const Byte_t* newBlock, size_t newSize, const Byte_t* oldBlock, size_t oldSize, Byte_t*& diffBlock, size_t& diffSize ) = 0;
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const = 0;
	};

	class DiffEncoderExternal_i
	{
	public:
		virtual bool EncodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName ) = 0;
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const = 0;
	};
}
