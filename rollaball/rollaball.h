#pragma once

#include <dung/dung.h>

#include <string>
#include <vector>
#include <regex>

namespace zip
{
	class ZipArchiveOutput;
}

namespace rab
{
	class DiffEncoders;
}

namespace rab
{
	typedef _tstring String_t;
	typedef zip::ZipArchiveOutput PackageOutput_t;
	typedef std::wostream LogOutput_t;
	typedef std::wregex Regex_t;
	typedef _tstring DiffMethod_t;

	struct Options
	{
		String_t pathToNew, pathToOld, pathToTemp;
		String_t newVersion, oldVersion;
		String_t configFile;
		String_t packageFile;
		bool quiet;
		bool produceTemp;
	};

	struct Config
	{
		Config();

		typedef std::vector< String_t > StringValues_t;
		typedef std::vector< Regex_t > RegexValues_t;

		StringValues_t includeFolders;
		StringValues_t includeFiles;
		StringValues_t ignoreFolders;
		StringValues_t ignoreFiles;
		StringValues_t oldSkipChanged;
		StringValues_t oldPreserveRemoved;
		StringValues_t newIgnoreFolders;
		StringValues_t newIgnoreFiles;
		StringValues_t newOverrideFiles;

		void BuildRegexps();
		RegexValues_t includeFolders_regex;
		RegexValues_t includeFiles_regex;
		RegexValues_t ignoreFolders_regex;
		RegexValues_t ignoreFiles_regex;
		RegexValues_t oldSkipChanged_regex;
		RegexValues_t oldPreserveRemoved_regex;
		RegexValues_t newIgnoreFolders_regex;
		RegexValues_t newIgnoreFiles_regex;
		RegexValues_t newOverrideFiles_regex;

		size_t newFileLimit;
		String_t packedExtension;

		int zipCompressionLevel;
	};

	String_t DiffFileName( String_t const& fileName, Config const& config );

	void BuildRegexVector( Config::StringValues_t const& strings, Config::RegexValues_t& regexps );
	bool MatchName( Config::RegexValues_t const & filters, String_t const& name );

	class RollABall
	{
	public:
		RollABall();
		~RollABall();

		bool ProcessData( Options const &options, Config&config, DiffEncoders const& diffEncoders, LogOutput_t& out );
	};
};

