#include "filetree.h"

#include <unordered_set>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef std::unordered_set< String_t > StringSet_t;
	typedef std::vector< String_t > VectorString_t;
	typedef fs::path Path_t;

	void BuildFileTree_Rec( Config const& config, Path_t const& pathToNew, Path_t const& pathToOld, 
		FolderInfo& rootFolder, LogOutput_t& out );

	void BuildFileTreeForFolder_Rec( Config const& config, Path_t const& pathToNew, Path_t const& pathToOld, 
		FolderInfo::FolderInfos_t const& folderInfos, LogOutput_t& out );

	void BuildFilesAndFoldersForPath( Config const& config, Path_t const& path, StringSet_t& files, StringSet_t& folders, bool newPath );

	void BuildThreeSets( Config const& config, StringSet_t& newSet, StringSet_t& oldSet, 
		VectorString_t& existInBoth, VectorString_t& newOnly, VectorString_t& oldOnly );

	void CreateFilesInTree( VectorString_t const& v, FolderInfo::FileInfos_t& fileInfos );
	void CreateFoldersInTree( VectorString_t const& v, FolderInfo::FolderInfos_t& folderInfos );
}


rab::FileInfo::FileInfo()
	: newSize()
	, oldSize()
	, isDifferent( false )
{
}

rab::FolderInfo::~FolderInfo()
{
	dung::DeleteContainer( folders_newOnly );
	dung::DeleteContainer( folders_oldOnly );
	dung::DeleteContainer( folders_existInBoth );

	dung::DeleteContainer( files_newOnly );
	dung::DeleteContainer( files_oldOnly );
	dung::DeleteContainer( files_existInBoth );
}

void rab::BuildFilesAndFoldersForPath( Config const& config, Path_t const& path, StringSet_t& files, StringSet_t& folders, bool newPath )
{
	for( fs::directory_iterator it( path ); it != fs::directory_iterator(); ++it )
	{		
		fs::file_status status = it->status();
		fs::path fullpath = it->path();
		String_t name = GenericString( fullpath.filename() );

		if( fs::is_regular_file( status ))
		{
			if( ( config.includeFiles_regex.empty() || MatchName( config.includeFiles_regex, name ) ) 
				&& !MatchName( config.ignoreFiles_regex, name ) )
			{
				if( !newPath || !MatchName( config.newIgnoreFiles_regex, name ) )
					files.insert( FileString( fullpath.filename() ) );
			}
		}
		else if( fs::is_directory( status ) )
		{
			if( ( config.includeFolders_regex.empty() || MatchName( config.includeFolders_regex, name ) ) 
				&& !MatchName( config.ignoreFolders_regex, name ) )
			{
				if( !newPath || !MatchName( config.newIgnoreFolders_regex, name ) )
					folders.insert( FileString( fullpath.filename() ) );
			}
		}
	}
}

void rab::BuildThreeSets( Config const& config, StringSet_t& newSet, StringSet_t& oldSet, 
	VectorString_t& existInBoth, VectorString_t& newOnly, VectorString_t& oldOnly )
{
	existInBoth.clear();
	newOnly.clear();
	oldOnly.clear();

	for( StringSet_t::iterator i = newSet.begin(); i != newSet.end(); ++i )
	{
		const String_t& name = *i;

		bool overrideFile = MatchName( config.newOverrideFiles_regex, name );

		StringSet_t::iterator f = oldSet.find( name );
		if( f != oldSet.end() )
		{
			if( overrideFile )
				newOnly.push_back( name );
			else
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
		pFileInfo->name = *i;
		fileInfos.push_back( pFileInfo );
	}
}

void rab::CreateFoldersInTree( VectorString_t const& names, FolderInfo::FolderInfos_t& folderInfos )
{
	for( VectorString_t::const_iterator i = names.begin(); i != names.end(); ++i )
	{
		FolderInfo* pFolderInfo = SCARAB_NEW FolderInfo();
		pFolderInfo->name = *i;
		folderInfos.push_back( pFolderInfo );
	}
}

void rab::BuildFileTreeForFolder_Rec( Config const& config, Path_t const& pathToNew, Path_t const& pathToOld, FolderInfo::FolderInfos_t const& folderInfos, LogOutput_t& out )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo& folderInfo = **i;
		BuildFileTree_Rec( config, pathToNew / folderInfo.name, pathToOld / folderInfo.name, folderInfo, out );
	}
}

void rab::BuildFileTree_Rec( Config const& config, Path_t const& pathToNew, Path_t const& pathToOld, FolderInfo& folderInfo, LogOutput_t& out )
{
	StringSet_t newFiles, oldFiles;
	StringSet_t newFolders, oldFolders;

	if( fs::exists( pathToOld ) )
		BuildFilesAndFoldersForPath( config, pathToOld, oldFiles, oldFolders, false );

	if( fs::exists( pathToNew ) )
		BuildFilesAndFoldersForPath( config, pathToNew, newFiles, newFolders, true );

	// files
	{
		VectorString_t existInBoth, newOnly, oldOnly;
		BuildThreeSets( config, newFiles, oldFiles, existInBoth, newOnly, oldOnly );

		CreateFilesInTree( existInBoth, folderInfo.files_existInBoth );
		CreateFilesInTree( newOnly, folderInfo.files_newOnly );
		CreateFilesInTree( oldOnly, folderInfo.files_oldOnly );
	}
	// folders
	{
		VectorString_t existInBoth, newOnly, oldOnly;
		BuildThreeSets( config, newFolders, oldFolders, existInBoth, newOnly, oldOnly );

		CreateFoldersInTree( existInBoth, folderInfo.folders_existInBoth );
		CreateFoldersInTree( newOnly, folderInfo.folders_newOnly );
		CreateFoldersInTree( oldOnly, folderInfo.folders_oldOnly );
	}

	BuildFileTreeForFolder_Rec( config, pathToNew, pathToOld, folderInfo.folders_existInBoth, out );
	BuildFileTreeForFolder_Rec( config, pathToNew, pathToOld, folderInfo.folders_newOnly, out );
	BuildFileTreeForFolder_Rec( config, pathToNew, pathToOld, folderInfo.folders_oldOnly, out );
}

void rab::BuildFileTree( Options const& options, Config const& config, FolderInfo& rootFolder, LogOutput_t& out )
{
	out << "Gathering all names in new and old directories..." << std::endl;
	BuildFileTree_Rec( config, options.pathToNew, options.pathToOld, rootFolder, out );
}

