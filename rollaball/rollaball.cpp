#include "rollaball.h"
#include "filetree.h"

void rab::ProcessData( Options const& options, Config const& config )
{
	FolderInfo* pRootFolder = SCARAB_NEW FolderInfo();
	pRootFolder->m_name = _T("./");

	BuildFileTree( options.src, options.dst, *pRootFolder );
}

