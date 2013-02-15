#pragma once

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
#else
#	define	_TCHAR char
#	define _T(x) x
#	define _tmain main
#endif