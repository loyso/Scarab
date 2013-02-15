#pragma once

#include <vector>

#include "rollaball.h"

namespace rab
{
	struct FileInfo
	{
		String_t m_name;
	};

	struct FolderInfo 
	{
		String_t m_name;

		typedef std::vector< FolderInfo* > FolderInfos_t;
		typedef std::vector< FileInfo* > FileInfos_t;

		FolderInfos_t m_folders_newOnly;
		FolderInfos_t m_folders_oldOnly;
		FolderInfos_t m_folders_existInBoth;

		FileInfos_t m_files_newOnly;
		FileInfos_t m_files_oldOnly;
		FileInfos_t m_files_existInBoth;
	};

	void BuildFileTree( String_t const& pathToNew, String_t const& pathToOld, FolderInfo& rootFolder );
}