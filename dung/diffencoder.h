#pragma once

namespace dung
{
	class DiffEncoder_i
	{
	public:
		virtual bool EncodeDiffMemoryBlock( const void* newBlock, size_t newSize, const void* oldBlock, size_t oldSize, void*& diffBlock, size_t& diffSize ) = 0;
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const = 0;
	};

	class DiffEncoderExternal_i
	{
	public:
		virtual bool EncodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName ) = 0;
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const = 0;
	};
}
