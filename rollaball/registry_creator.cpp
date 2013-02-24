#include "registry_creator.h"

#include "filetree.h"

#include <dung/memoryblock.h>
#include <dung/registry.h>
#include <zlib/minizip.h>

#include <fstream>

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

	void WriteRegistryFiles( Options const& options, Config const& config, FolderInfo::FileInfos_t const& fileInfos, dung::Action::Enum action, Path_t const& relativePath, OutputContext& output );
	void WriteRegistryFolders( Options const& options, Config const& config, FolderInfo::FolderInfos_t const& folderInfos, Path_t const& relativePath, OutputContext& output );

	const _TCHAR endl = '\n';
	const _TCHAR quote = '\"';
}

rab::OutputContext::OutputContext( OutputStream_t& stream )
	: stream( stream )
{
}

void rab::WriteRegistryFiles( Options const& options, Config const& config, 
	FolderInfo::FileInfos_t const& fileInfos, dung::Action::Enum action, Path_t const& relativePath, OutputContext& output )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo const& fileInfo = **i;
		dung::Action::Enum fileAction = fileInfo.isDifferent ? dung::Action::APPLY_DIFF : action;

		output.stream << _T("file") << endl;
		output.stream << _T("{") << endl;

		output.stream << _T("\t") << _T("action=") << dung::ActionToString(fileAction) << endl;

		if( fileInfo.newSize >= 0 && fileAction != dung::Action::NONE )
		{
			output.stream << _T("\t") << _T("new_path=") << quote << ( relativePath / fileInfo.name ).generic_wstring() << quote << endl;
			output.stream << _T("\t") << _T("new_size=") << fileInfo.newSize << endl;
			output.stream << _T("\t") << _T("new_sha1=") << quote << dung::SHA1ToString(fileInfo.newSha1) << quote << endl;
		}

		if( fileInfo.oldSize >= 0 )
		{
			output.stream << _T("\t") << _T("old_path=") << quote << ( relativePath / fileInfo.name ).generic_wstring() << quote << endl;
			output.stream << _T("\t") << _T("old_size=") << fileInfo.oldSize << endl;
			output.stream << _T("\t") << _T("old_sha1=") << quote << dung::SHA1ToString(fileInfo.oldSha1) << quote << endl;
		}

		if( fileInfo.isDifferent )
			output.stream << _T("\t") << _T("diff_path=") << quote << ( relativePath / DiffFileName(fileInfo.name, config) ).generic_wstring() << quote << endl;

		output.stream << _T("}") << endl;
	}
}

void rab::WriteRegistryFolders( Options const& options, Config const& config, FolderInfo::FolderInfos_t const& folderInfos, Path_t const& relativePath, OutputContext& output )
{
	for( FolderInfo::FolderInfos_t::const_iterator i = folderInfos.begin(); i != folderInfos.end(); ++i )
	{
		FolderInfo const& folderInfo = **i;

		Path_t nextRelativePath = relativePath / folderInfo.name;

		WriteRegistryFiles( options, config,  folderInfo.files_existInBoth, dung::Action::NONE, nextRelativePath, output );
		WriteRegistryFiles( options, config, folderInfo.files_newOnly, dung::Action::NEW, nextRelativePath, output );
		WriteRegistryFiles( options, config, folderInfo.files_oldOnly, dung::Action::DELETE, nextRelativePath, output );

		WriteRegistryFolders( options, config, folderInfo.folders_existInBoth, nextRelativePath, output );
		WriteRegistryFolders( options, config, folderInfo.folders_newOnly, nextRelativePath, output );
		WriteRegistryFolders( options, config, folderInfo.folders_oldOnly, nextRelativePath, output );
	}
}

bool rab::WriteRegistry( Options const& options, Config const& config, FolderInfo& rootFolder, PackageOutput_t& output )
{
	std::wofstream file ( REGISTRY_FILENAME, std::ios::out|std::ios::trunc );
	if( !file.is_open() )
		return false;

	OutputContext os( file );

	os.stream << _T("new_version=") << quote << options.newVersion << quote << endl;
	os.stream << _T("old_version=") << quote << options.oldVersion << quote << endl;

	Path_t relativePath;
	WriteRegistryFolders( options, config, FolderInfo::FolderInfos_t( 1, &rootFolder ), relativePath, os );

	file.close();

	dung::MemoryBlock registryFileContent;
	if( !dung::ReadWholeFile( REGISTRY_FILENAME, registryFileContent ) )
		return false;

	output.WriteFile( REGISTRY_FILENAME, registryFileContent.pBlock, registryFileContent.size );

	return true;
}
