#pragma once

#include <dung/dung.h>

#include <string>

namespace hatch
{
	class DiffDecoders;
}

namespace hatch
{
	typedef std::string String_t;
	typedef std::ostream LogOutput_t;

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
