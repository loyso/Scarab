#pragma once

#include <vector>

#include <dung/sha1.h>

#include "rollaball.h"

namespace rab
{
	struct FileInfo
	{
		FileInfo();

		String_t name;
		
		dung::Sha1 newSha1, oldSha1;
		size_t newSize, oldSize;

		DiffMethod_t diffMethod;
		bool isDifferent;
	};

	struct FolderInfo 
	{
		~FolderInfo();

		typedef std::vector< FolderInfo* > FolderInfos_t;
		typedef std::vector< FileInfo* > FileInfos_t;

		String_t name;

		FolderInfos_t folders_newOnly;
		FolderInfos_t folders_oldOnly;
		FolderInfos_t folders_existInBoth;

		FileInfos_t files_newOnly;
		FileInfos_t files_oldOnly;
		FileInfos_t files_existInBoth;
	};

	void BuildFileTree( Options const& options, Config const& config, FolderInfo& rootFolder );
}