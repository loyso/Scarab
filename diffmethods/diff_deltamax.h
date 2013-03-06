#pragma once

#include "diffmethods.h"

#ifndef SCARAB_DELTAMAX
#define SCARAB_DELTAMAX 0 // disabled by default
#endif

#if SCARAB_DELTAMAX

#include <dung/diffencoder.h>
#include <dung/diffdecoder.h>

namespace deltamax
{
	class DeltaMax
	{
	public:
		DeltaMax();
		void SetUserLicense( const char* userName, const char* licenseKey );

	protected:
		void GetErrorMessage( char* errorMessage, size_t bufferSize ) const;

		const char* m_userName; 
		const char* m_licenseKey;
		int m_result;
	};

	class DeltaMaxEncoder : public DeltaMax, public dung::DiffEncoderExternal_i
	{
	public:
		DeltaMaxEncoder();
	private:
		virtual bool EncodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName );
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const;
	};

	class DeltaMaxDecoder : public DeltaMax, public dung::DiffDecoderExternal_i
	{
	public:
		DeltaMaxDecoder();
	private:
		virtual bool DecodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName );
		virtual void GetErrorMessage( char* errorMessage, size_t bufferSize ) const;
	};
}

#endif // SCARAB_DELTAMAX
