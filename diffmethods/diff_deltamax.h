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
		void SetUserLicense( const _TCHAR* userName, const _TCHAR* licenseKey );

	protected:
		void GetErrorMessage( _tstring& errorMessage ) const;

		const _TCHAR* m_userName; 
		const _TCHAR* m_licenseKey;
		int m_result;
	};

	class DeltaMaxEncoder : public DeltaMax, public dung::DiffEncoderExternal_i
	{
	public:
		DeltaMaxEncoder();
	private:
		virtual bool EncodeDiffFile( const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName );
		virtual void GetErrorMessage( _tstring& errorMessage ) const;
	};

	class DeltaMaxDecoder : public DeltaMax, public dung::DiffDecoderExternal_i
	{
	public:
		DeltaMaxDecoder();
	private:
		virtual bool DecodeDiffFile( const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName );
		virtual void GetErrorMessage( _tstring& errorMessage ) const;
	};
}

#endif // SCARAB_DELTAMAX
