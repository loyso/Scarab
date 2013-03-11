#include "dung.h"

namespace dung
{
	namespace Action
	{
		enum Enum
		{
			  NEW = 0
			, DELETE
			, MOVE
			, APPLY_DIFF
			, NONE
			, NEW_BUT_NOT_INCLUDED
			, OVERRIDE
		};
	}

	std::string ActionToString( Action::Enum action );
	std::wstring ActionToWString( Action::Enum action );

	bool StringToAction( const char* parse, Action::Enum& action );
	bool StringToAction( const wchar_t* parse, Action::Enum& action );
}
