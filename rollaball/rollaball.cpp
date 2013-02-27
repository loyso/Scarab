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
}

rab::RollABall::RollABall()
{
}

rab::RollABall::~RollABall()
{
}

void rab::RollABall::ProcessData( Options const& options, Config& config, DiffEncoders const& diffEncoders )
{
	config.BuildRegexps();

	FolderInfo* pRootFolder = SCARAB_NEW FolderInfo();

	BuildFileTree( options, config, *pRootFolder );

	zip::ZipArchiveOutput zipOut( options.packageFile, true, zip::CompressionLevel::NO_COMPRESSION );

	BuildTempCopies( options, config, *pRootFolder, zipOut );
	BuildDiffs( options, config, diffEncoders, *pRootFolder, zipOut );
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
	BuildRegexVector( includeFolders, includeFolders_regex );
	BuildRegexVector( includeFiles, includeFiles_regex );
	BuildRegexVector( ignoreFolders, ignoreFolders_regex );
	BuildRegexVector( ignoreFiles, ignoreFiles_regex );
	BuildRegexVector( oldSkipChanged, oldSkipChanged_regex );
	BuildRegexVector( oldPreserveRemoved, oldPreserveRemoved_regex );
	BuildRegexVector( newIgnoreFolders, newIgnoreFolders_regex );
	BuildRegexVector( newIgnoreFiles, newIgnoreFiles_regex );
	BuildRegexVector( newOverrideFiles, newOverrideFiles_regex );
}
