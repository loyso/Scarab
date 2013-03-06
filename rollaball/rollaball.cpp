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

bool rab::RollABall::ProcessData( Options const& options, Config& config, DiffEncoders const& diffEncoders, LogOutput_t& out )
{
	bool result = true;

	out << "Building optimized regular expressions..." << std::endl;
	config.BuildRegexps();

	FolderInfo rootFolder;
	BuildFileTree( options, config, rootFolder, out );

	zip::ZipArchiveOutput zipOut;
	if( !zipOut.Open( options.packageFile, true, zip::CompressionLevel::NO_COMPRESSION ) )
	{
		out << "Can't open zip archive " << options.packageFile << " zip error: " << zipOut.ErrorMessage() << std::endl;
		return false;
	}

	result &= BuildTempCopies( options, config, rootFolder, zipOut, out );
	result &= BuildDiffs( options, config, diffEncoders, rootFolder, zipOut, out );
	result &= GatherSha1( options, config, rootFolder, out );

	result &= WriteRegistry( options, config, rootFolder, zipOut, out );	

	if( !zipOut.Close() )
	{
		out << "Can't close zip archive " << options.packageFile << " zip error: " << zipOut.ErrorMessage() << std::endl;
		result = false;
	}

	if( result )
		out << "Successfully done!" << std::endl;
	else
		out << "FAILED." << std::endl;

	return result;
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
