#pragma once

#pragma once

#include "rollaball.h"

namespace rab
{
	struct FolderInfo;
	struct Config;
}

namespace rab
{
	bool GatherSha1( Options const& options, Config const& config, FolderInfo const& rootFolder, LogOutput_t& out );
}
