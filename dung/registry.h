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
		};
	}

	const _TCHAR* ActionToString( Action::Enum action );
	bool StringToAction( const char* parse, Action::Enum& action );
}
