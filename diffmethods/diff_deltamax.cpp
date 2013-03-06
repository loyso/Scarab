#include "diff_deltamax.h"

#if SCARAB_DELTAMAX

#pragma comment(linker, "/defaultlib:DeltaMAX.lib")
#pragma message("Automatically linking with DeltaMAX.lib (DeltaMAX.dll)")

#ifdef DELTAMAX_UNICODE
#	define _T(x) L ## x
#	define TCHAR wchar_t
#else
#	define _T(x) x
#	define TCHAR char
#	undef UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _TCHAR_DEFINED
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

void deltamax::DeltaMax::SetUserLicense( const char* userName, const char* licenseKey )
{
	m_userName = userName;
	m_licenseKey = licenseKey;
}

void deltamax::DeltaMax::GetErrorMessage( char* errorMessage, size_t bufferSize ) const
{
	const char* errorString = DeltaMAXGetErrorString( m_result );
	strncpy( errorMessage, errorString, bufferSize-1 );
}


deltamax::DeltaMaxEncoder::DeltaMaxEncoder()
{
}

bool deltamax::DeltaMaxEncoder::EncodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName )
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

void deltamax::DeltaMaxEncoder::GetErrorMessage( char* errorMessage, size_t bufferSize ) const
{
	DeltaMax::GetErrorMessage( errorMessage, bufferSize );
}

deltamax::DeltaMaxDecoder::DeltaMaxDecoder()
{
}

bool deltamax::DeltaMaxDecoder::DecodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName )
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

void deltamax::DeltaMaxDecoder::GetErrorMessage( char* errorMessage, size_t bufferSize ) const
{
	DeltaMax::GetErrorMessage( errorMessage, bufferSize );
}

#endif // SCARAB_DELTAMAX
