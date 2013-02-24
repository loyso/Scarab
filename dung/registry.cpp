#include "registry.h"

#include <string>
#include "string.h"

namespace dung
{
	struct StringsForAction
	{
		Action::Enum action;
		const _TCHAR* str;
	};

	StringsForAction g_stringsForAction[] =
	{
		  Action::NEW, _T("new")
		, Action::DELETE, _T("delete")
		, Action::MOVE, _T("move")
		, Action::APPLY_DIFF, _T("apply_diff")
		, Action::NONE, _T("none")
		, Action::NEW_BUT_NOT_INCLUDED, _T("new_but_not_included")
	};
}

const _TCHAR* dung::ActionToString( Action::Enum action )
{
	SCARAB_ASSERT( g_stringsForAction[action].action == action );
	return g_stringsForAction[action].str;
}

bool dung::StringToAction( const char* parse, Action::Enum& action )
{
	const size_t size = strlen(parse)+1;
	std::wstring wparse( size, L' ' );
	mbstowcs( &wparse[0], parse, size );

	for( int i = 0; i < sizeof(g_stringsForAction)/sizeof(g_stringsForAction[0]); ++i )
	{
		if( wcscmp( g_stringsForAction[i].str, wparse.c_str() ) == 0 )
		{
			action = g_stringsForAction[i].action;
			return true;
		}
	}

	return false;
}
