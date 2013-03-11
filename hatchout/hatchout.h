#pragma once

#include <dung/dung.h>

#include <string>

namespace hatch
{
	class DiffDecoders;
}

namespace hatch
{
	typedef _tstring String_t;
	typedef _tostream LogOutput_t;
	typedef _tstringstream StringStream_t;

	struct Options
	{
		String_t pathToPackage;
		String_t pathToOld;

		bool quiet;
		bool reportFile;
		bool verbose;
		bool stopIfError;
		bool checkOldSize;
		bool checkOldSha1;
	};

	class HatchOut
	{
	public:
		HatchOut();
		~HatchOut();

		bool ProcessData( Options const& options, DiffDecoders const& diffDecoders, LogOutput_t& out );
	};
}
