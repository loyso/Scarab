#pragma once

#ifndef DELTAMAX
#define DELTAMAX 0 // disabled by default
#endif

#if DELTAMAX

#include <dung/diffencoder.h>
#include <dung/diffdecoder.h>

namespace deltamax
{
	class DeltaMaxEncoder : public dung::DiffEncoderExternal_i
	{
	public:
		DeltaMaxEncoder( const char* userName, const char* licenseKey );

		virtual bool EncodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName );
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const;

	private:
		const char* m_userName; 
		const char* m_licenseKey;
		int m_result;
	};

	class DeltaMaxDecoder : public dung::DiffDecoderExternal_i
	{
	};
}

#endif // DELTAMAX
