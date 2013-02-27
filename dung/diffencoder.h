#pragma once

namespace dung
{
	class DiffEncoder_i
	{
	public:
	};

	class DiffEncoderExternal_i
	{
	public:
		virtual bool EncodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName ) = 0;
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const = 0;
	};
}
