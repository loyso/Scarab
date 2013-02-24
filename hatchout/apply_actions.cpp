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
		return false;

	String_t dirPath;
	if( !ParentPath( FullPath( options, action.new_path ), dirPath ) )
		return false;

	if( !zip::ZipCreateDirectories( dirPath.c_str() ) )
		return false;

	if( !dung::WriteWholeFile( FullPath( options, action.new_path ), memoryBlock ) )
		return false;

	return true;
}

bool hatch::ApplyDiff( Options const& options, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	dung::MemoryBlock diffBlock;
	if( !zipInput.ReadFile( action.diff_path, diffBlock.pBlock, diffBlock.size ) )
		return false;

	dung::MemoryBlock oldFile;
	if( !ReadWholeFile( FullPath( options, action.old_path ), oldFile ) )
		return false;

	dung::MemoryBlock newFile( action.newSize );

	size_t size;
	int ret = xd3_decode_memory( diffBlock.pBlock, diffBlock.size, oldFile.pBlock, oldFile.size, newFile.pBlock, &size, newFile.size, 0 );
	if( ret != 0 )
		return false;

	SCARAB_ASSERT( size == newFile.size );

	if( !WriteWholeFile( FullPath( options, action.old_path ), newFile ) )
		return false;

	return true;
}

bool hatch::DeleteOldFile( Options const& options, RegistryAction const& action, LogOutput_t& out )
{
	String_t fullPath = FullPath( options, action.old_path );
	return !::remove( fullPath.c_str() );
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
