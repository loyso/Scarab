#pragma once

namespace dung
{
	class DiffDecoder_i
	{
	};

	class DiffDecoderExternal_i
	{
	public:
		virtual bool DecodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName ) = 0;
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const = 0;
	};
}
