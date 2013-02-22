#pragma once

#include <dung/dung.h>

#include <string>

namespace hatch
{
	typedef std::string String_t;

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

	void ProcessData( Options const& options );
}
