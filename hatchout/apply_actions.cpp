#include "apply_actions.h"

#include <dung/memoryblock.h>

#include <zlib/miniunzip.h>

extern "C"
{
#include <external/xdelta/xdelta3.h>
#undef NEW
#undef DELETE
#undef MOVE
}


namespace hatch
{
	bool CreateNewFile( Options const& options, RegistryAction const& action, bool overrideFile, zip::ZipArchiveInput& zipInput, LogOutput_t& out );
	bool DeleteOldFile( Options const& options, RegistryAction const& action, LogOutput_t& out );
	bool ApplyDiff( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out );
	
	bool CheckOldFileData( Options const& options, dung::MemoryBlock const& oldFile, RegistryAction const& action, LogOutput_t& out );
	bool ApplyAction( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out );

	String_t FullPath( Options const& options, String_t const& relativePath )
	{
		return options.pathToOld + "/" + relativePath;
	}

	bool ParentPath( String_t const& path, String_t& parentPath )
	{
		size_t lastSlash = 0;
		for( size_t i = 0; i < path.size(); ++i )
		{
			if( path[i] == '\\' || path[i] == '/' )
				lastSlash = i;
		}

		if( lastSlash == 0 )
			return false;

		parentPath = path.substr( 0, lastSlash );
		return true;
	}
}

bool hatch::CreateNewFile( Options const& options, RegistryAction const& action, bool overrideFile, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	String_t fullPathNew = FullPath( options, action.new_path );

	if( !overrideFile )
	{
		size_t existFileSize;
		if( zip::FileSize( fullPathNew.c_str(), existFileSize ) )
		{
			if( !options.quiet )
				out << "Can't create new file " << action.new_path << ". File already exists." << std::endl;
			return false;
		}
	}

	dung::MemoryBlock memoryBlock;
	if( !zipInput.ReadFile( action.new_path, memoryBlock.pBlock, memoryBlock.size ) )
	{
		if( !options.quiet )
			out << "Can't unzip file " << action.new_path << ". zip error: " << zipInput.ErrorMessage() << std::endl;
		return false;
	}

	String_t dirPath;
	if( !ParentPath( fullPathNew, dirPath ) )
	{
		if( !options.quiet )
			out << "Can't extract parent path from " << fullPathNew << std::endl;
		return false;
	}

	if( !zip::ZipCreateDirectories( dirPath.c_str() ) )
	{
		if( !options.quiet )
			out << "Can't create directory " << dirPath << std::endl;
		return false;
	}

	if( !dung::WriteWholeFile( fullPathNew, memoryBlock ) )
	{
		if( !options.quiet )
			out << "Can't write file " << fullPathNew << std::endl;
		return false;
	}

	return true;
}

bool hatch::CheckOldFileData( Options const& options, dung::MemoryBlock const& oldFile, RegistryAction const& action, LogOutput_t& out )
{
	if( options.checkOldSize && oldFile.size != action.oldSize )
	{
		if( !options.quiet )
			out << "Old file has wrong size. " << action.old_path << " Real size=" << oldFile.size << ", registry size=" << action.oldSize << std::endl;
		return false;
	}

	if( options.checkOldSha1 )
	{
		dung::Sha1 oldSha1;
		dung::SHA1Compute( oldFile.pBlock, oldFile.size, oldSha1 );
		if( oldSha1 != action.oldSha1 )
		{
			if( !options.quiet )
				out << "Old file has wrong SHA1. " << action.old_path << " Real SHA1=" << SHA1ToString(oldSha1) << ", registry SHA1=" << SHA1ToString(action.oldSha1) << std::endl;
			return false;
		}
	}

	return true;
}

bool hatch::ApplyDiff( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	dung::MemoryBlock diffBlock;
	if( !zipInput.ReadFile( action.diff_path, diffBlock.pBlock, diffBlock.size ) )
	{
		if( !options.quiet )
			out << "Can't unzip file " << action.diff_path << ". zip error: " << zipInput.ErrorMessage() << std::endl;
		return false;
	}

	dung::MemoryBlock oldFile;
	String_t fullPathOld = FullPath( options, action.old_path );
	if( !ReadWholeFile( fullPathOld, oldFile ) )
	{
		if( !options.quiet )
			out << "Can't read file " << fullPathOld << std::endl;
		return false;
	}

	if( !CheckOldFileData( options, oldFile, action, out ) )
		return false;

	dung::MemoryBlock newFile( action.newSize );

	size_t size;
	int ret = xd3_decode_memory( diffBlock.pBlock, diffBlock.size, oldFile.pBlock, oldFile.size, newFile.pBlock, &size, newFile.size, 0 );
	if( ret != 0 )
	{
		if( !options.quiet )
			out << "Can't decode file " << action.diff_path << " error code=" << ret << std::endl;
		return false;
	}

	SCARAB_ASSERT( size == action.newSize );

	if( !WriteWholeFile( fullPathOld, newFile ) )
	{
		if( !options.quiet )
			out << "Can't write file " << fullPathOld << std::endl;
		return false;
	}

	return true;
}

bool hatch::DeleteOldFile( Options const& options, RegistryAction const& action, LogOutput_t& out )
{
	String_t fullPath = FullPath( options, action.old_path );
	bool result = !::remove( fullPath.c_str() );
	if( !result )
	{
		if( !options.quiet )
			out << "Can't delete file " << fullPath << std::endl;
	}

	return result;
}

bool hatch::ApplyAction( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	switch( action.action )
	{
	case dung::Action::NEW:
		if( options.reportFile )
			out << "Creating new file " << action.new_path << std::endl;
		if( !CreateNewFile( options, action, false, zipInput, out ) )
			return false;
		break;

	case dung::Action::OVERRIDE:
		if( options.reportFile )
			out << "Overriding with new file " << action.new_path << std::endl;
		if( !CreateNewFile( options, action, true, zipInput, out ) )
			return false;
		break;

	case dung::Action::DELETE:
		if( options.reportFile )
			out << "Deleting old file " << action.old_path << std::endl;
		if( !DeleteOldFile( options, action, out ) )
			return false;
		break;

	case dung::Action::MOVE:
		// TODO: implement
		break;

	case dung::Action::APPLY_DIFF:
		if( options.reportFile )
			out << "Applying difference " << action.diff_path << " to " << action.old_path << std::endl;
		if( !ApplyDiff( options, action, zipInput, out ) )
			return false;
		break;

	case dung::Action::NONE:
		if( options.reportFile )
			out << "Skipping old file " << action.old_path << std::endl;
		break;

	case dung::Action::NEW_BUT_NOT_INCLUDED:
		if( options.reportFile )
			out << "Skipping not included file " << action.new_path << std::endl;
		break;

	default:
		SCARAB_ASSERT( false && "unknown action type" );
	}

	return true;
}

bool hatch::ApplyActions( Options const& options, Registry const& registry, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	int numErrors = 0;

	for( Registry::Actions_t::const_iterator i = registry.actions.begin(); i != registry.actions.end(); ++i )
	{
		RegistryAction const& action = **i;

		if( !ApplyAction( options, action, zipInput, out ) )
		{
			if( options.stopIfError )
				return false;
			else
				++numErrors;
		}
	}

	if( numErrors == 0 )
		return true;

	if( !options.quiet )
		out << "FAILED. " << numErrors << " errors occured." << std::endl;

	return false;
}
