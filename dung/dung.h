#pragma once

#define _CRT_SECURE_NO_WARNINGS
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

#ifdef  _UNICODE
#	define	_TCHAR wchar_t
#	define _T(x) L ## x
#	define _tmain wmain
#	define _tstring std::wstring
#else
#	define	_TCHAR char
#	define _T(x) x
#	define _tmain main
#	define _tstring std::string
#endif

#include <streambuf>

namespace dung
{
	extern const wchar_t WREGISTRY_FILENAME[];
	extern const char REGISTRY_FILENAME[];

	template< typename T >
	void DeleteContainer( T& container )
	{
		for( T::iterator i = container.begin(); i != container.end(); ++i )
			delete *i;
		container.clear();
	}

	class nil_buf : public std::streambuf  
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

	class nil_wbuf : public std::wstreambuf  
	{
	public:
		nil_wbuf();
		virtual	~nil_wbuf();

	protected:
		virtual int	overflow( int c_ );
		virtual int sync();
		void put_buffer();
		void put_char( int c_ );
	};

}
