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
	void GatherSha1( Options const& options, Config const& config, FolderInfo const& rootFolder );
}
