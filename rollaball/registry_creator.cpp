#include "registry_creator.h"

#include "filetree.h"

#include <dung/memoryblock.h>
#include <dung/registry.h>
#include <zlib/minizip.h>

#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <boost/locale.hpp>
namespace loc = boost::locale;

namespace rab
{
	typedef fs::path Path_t;
	typedef _tostream OutputStream_t;

	struct OutputContext
	{
		OutputContext( OutputStream_t& stream, LogOutput_t& out );

		OutputStream_t& stream;
		LogOutput_t& out;
	};

	void WriteRegistryFiles( Options const& options, Config const& config, FolderInfo::FileInfos_t const& fileInfos, dung::Action::Enum action, Path_t const& relativePath, OutputContext& output );
	void WriteRegistryFolders( Options const& options, Config const& config, FolderInfo::FolderInfos_t const& folderInfos, Path_t const& relativePath, OutputContext& output );

	const _TCHAR endl = '\n';
	const _TCHAR quote = '\"';
}

rab::OutputContext::OutputContext( OutputStream_t& stream, LogOutput_t& out )
	: stream( stream )
	, out( out )
{
}

void rab::WriteRegistryFiles( Options const& options, Config const& config, 
	FolderInfo::FileInfos_t const& fileInfos, dung::Action::Enum action, Path_t const& relativePath, OutputContext& output )
{
	for( FolderInfo::FileInfos_t::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i )
	{
		FileInfo const& fileInfo = **i;
		dung::Action::Enum fileAction = fileInfo.isDifferent ? dung::Action::APPLY_DIFF : action;

		if( fileAction == dung::Action::NEW && config.newFileLimit > 0 && fileInfo.newSize >= config.newFileLimit )
			fileAction = dung::Action::NEW_BUT_NOT_INCLUDED;

		if( fileAction == dung::Action::NEW && MatchName( config.newOverrideFiles_regex, fileInfo.name ) )
			fileAction = dung::Action::OVERRIDE;

		if( fileAction == dung::Action::DELETE && MatchName( config.oldPreserveRemoved_regex, fileInfo.name ) )
			fileAction = dung::Action::NONE;

		if( fileAction == dung::Action::APPLY_DIFF && MatchName( config.oldSkipChanged_regex, fileInfo.name ) )
			fileAction = dung::Action::NONE;

		output.stream << _T("file") << endl;
		output.stream << _T("{") << endl;

		output.stream << _T("\t") << _T("action=") << dung::ActionToString(fileAction) << endl;

		if( fileAction != dung::Action::DELETE && fileAction != dung::Action::NONE )
		{
			output.stream << _T("\t") << _T("new_path=") << quote << GenericString( relativePath / fileInfo.name ) << quote << endl;
			output.stream << _T("\t") << _T("new_size=") << fileInfo.newSize << endl;
			output.stream << _T("\t") << _T("new_sha1=") << quote << dung::SHA1_TO_TSTRING(fileInfo.newSha1) << quote << endl;
		}

		if( fileAction != dung::Action::NEW && fileAction != dung::Action::OVERRIDE && fileAction != dung::Action::NEW_BUT_NOT_INCLUDED )
		{
			output.stream << _T("\t") << _T("old_path=") << quote << GenericString( relativePath / fileInfo.name ) << quote << endl;
			output.stream << _T("\t") << _T("old_size=") << fileInfo.oldSize << endl;
			output.stream << _T("\t") << _T("old_sha1=") << quote << dung::SHA1_TO_TSTRING(fileInfo.oldSha1) << quote << endl;
		}

		if( fileAction == dung::Action::APPLY_DIFF )
		{
			output.stream << _T("\t") << _T("diff_path=") << quote << GenericString( relativePath / DiffFileName(fileInfo.name, config) ) << quote << endl;
			output.stream << _T("\t") << _T("diff_method=") << fileInfo.diffMethod << endl;
		}

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

bool rab::WriteRegistry( Options const& options, Config const& config, FolderInfo& rootFolder, PackageOutput_t& package, LogOutput_t& out )
{
	out << "Creating registry file..." << std:: endl;

	_tstringstream stringStream;
	OutputContext outputContext( stringStream, out );

	if( !options.newVersion.empty() )
		outputContext.stream << _T("new_version=") << quote << options.newVersion << quote << endl;
	
	if( !options.oldVersion.empty() )
		outputContext.stream << _T("old_version=") << quote << options.oldVersion << quote << endl;

	Path_t relativePath;
	WriteRegistryFolders( options, config, FolderInfo::FolderInfos_t( 1, &rootFolder ), relativePath, outputContext );

	_tstring fileContent = stringStream.str();
	std::string fileContentUtf8 = loc::conv::utf_to_utf<char>( fileContent );

	if( !package.WriteFile( dung::REGISTRY_FILENAME, fileContentUtf8.c_str(), fileContentUtf8.size() ) )
	{
		out << "Can't write file " << dung::REGISTRY_FILENAME << " to package" << std::endl;
		return false;
	}

	if( options.produceTemp )
	{
		std::ofstream file ( dung::REGISTRY_FILENAME, std::ios::out|std::ios::binary|std::ios::trunc );
		if( !file.is_open() )
		{
			out << "Can't create " << dung::REGISTRY_FILENAME << std::endl;
			return false;
		}

		file.write( fileContentUtf8.c_str(), fileContentUtf8.size() );

		file.close();
	}

	return true;
}
