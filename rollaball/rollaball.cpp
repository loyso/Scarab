#include "rollaball.h"

#include "filetree.h"
#include "filediff.h"

void rab::ProcessData( Options const& options, Config const& config )
{
	FolderInfo* pRootFolder = SCARAB_NEW FolderInfo();
	pRootFolder->m_name = _T("./");

	BuildFileTree( options.pathToNew, options.pathToOld, *pRootFolder );
	BuildDiffs( options, config, *pRootFolder );

	delete pRootFolder;
	pRootFolder = NULL;
}

