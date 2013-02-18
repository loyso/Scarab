#include "registry.h"

#include "filetree.h"

#include <fstream>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	typedef fs::path Path_t;
	typedef std::wostream OutputStream_t;

	struct OutputContext
	{
		OutputContext( OutputStream_t& stream );

		OutputStream_t& stream;
	};

	namespace Action
	{
		enum Enum
		{
			  NEW
			, DELETE
			, MOVE
			, APPLY_DIFF
			, NONE
			, NEW_BUT_NOT_INCLUDED
		};
	}

	String_t ActionToString( Action::Enum action );

	void WriteRegistryFiles( Options const& options, Config const& config, FolderInfo::FileInfos_t const& fileInfos, Action::Enum action, Path_t const& relativePath, OutputContext& output );
	void WriteRegistryFolders( Options const& options, Config const& config, FolderInfo::FolderInfos_t const& folderInfos, Path_t const& relativePath, OutputContext& output );
}

rab::OutputContext::OutputContext( OutputStream_t& stream )
	: stream( stream )
{
}

rab::String_t rab::ActionToString( Action::Enum action )
{
	switch( action )
	{
		case Action::NEW: return _T("new");
		case Action::DELETE: return _T("delete");
		case Action::MOVE: return _T("move");
		case Action::APPLY_DIFF: return _T("apply_diff");
		case Action::NONE: return _T("none");
		case Action::NEW_BUT_NOT_INCLUDED: return _T("new_but_not_included");
	}

	SCARAB_ASSERT( false && _T("unknown Action enum entry") );
	return _T("unknown");
}


void rab::WriteRegistryFiles( Options const& options, Config const& config, 
	FolderInfo::FileInfos_t const& fileInfos, Action::Enum action, Path_t const& relativePath, OutputContext& output )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo const& fileInfo = **i;
		Action::Enum fileAction = fileInfo.isDifferent ? Action::APPLY_DIFF : action;

		output.stream << _T("file") << std::endl;
		output.stream << _T("{") << std::endl;

		if( fileAction != Action::DELETE  )
		{
			output.stream << _T("\t") << _T("src_path=") << ( relativePath / fileInfo.name ).generic_wstring() << std::endl;
			output.stream << _T("\t") << _T("src_size=") << fileInfo.newSize << std::endl;
			output.stream << _T("\t") << _T("src_sha1=") << SHA1ToString(fileInfo.newSha1) << std::endl;
		}

		if( fileAction != Action::NEW && fileAction != Action::NEW_BUT_NOT_INCLUDED )
		{
			output.stream << _T("\t") << _T("dst_path=") << ( relativePath / fileInfo.name ).generic_wstring() << std::endl;
			output.stream << _T("\t") << _T("dst_size=") << fileInfo.oldSize << std::endl;
			output.stream << _T("\t") << _T("dst_sha1=") << SHA1ToString(fileInfo.oldSha1) << std::endl;
		}

		if( fileInfo.isDifferent )
			output.stream << _T("\t") << _T("diff_path=") << ( relativePath / DiffFileName(fileInfo.name, config) ).generic_wstring() << std::endl;

		output.stream << _T("\t") << _T("action=") << ActionToString(fileAction) << std::endl;
		output.stream << _T("}") << std::endl;
	}
}

void rab::WriteRegistryFolders( Options const& options, Config const& config, FolderInfo::FolderInfos_t const& folderInfos, Path_t const& relativePath, OutputContext& output )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo const& folderInfo = **i;

		Path_t nextRelativePath = relativePath / folderInfo.name;

		WriteRegistryFiles( options, config,  folderInfo.files_existInBoth, Action::NONE, nextRelativePath, output );
		WriteRegistryFiles( options, config, folderInfo.files_newOnly, Action::NEW, nextRelativePath, output );
		WriteRegistryFiles( options, config, folderInfo.files_oldOnly, Action::DELETE, nextRelativePath, output );

		WriteRegistryFolders( options, config, folderInfo.folders_existInBoth, nextRelativePath, output );
		WriteRegistryFolders( options, config, folderInfo.folders_newOnly, nextRelativePath, output );
		WriteRegistryFolders( options, config, folderInfo.folders_oldOnly, nextRelativePath, output );
	}
}

bool rab::WriteRegistry( Options const& options, Config const& config, FolderInfo& rootFolder, String_t const& filePath )
{
	std::locale loc = boost::locale::generator().generate(""); // system + utf-8 by default
	std::wofstream file ( filePath, std::ios::out|std::ios::trunc );
	file.imbue(loc);
	if( !file.is_open() )
		return false;

	OutputContext output( file );

	output.stream << _T("source_version=") << options.newVersion << std::endl;
	output.stream << _T("dest_version=") << options.oldVersion << std::endl;

	Path_t relativePath;
	WriteRegistryFolders( options, config, FolderInfo::FolderInfos_t( 1, &rootFolder ), relativePath, output );

	file.close();
	return true;
}
