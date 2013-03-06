#pragma once

namespace dung
{
	class DiffDecoder_i
	{
	public:
		virtual bool DecodeDiffMemoryBlock( const Byte_t* oldBlock, size_t oldSize, const Byte_t* diffBlock, size_t diffSize, Byte_t*& newBlock, size_t& newSize ) = 0;
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const = 0;
	};

	class DiffDecoderExternal_i
	{
	public:
		virtual bool DecodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName ) = 0;
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const = 0;
	};
}
