#include "apply_actions.h"

#include <dung/memoryblock.h>

#include <zlib/miniunzip.h>
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
	bool CreateNewFile( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out );
	bool DeleteOldFile( Options const& options, RegistryAction const& action, LogOutput_t& out );
	bool ApplyDiff( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out );

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

bool hatch::CreateNewFile( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	dung::MemoryBlock memoryBlock;
	if( !zipInput.ReadFile( action.new_path, memoryBlock.pBlock, memoryBlock.size ) )
	{
		out << "Can't unzip file " << action.new_path << std::endl;
		return false;
	}

	String_t dirPath;
	String_t fullPathNew = FullPath( options, action.new_path );
	if( !ParentPath( fullPathNew, dirPath ) )
		return false;

	if( !zip::ZipCreateDirectories( dirPath.c_str() ) )
	{
		out << "Can't create directory " << dirPath << std::endl;
		return false;
	}

	if( !dung::WriteWholeFile( fullPathNew, memoryBlock ) )
	{
		out << "Can't write file " << fullPathNew << std::endl;
		return false;
	}

	return true;
}

bool hatch::ApplyDiff( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	dung::MemoryBlock diffBlock;
	if( !zipInput.ReadFile( action.diff_path, diffBlock.pBlock, diffBlock.size ) )
	{
		out << "Can't unzip file " << action.diff_path << std::endl;
		return false;
	}

	dung::MemoryBlock oldFile;
	String_t fullPathOld = FullPath( options, action.old_path );
	if( !ReadWholeFile( fullPathOld, oldFile ) )
	{
		out << "Can't read file " << fullPathOld << std::endl;
		return false;
	}

	dung::MemoryBlock newFile( action.newSize );

	size_t size;
	int ret = xd3_decode_memory( diffBlock.pBlock, diffBlock.size, oldFile.pBlock, oldFile.size, newFile.pBlock, &size, newFile.size, 0 );
	if( ret != 0 )
	{
		out << "Can't decode file " << action.diff_path << " error code=" << ret << std::endl;
		return false;
	}

	SCARAB_ASSERT( size == action.newSize );

	if( !WriteWholeFile( fullPathOld, newFile ) )
	{
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
		out << "Can't delete file " << fullPath << std::endl;

	return result;
}


bool hatch::ApplyActions( Options const& options, Registry const& registry, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	for( Registry::Actions_t::const_iterator i = registry.actions.begin(); i != registry.actions.end(); ++i )
	{
		RegistryAction const& action = **i;
		switch( action.action )
		{
		case dung::Action::NEW:
			if( !CreateNewFile( options, action, zipInput, out ) )
				return false;
			break;
		case dung::Action::DELETE:
			if( !DeleteOldFile( options, action, out ) )
				return false;
			break;
		case dung::Action::MOVE:
			break;
		case dung::Action::APPLY_DIFF:
			if( !ApplyDiff( options, action, zipInput, out ) )
				return false;
			break;
		case dung::Action::NONE:
			break;
		case dung::Action::NEW_BUT_NOT_INCLUDED:
			break;
		default:
			SCARAB_ASSERT( false && "unknown action type" );
		}
	}

	return true;
}
