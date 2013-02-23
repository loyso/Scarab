#include "hatchout.h"

#include <dung/memoryblock.h>
#include <zlib/miniunzip.h>

#include "registry_parser.h"

void hatch::ProcessData( Options const& options )
{
	zip::ZipArchiveInput zipInput( options.pathToPackage );

	dung::MemoryBlock registryContent;
	zipInput.ReadFile( "registry.txt", registryContent.pBlock, registryContent.size );

	Registry registry;
	{
		RegistryParser parser;
		parser.Open( (const char*)registryContent.pBlock, registryContent.size );
		parser.Parse( registry );
		parser.Close();
	}

	zipInput.Close();
}
