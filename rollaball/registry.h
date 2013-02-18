#pragma once

#include "rollaball.h"

namespace rab
{
	struct FolderInfo;
}

namespace rab
{
	bool WriteRegistry( Options const& options, Config const& config, FolderInfo& rootFolder, String_t const& filePath );
}
