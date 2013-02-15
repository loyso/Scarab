#include "filetree.h"

#include <unordered_set>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef std::unordered_set< String_t > StringSet_t;
	typedef std::vector< String_t > VectorString_t;
	typedef fs::path Path_t;

	void BuildFileTree_Rec( Path_t const& pathToNew, Path_t const& pathToOld, FolderInfo& rootFolder );
	void BuildFileTreeForFolder_Rec( Path_t const& pathToNew, Path_t const& pathToOld, FolderInfo::FolderInfos_t const& folderInfos );

	void BuildFilesAndFoldersForPath( Path_t const& path, StringSet_t& files, StringSet_t& folders );

	void BuildThreeSets( StringSet_t& newSet, StringSet_t& oldSet, 
		VectorString_t& existInBoth, VectorString_t& newOnly, VectorString_t& oldOnly );

	void CreateFilesInTree( VectorString_t const& v, FolderInfo::FileInfos_t& fileInfos );
	void CreateFoldersInTree( VectorString_t const& v, FolderInfo::FolderInfos_t& folderInfos );

	template< typename T >
	void DeleteContainer( T& container )
	{
		for( T::iterator i = container.begin(); i != container.end(); ++i )
			delete *i;
		container.clear();
	}
}


rab::FolderInfo::~FolderInfo()
{
	DeleteContainer( m_folders_newOnly );
	DeleteContainer( m_folders_oldOnly );
	DeleteContainer( m_folders_existInBoth );

	DeleteContainer( m_files_newOnly );
	DeleteContainer( m_files_oldOnly );
	DeleteContainer( m_files_existInBoth );
}

void rab::BuildFilesAndFoldersForPath( Path_t const& path, StringSet_t& files, StringSet_t& folders )
{
	for( fs::recursive_directory_iterator it( path ); it != fs::recursive_directory_iterator(); ++it )
	{		
		fs::file_status status = it->status();
		fs::path fullpath = it->path();

		if( fs::is_regular_file( status ))
		{
			files.insert( fullpath.filename().wstring() );
		}
		else if( fs::is_directory( status ) )
		{
			folders.insert( fullpath.filename().wstring() );
		}
	}
}

void rab::BuildThreeSets( StringSet_t& newSet, StringSet_t& oldSet, 
	VectorString_t& existInBoth, VectorString_t& newOnly, VectorString_t& oldOnly )
{
	existInBoth.clear();
	newOnly.clear();
	oldOnly.clear();

	for( StringSet_t::iterator i = newSet.begin(); i != newSet.end(); ++i )
	{
		const String_t& name = *i;
		StringSet_t::iterator f = oldSet.find( name );
		if( f != oldSet.end() )
		{
			existInBoth.push_back( name );
			oldSet.erase( f );
		}
		else
		{
			newOnly.push_back( name );
		}
	}

	oldOnly.assign( oldSet.begin(), oldSet.end() );

	oldSet.clear();
	newSet.clear();
}

void rab::CreateFilesInTree( VectorString_t const& names, FolderInfo::FileInfos_t& fileInfos )
{
	for( VectorString_t::const_iterator i = names.begin(); i != names.end(); ++i )
	{
		FileInfo* pFileInfo = SCARAB_NEW FileInfo();
		pFileInfo->m_name = *i;
		fileInfos.push_back( pFileInfo );
	}
}

void rab::CreateFoldersInTree( VectorString_t const& names, FolderInfo::FolderInfos_t& folderInfos )
{
	for( VectorString_t::const_iterator i = names.begin(); i != names.end(); ++i )
	{
		FolderInfo* pFolderInfo = SCARAB_NEW FolderInfo();
		pFolderInfo->m_name = *i;
		folderInfos.push_back( pFolderInfo );
	}
}

void rab::BuildFileTreeForFolder_Rec( Path_t const& pathToNew, Path_t const& pathToOld, FolderInfo::FolderInfos_t const& folderInfos )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;
		BuildFileTree_Rec( pathToNew / folderInfo.m_name, pathToOld / folderInfo.m_name, folderInfo );
	}
}

void rab::BuildFileTree_Rec( Path_t const& pathToNew, Path_t const& pathToOld, FolderInfo& folderInfo )
{
	StringSet_t newFiles, oldFiles;
	StringSet_t newFolders, oldFolders;

	BuildFilesAndFoldersForPath( pathToOld, oldFiles, oldFolders );
	BuildFilesAndFoldersForPath( pathToNew, newFiles, newFolders );

	// files
	{
		VectorString_t existInBoth, newOnly, oldOnly;
		BuildThreeSets( newFiles, oldFiles, existInBoth, newOnly, oldOnly );

		CreateFilesInTree( existInBoth, folderInfo.m_files_existInBoth );
		CreateFilesInTree( newOnly, folderInfo.m_files_newOnly );
		CreateFilesInTree( oldOnly, folderInfo.m_files_oldOnly );
	}
	// folders
	{
		VectorString_t existInBoth, newOnly, oldOnly;
		BuildThreeSets( newFolders, oldFolders, existInBoth, newOnly, oldOnly );

		CreateFoldersInTree( existInBoth, folderInfo.m_folders_existInBoth );
		CreateFoldersInTree( newOnly, folderInfo.m_folders_newOnly );
		CreateFoldersInTree( oldOnly, folderInfo.m_folders_oldOnly );
	}

	BuildFileTreeForFolder_Rec( pathToNew, pathToOld, folderInfo.m_folders_existInBoth );
	BuildFileTreeForFolder_Rec( pathToNew, pathToOld, folderInfo.m_folders_newOnly );
	BuildFileTreeForFolder_Rec( pathToNew, pathToOld, folderInfo.m_folders_oldOnly );
}

void rab::BuildFileTree( String_t const& pathToNew, String_t const& pathToOld, FolderInfo& rootFolder )
{
	BuildFileTree_Rec( pathToNew, pathToOld, rootFolder );
}

