#pragma once

#include "dung.h"

namespace dung
{
	typedef unsigned char Byte_t;

	class DiffEncoder_i
	{
	public:
		virtual bool EncodeDiffMemoryBlock( const Byte_t* newBlock, size_t newSize, const Byte_t* oldBlock, size_t oldSize, Byte_t*& diffBlock, size_t& diffSize ) = 0;
		virtual void GetErrorMessage( _tstring& errorMessage ) const = 0;
	};

	class DiffEncoderExternal_i
	{
	public:
		virtual bool EncodeDiffFile( const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName ) = 0;
		virtual void GetErrorMessage( _tstring& errorMessage ) const = 0;
	};
}
