#pragma once

#include "dung.h"

namespace dung
{
	typedef unsigned char Byte_t;

	class DiffDecoder_i
	{
	public:
		virtual bool DecodeDiffMemoryBlock( const Byte_t* oldBlock, size_t oldSize, const Byte_t* diffBlock, size_t diffSize, Byte_t*& newBlock, size_t& newSize ) = 0;
		virtual void GetErrorMessage( _tstring& errorMessage ) const = 0;
	};

	class DiffDecoderExternal_i
	{
	public:
		virtual bool DecodeDiffFile( const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName ) = 0;
		virtual void GetErrorMessage( _tstring& errorMessage ) const = 0;
	};
}
