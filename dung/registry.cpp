#include "registry.h"

#include <string>
#include "string.h"

#define CHAR_WCHAR(x) x, L ## x

namespace dung
{
	struct StringsForAction
	{
		Action::Enum action;
		const char* str;
		const wchar_t* wstr;
	};

	StringsForAction g_stringsForAction[] =
	{
		  Action::NEW, CHAR_WCHAR("new")
		, Action::DELETE, CHAR_WCHAR("delete")
		, Action::MOVE, CHAR_WCHAR("move")
		, Action::APPLY_DIFF, CHAR_WCHAR("apply_diff")
		, Action::NONE, CHAR_WCHAR("none")
		, Action::NEW_BUT_NOT_INCLUDED, CHAR_WCHAR("new_but_not_included")
		, Action::OVERRIDE, CHAR_WCHAR("override")
	};
}

std::string dung::ActionToString( Action::Enum action )
{
	SCARAB_ASSERT( g_stringsForAction[action].action == action );
	return g_stringsForAction[action].str;
}

std::wstring dung::ActionToWString( Action::Enum action )
{
	SCARAB_ASSERT( g_stringsForAction[action].action == action );
	return g_stringsForAction[action].wstr;
}

bool dung::StringToAction( const wchar_t* parse, Action::Enum& action )
{
	for( int i = 0; i < sizeof(g_stringsForAction)/sizeof(g_stringsForAction[0]); ++i )
	{
		if( wcscmp( g_stringsForAction[i].wstr, parse ) == 0 )
		{
			action = g_stringsForAction[i].action;
			return true;
		}
	}

	return false;
}

bool dung::StringToAction( const char* parse, Action::Enum& action )
{
	const size_t size = strlen(parse)+1;
	std::wstring wparse( size, L' ' );
	mbstowcs( &wparse[0], parse, size );

	return StringToAction( wparse.c_str(), action );
}
