#include "hatchout.h"

#include <dung/memoryblock.h>
#include <zlib/miniunzip.h>

#include "registry_parser.h"
#include "apply_actions.h"

hatch::HatchOut::HatchOut()
{
}

hatch::HatchOut::~HatchOut()
{
}

bool hatch::HatchOut::ProcessData( Options const& options, DiffDecoders const& diffDecoders, LogOutput_t& out )
{
	if( diffDecoders.Empty() )
	{
		out << "No decoders added!" << std::endl;
		return false;
	}

	zip::ZipArchiveInput zipInput;
	if( !zipInput.Open( options.pathToPackage, false ) )
	{
		out << "Can't open zip archive " << options.pathToPackage << ". Zip error: " << zipInput.ErrorMessage() << std::endl;
		return false;
	}

	out << "Opened " << options.pathToPackage << std::endl;

	dung::MemoryBlock registryContent;
	if( !zipInput.ReadFile( dung::REGISTRY_FILENAME, registryContent.pBlock, registryContent.size ) )
	{
		out << "Can't read file " << dung::REGISTRY_FILENAME << " from zip. Zip error: " << zipInput.ErrorMessage() << std::endl;
		return false;
	}

	Registry registry;
	{
		RegistryParser parser;
		parser.Open( (const char*)registryContent.pBlock, registryContent.size );
		if( !parser.Parse( registry ) )
		{
			out << "Can't parse file " << dung::REGISTRY_FILENAME << ". parse error: " << parser.ErrorMessage() << std::endl;
			return false;
		}
		parser.Close();
	}

	out << "Parsed " << dung::REGISTRY_FILENAME << std::endl;
	if( options.verbose )
		out << registry.actions.size() << " actions in total." << std::endl;

	bool result = ApplyActions( options, diffDecoders, registry, zipInput, out );

	if( result )
		out << "Successfully done!" << std::endl;

	zipInput.Close();
	
	return result;
}
