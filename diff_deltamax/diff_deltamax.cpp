#include "diff_deltamax.h"

#if DELTAMAX

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

deltamax::DeltaMaxEncoder::DeltaMaxEncoder( const char* userName, const char* licenseKey )
	: m_userName( userName )
	, m_licenseKey( licenseKey )
	, m_result( DELTAMAX_ERR_OK )
{
}

bool deltamax::DeltaMaxEncoder::EncodeDiffFile( const char* newFileName, const char* oldFileName, const char* diffFileName )
{
	DELTAMAX_ENCODE_OPTIONS options;
	DeltaMAXInitEncodeOptions( &options );
	options.lpszLicensedTo = m_userName;
	options.lpszLicenseKey = m_licenseKey;
	options.lpUserData = this;

	int result = DeltaMAXEncode( oldFileName, newFileName, diffFileName, &options );
	if( result == DELTAMAX_ERR_OK )
		return true;

	return false;
}

void deltamax::DeltaMaxEncoder::GetErrorMessage( char* errorMessage, size_t bufferSize ) const
{
	const char* errorString = DeltaMAXGetErrorString( m_result );
	strncpy( errorMessage, errorString, bufferSize-1 );
}

#endif // DELTAMAX
