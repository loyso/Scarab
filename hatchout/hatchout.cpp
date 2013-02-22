#include "hatchout.h"

#include <dung/memoryblock.h>
#include <zlib/miniunzip.h>

void hatch::ProcessData( Options const& options )
{
	zip::ZipArchiveInput zipInput( options.pathToPackage );

	dung::MemoryBlock registryContent;
	zipInput.ReadFile( "registry.txt", registryContent.pBlock, registryContent.size );

	zipInput.Close();
}
