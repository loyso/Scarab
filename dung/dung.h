#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define SCARAB_WCHAR_MODE // you can comment it out.

#define NOMINMAX

#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#	ifndef DBG_NEW
#		define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#		define SCARAB_NEW DBG_NEW
#	else
#		define SCARAB_NEW new
#	endif
#else  // NDEBUG
#	define SCARAB_NEW new
#endif

#include <assert.h>
#define SCARAB_ASSERT assert

#ifdef SCARAB_WCHAR_MODE
#	define UTF_CONVERT_WCHAR_MODE
#	define	_TCHAR wchar_t
#	define _T(x) L ## x
#	define _tmain wmain
#	define _tstring std::wstring
#	define _tostream std::wostream
#	define _tcout std::wcout
#	define _tstringstream std::wstringstream
#	define _tstreambuf std::wstreambuf
#	define _tchar_to_long _wtol
#	define _ttolower towlower
#	define _tregex std::wregex
#	define _tstrcmp wcscmp
#else
#	define	_TCHAR char
#	define _T(x) x
#	define _tmain main
#	define _tstring std::string
#	define _tostream std::ostream
#	define _tcout std::cout
#	define _tstringstream std::stringstream
#	define _tstreambuf std::streambuf
#	define _tchar_to_long atol
#	define _ttolower tolower
#	define _tregex std::regex
#	define _tstrcmp strcmp
#endif

#include <streambuf>

#include "utf_convert.h"

namespace dung
{
	typedef unsigned char Byte_t;

	extern const _TCHAR REGISTRY_FILENAME[];

	template< typename T >
	void DeleteContainer( T& container )
	{
		for( T::iterator i = container.begin(); i != container.end(); ++i )
			delete *i;
		container.clear();
	}

	class nil_buf : public _tstreambuf  
	{
	public:
		nil_buf();
		virtual	~nil_buf();

	protected:
		virtual int	overflow( int c_ );
		virtual int sync();
		void put_buffer();
		void put_char( int c_ );
	};

	size_t StrLen( const char* str );
	size_t StrLen( const wchar_t* str );
}
