#include "dung.h"

namespace dung
{
	static const _TCHAR REGISTRY_FILENAME[] = _T("registry.txt");
}

dung::nil_buf::nil_buf()
{
}

dung::nil_buf::~nil_buf()
{
}

int dung::nil_buf::overflow( int c_ )
{
	return 0;
}

int dung::nil_buf::sync()
{
	return 0;
}

void dung::nil_buf::put_buffer()
{
}

void dung::nil_buf::put_char( int c_ )
{
}

size_t dung::StrLen( const char* str )
{
	return strlen( str );
}

size_t dung::StrLen( const wchar_t* str )
{
	return wcslen( str );
}
