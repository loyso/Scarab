#include "diff_deltamax.h"

#if SCARAB_DELTAMAX

#ifdef SCARAB_WCHAR_MODE
#	pragma comment(linker, "/defaultlib:DeltaMAX_unicode.lib")
#	pragma message("Automatically linking with DeltaMAX_unicode.lib (DeltaMAX_unicode.dll)")
#else
#	pragma comment(linker, "/defaultlib:DeltaMAX.lib")
#	pragma message("Automatically linking with DeltaMAX.lib (DeltaMAX.dll)")
#endif

#define _TCHAR_DEFINED
#define TCHAR _TCHAR

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <external/deltamax/Include/DeltaMAX.h>
#include <external/deltamax/Include/DeltaMAX_messages.h>

namespace deltamax
{
}


deltamax::DeltaMax::DeltaMax()
	: m_userName()
	, m_licenseKey()
	, m_result( DELTAMAX_ERR_OK )
{
}

void deltamax::DeltaMax::SetUserLicense( const _TCHAR* userName, const _TCHAR* licenseKey )
{
	m_userName = userName;
	m_licenseKey = licenseKey;
}

void deltamax::DeltaMax::GetErrorMessage( _tstring& errorMessage ) const
{
	errorMessage = DeltaMAXGetErrorString( m_result );
}


deltamax::DeltaMaxEncoder::DeltaMaxEncoder()
{
}

bool deltamax::DeltaMaxEncoder::EncodeDiffFile( const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName )
{
	DELTAMAX_ENCODE_OPTIONS options;
	DeltaMAXInitEncodeOptions( &options );
	options.lpszLicensedTo = m_userName;
	options.lpszLicenseKey = m_licenseKey;
	options.lpUserData = this;

	m_result = DeltaMAXEncode( oldFileName, newFileName, diffFileName, &options );
	if( m_result == DELTAMAX_ERR_OK )
		return true;

	return false;
}

void deltamax::DeltaMaxEncoder::GetErrorMessage( _tstring& errorMessage ) const
{
	DeltaMax::GetErrorMessage( errorMessage );
}

deltamax::DeltaMaxDecoder::DeltaMaxDecoder()
{
}

bool deltamax::DeltaMaxDecoder::DecodeDiffFile( const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName )
{
	DELTAMAX_DECODE_OPTIONS options;
	DeltaMAXInitDecodeOptions( &options );
	options.lpszLicensedTo = m_userName;
	options.lpszLicenseKey = m_licenseKey;
	options.lpUserData = this;

	m_result = DeltaMAXDecode( oldFileName, newFileName, diffFileName, &options );
	if( m_result == DELTAMAX_ERR_OK )
		return true;

	return false;
}

void deltamax::DeltaMaxDecoder::GetErrorMessage( _tstring& errorMessage ) const
{
	DeltaMax::GetErrorMessage( errorMessage );
}

#endif // SCARAB_DELTAMAX
