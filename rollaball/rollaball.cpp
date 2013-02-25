#include "rollaball.h"

#include "filetree.h"

#include "filecopy.h"
#include "filediff.h"
#include "filesha1.h"

#include "registry_creator.h"

#include <zlib/minizip.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace rab
{
	void BuildRegexVector( Config::StringValues_t const& strings, Config::RegexValues_t& regexps );
}

void rab::ProcessData( Options const& options, Config& config )
{
	config.BuildRegexps();

	FolderInfo* pRootFolder = SCARAB_NEW FolderInfo();

	BuildFileTree( options, config, *pRootFolder );

	zip::ZipArchiveOutput zipOut( options.packageFile, true, zip::CompressionLevel::NO_COMPRESSION );

	BuildTempCopies( options, config, *pRootFolder, zipOut );
	BuildDiffs( options, config, *pRootFolder, zipOut );
	GatherSha1( options, config, *pRootFolder );

	WriteRegistry( options, config, *pRootFolder, zipOut );	

	zipOut.Close();

	delete pRootFolder;
	pRootFolder = NULL;
}

rab::String_t rab::DiffFileName( String_t const& fileName, Config const& config )
{
	return fileName + _T(".") + config.packedExtension;
}

bool rab::MatchName( Config::RegexValues_t const & filters, String_t const& name )
{
	for( Config::RegexValues_t::const_iterator i = filters.begin(); i != filters.end(); ++i )
	{
		Regex_t const& filter = *i;
		if( std::regex_match( name, filter ) )
			return true;
	}

	return false;
}

void rab::BuildRegexVector( Config::StringValues_t const& strings, Config::RegexValues_t& regexps )
{
	for( Config::StringValues_t::const_iterator i = strings.begin(); i != strings.end(); ++i )
	{
		String_t const& value = *i;

		Regex_t regexp( value, std::regex::icase | std::regex::optimize | std::regex::ECMAScript );
		regexps.push_back( regexp );
	}
}

void rab::Config::BuildRegexps()
{
	BuildRegexVector( dst_folders, dst_folders_regex );
	BuildRegexVector( dst_files, dst_files_regex );
	BuildRegexVector( dst_ignore_folders, dst_ignore_folders_regex );
	BuildRegexVector( dst_ignore_files, dst_ignore_files_regex );
	BuildRegexVector( dst_ignore_changed, dst_ignore_changed_regex );
	BuildRegexVector( dst_preserve_removed, dst_preserve_removed_regex );
	BuildRegexVector( newIgnoreFolders, newIgnoreFolders_regex );
	BuildRegexVector( newIgnoreFiles, newIgnoreFiles_regex );
}
