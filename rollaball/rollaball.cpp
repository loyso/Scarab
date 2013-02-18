#include "rollaball.h"

#include "filetree.h"

#include "filecopy.h"
#include "filediff.h"
#include "filesha1.h"

#include "registry.h"

void rab::ProcessData( Options const& options, Config const& config )
{
	FolderInfo* pRootFolder = SCARAB_NEW FolderInfo();

	BuildFileTree( options.pathToNew, options.pathToOld, *pRootFolder );

	BuildTempCopies( options, config, *pRootFolder );
	BuildDiffs( options, config, *pRootFolder );
	GatherSha1( options, config, *pRootFolder );

	WriteRegistry( options, config, *pRootFolder, options.registryFile );

	delete pRootFolder;
	pRootFolder = NULL;
}

rab::String_t rab::DiffFileName( String_t const& fileName, Config const& config )
{
	return fileName + _T(".") + config.packedExtension;
}


