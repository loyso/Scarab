#include "rollaball.h"

#include "filetree.h"

#include "filecopy.h"
#include "filediff.h"

void rab::ProcessData( Options const& options, Config const& config )
{
	FolderInfo* pRootFolder = SCARAB_NEW FolderInfo();
	pRootFolder->name = _T("./");

	BuildFileTree( options.pathToNew, options.pathToOld, *pRootFolder );

	BuildTempCopies( options, config, *pRootFolder );
	BuildDiffs( options, config, *pRootFolder );

	delete pRootFolder;
	pRootFolder = NULL;
}

