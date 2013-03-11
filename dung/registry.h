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

	_tstring ActionToString( Action::Enum action );
	bool StringToAction( const _TCHAR* parse, Action::Enum& action );
}
