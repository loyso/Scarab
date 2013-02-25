#include "hatchout.h"

#include <dung/memoryblock.h>
#include <zlib/miniunzip.h>

#include "registry_parser.h"
#include "apply_actions.h"

bool hatch::ProcessData( Options const& options, LogOutput_t& out )
{
	zip::ZipArchiveInput zipInput;
	if( !zipInput.Open( options.pathToPackage ) )
	{
		if( !options.quiet )
			out << "Can't open zip archive " << options.pathToPackage << " zip error: " << zipInput.ErrorMessage() << std::endl;
		return false;
	}

	if( !options.quiet )
		out << "Opened " << options.pathToPackage << std::endl;

	dung::MemoryBlock registryContent;
	const char registryFileName[] = "registry.txt";
	if( !zipInput.ReadFile( registryFileName, registryContent.pBlock, registryContent.size ) )
	{
		if( !options.quiet )
			out << "Can't read file " << registryFileName << " from zip. zip error: " << zipInput.ErrorMessage() << std::endl;
		return false;
	}

	Registry registry;
	{
		RegistryParser parser;
		parser.Open( (const char*)registryContent.pBlock, registryContent.size );
		if( !parser.Parse( registry ) )
		{
			if( !options.quiet )
				out << "Can't parse file " << registryFileName << ". parse error: " << parser.ErrorMessage() << std::endl;
			return false;
		}
		parser.Close();
	}

	if( !options.quiet )
	{
		out << "Parsed " << registryFileName << std::endl;
		if( options.verbose )
			out << registry.actions.size() << " actions in total." << std::endl;
	}

	bool result = ApplyActions( options, registry, zipInput, out );

	if( result && !options.quiet )
		out << "Successfully done!" << std::endl;

	zipInput.Close();
	
	return result;
}
