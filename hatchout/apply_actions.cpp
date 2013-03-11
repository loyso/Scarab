#include "apply_actions.h"

#include <dung/memoryblock.h>
#include <dung/diffdecoder.h>

#include <zlib/miniunzip.h>

#ifdef WIN32
#	include <direct.h>
#endif

namespace hatch
{
	bool CreateNewFile( Options const& options, RegistryAction const& action, bool overrideFile, zip::ZipArchiveInput& zipInput, LogOutput_t& out );
	bool DeleteOldFile( Options const& options, RegistryAction const& action, LogOutput_t& out );
	bool ApplyDiff( Options const& options, DiffDecoders const& diffDecoders, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out );
	
	bool CheckOldFileData( Options const& options, dung::MemoryBlock const& oldFile, RegistryAction const& action, LogOutput_t& out );
	bool ApplyAction( Options const& options, DiffDecoders const& diffDecoders, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out );

	String_t FullPath( Options const& options, String_t const& relativePath )
	{
		return options.pathToOld + _T("/") + relativePath;
	}

	String_t TempPath( Options const& options, String_t const& relativePath )
	{
		return options.pathToOld + _T("/") + relativePath + _T(".tmp");
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

	bool DeleteFile( const char* path )
	{
		return !::remove( path );
	}

	bool DeleteFile( const wchar_t* path )
	{
		return !::_wremove( path );
	}

	bool FileSize( const char* path, size_t& size )
	{
		FILE* fp = fopen( path, "rb");
		if( fp == NULL )
			return false;

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		fclose( fp );
		return true;
	}

	bool FileSize( const wchar_t* path, size_t& size )
	{
		FILE* fp = _wfopen( path, L"rb");
		if( fp == NULL )
			return false;

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		fclose( fp );
		return true;
	}

	int CreateDirectory( const char* dirname )
	{
		int ret=0;
#ifdef WIN32
		ret = _mkdir(dirname);
#else
		ret = mkdir (dirname,0775);
#endif
		return ret;
	}

	int CreateDirectory( const wchar_t* dirname )
	{
		int ret=0;
#ifdef WIN32
		ret = _wmkdir(dirname);
#else
		ret = mkdir (dirname,0775);
#endif
		return ret;
	}

	bool CreateDirectories( const _TCHAR *newdir )
	{
		size_t len = dung::StrLen( newdir );
		if (len <= 0)
			return false;

		_tstring buffer( newdir );

		if( buffer[len-1] == _T('/') || buffer[len-1] == _T('\\') )
			buffer.resize( len-1 );

		if( CreateDirectory( buffer.c_str() ) == 0 )
			return true;

		for( size_t p = 1; p<len; ++p )
		{
			while( p<len && buffer[p] != _T('\\') && buffer[p] != _T('/') )
				p++;

			_tstring subDir = buffer.substr( 0, p );
			if( CreateDirectory( subDir.c_str() ) == -1 && errno == ENOENT )
				return false;
		}

		return true;
	}
}

bool hatch::CreateNewFile( Options const& options, RegistryAction const& action, bool overrideFile, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	String_t fullPathNew = FullPath( options, action.new_path );

	if( !overrideFile )
	{
		size_t existFileSize;
		if( FileSize( fullPathNew.c_str(), existFileSize ) )
		{
			out << "Can't create new file " << action.new_path << ". File already exists." << std::endl;
			return false;
		}
	}

	dung::MemoryBlock memoryBlock;
	if( !zipInput.ReadFile( action.new_path, memoryBlock.pBlock, memoryBlock.size ) )
	{
		out << "Can't unzip file " << action.new_path << ". zip error: " << zipInput.ErrorMessage() << std::endl;
		return false;
	}

	String_t dirPath;
	if( !ParentPath( fullPathNew, dirPath ) )
	{
		out << "Can't extract parent path from " << fullPathNew << std::endl;
		return false;
	}

	if( !CreateDirectories( dirPath.c_str() ) )
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

bool hatch::CheckOldFileData( Options const& options, dung::MemoryBlock const& oldFile, RegistryAction const& action, LogOutput_t& out )
{
	if( options.checkOldSize && oldFile.size != action.oldSize )
	{
		out << _T("Old file has wrong size. ") << action.old_path << _T(" Real size=") << oldFile.size << _T(", registry size=") << action.oldSize << std::endl;
		return false;
	}

	if( options.checkOldSha1 )
	{
		dung::Sha1 oldSha1;
		dung::SHA1Compute( oldFile.pBlock, oldFile.size, oldSha1 );
		if( oldSha1 != action.oldSha1 )
		{
			out << _T("Old file has wrong SHA1. ") << action.old_path << _T(" Real SHA1=") << SHA1_TO_TSTRING(oldSha1) << _T(", registry SHA1=") << SHA1_TO_TSTRING(action.oldSha1) << std::endl;
			return false;
		}
	}

	return true;
}

bool hatch::ApplyDiff( Options const& options, DiffDecoders const& diffDecoders, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	dung::MemoryBlock diffBlock;
	if( !zipInput.ReadFile( action.diff_path, diffBlock.pBlock, diffBlock.size ) )
	{
		out << "Can't unzip file " << action.diff_path << ". zip error: " << zipInput.ErrorMessage() << std::endl;
		return false;
	}

	dung::MemoryBlock oldFile;
	String_t fullPathOld = FullPath( options, action.old_path );
	if( !ReadWholeFile( fullPathOld, oldFile ) )
	{
		out << "Can't read file " << fullPathOld << std::endl;
		return false;
	}

	if( !CheckOldFileData( options, oldFile, action, out ) )
		return false;

	dung::DiffDecoder_i* pDecoder = diffDecoders.FindDecoder( action.diff_method );
	if( pDecoder != NULL )
	{
		dung::MemoryBlock newFile;
		newFile.size = action.newSize;

		bool result = pDecoder->DecodeDiffMemoryBlock( oldFile.pBlock, oldFile.size, diffBlock.pBlock, diffBlock.size, newFile.pBlock, newFile.size );
		if( newFile.size != action.newSize )
			result = false;
		if( !result )
		{
			_tstring errorMessage;
			pDecoder->GetErrorMessage( errorMessage );
			out << "Can't decode with " << action.diff_method << " for file " << action.diff_path << " " << errorMessage << std::endl;
		}
		else
		{
			if( !WriteWholeFile( fullPathOld, newFile ) )
			{
				out << "Can't write file " << fullPathOld << std::endl;
				result = false;
			}
		}

		return result;
	}
	else
	{
		dung::DiffDecoderExternal_i* pExternalDecoder = diffDecoders.FindExternalDecoder( action.diff_method );
		if( pExternalDecoder != NULL )
		{
			String_t oldTempPath = TempPath( options, action.old_path );
			if( !WriteWholeFile( oldTempPath, oldFile ) )
			{
				out << "Can't write file " << oldTempPath << std::endl;
				return false;
			}

			String_t diffTempPath = TempPath( options, action.diff_path );
			if( !WriteWholeFile( diffTempPath, diffBlock ) )
			{
				out << "Can't write file " << diffTempPath << std::endl;
				hatch::DeleteFile( oldTempPath.c_str() );
				return false;
			}

			bool result = pExternalDecoder->DecodeDiffFile( fullPathOld.c_str(), oldTempPath.c_str(), diffTempPath.c_str() );
			if( !result )
			{
				_tstring errorMessage;
				pExternalDecoder->GetErrorMessage( errorMessage );
				out << "Can't decode with " << action.diff_method << " for file " << action.diff_path << " " << errorMessage << std::endl;
			}

			hatch::DeleteFile( oldTempPath.c_str() );
			hatch::DeleteFile( diffTempPath.c_str() );

			return result;
		}
		else
		{
			out << "Can't find decoder " << action.diff_method << " for file " << action.diff_path << std::endl;
			return false;
		}
	}

	return true;
}

bool hatch::DeleteOldFile( Options const& options, RegistryAction const& action, LogOutput_t& out )
{
	String_t fullPath = FullPath( options, action.old_path );
	bool result = DeleteFile( fullPath.c_str() );
	if( !result )
		out << "Can't delete file " << fullPath << std::endl;

	return result;
}

bool hatch::ApplyAction( Options const& options, DiffDecoders const& diffDecoders, RegistryAction const& action, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	switch( action.action )
	{
	case dung::Action::NEW:
		if( options.reportFile )
			out << "Creating new file " << action.new_path << std::endl;
		if( !CreateNewFile( options, action, true, zipInput, out ) ) // Override it anyway.
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
		if( !ApplyDiff( options, diffDecoders, action, zipInput, out ) )
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

bool hatch::ApplyActions( Options const& options, DiffDecoders const& diffDecoders, Registry const& registry, zip::ZipArchiveInput& zipInput, LogOutput_t& out )
{
	int numErrors = 0;

	for( Registry::Actions_t::const_iterator i = registry.actions.begin(); i != registry.actions.end(); ++i )
	{
		RegistryAction const& action = **i;

		if( !ApplyAction( options, diffDecoders, action, zipInput, out ) )
		{
			if( options.stopIfError )
				return false;
			else
				++numErrors;
		}
	}

	if( numErrors == 0 )
		return true;

	out << "FAILED. " << numErrors << " errors occured." << std::endl;

	return false;
}


hatch::DiffDecoders::DiffDecoders()
{
}

hatch::DiffDecoders::~DiffDecoders()
{	
}

void hatch::DiffDecoders::AddDecoder( dung::DiffDecoder_i& diffDecoder, String_t const& decoderName )
{
	m_nameToDecoder.insert( std::make_pair( String_t(decoderName), &diffDecoder ) );
}

void hatch::DiffDecoders::AddExternalDecoder( dung::DiffDecoderExternal_i& diffDecoder, String_t const& decoderName )
{
	m_nameToDecoderExternal.insert( std::make_pair( String_t(decoderName), &diffDecoder ) );
}

dung::DiffDecoder_i* hatch::DiffDecoders::FindDecoder( String_t const& decoderName ) const
{
	NameToDecoder_t::const_iterator i = m_nameToDecoder.find( decoderName );
	if( i == m_nameToDecoder.end() )
		return NULL;

	return i->second;
}

dung::DiffDecoderExternal_i* hatch::DiffDecoders::FindExternalDecoder( String_t const& decoderName ) const
{
	NameToDecoderExternal_t::const_iterator i = m_nameToDecoderExternal.find( decoderName );
	if( i == m_nameToDecoderExternal.end() )
		return NULL;

	return i->second;
}

bool hatch::DiffDecoders::Empty() const
{
	return m_nameToDecoder.empty() && m_nameToDecoderExternal.empty();
}
