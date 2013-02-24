#pragma once

#include "hatchout.h"
#include "registry_parser.h"

namespace zip
{
	class ZipArchiveInput;
}

namespace hatch
{
	bool ApplyActions( Options const& options, Registry const& registry, zip::ZipArchiveInput& zipInput, LogOutput_t& out );
}
