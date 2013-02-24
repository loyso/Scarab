#include "rollaball.h"

#include "filetree.h"

#include "filecopy.h"
#include "filediff.h"
#include "filesha1.h"

#include "registry_creator.h"

#include <zlib/minizip.h>

void rab::ProcessData( Options const& options, Config const& config )
{
	FolderInfo* pRootFolder = SCARAB_NEW FolderInfo();

	BuildFileTree( options.pathToNew, options.pathToOld, *pRootFolder );

	zip::ZipArchiveOutput zipOut( options.packageFile, true, zip::CompressionLevel::NO_COMPRESSION );

	BuildTempCopies( options, config, *pRootFolder, zipOut );
	BuildDiffs( options, config, *pRootFolder, zipOut );
	GatherSha1( options, config, *pRootFolder );

	WriteRegistry( options, config, *pRootFolder, zipOut );	

	zipOut.Close();

	delete pRootFolder;
	pRootFolder = NULL;
}

rab::String_t rab::DiffFileName( String_t const& fileName, Config const& config )
{
	return fileName + _T(".") + config.packedExtension;
}


