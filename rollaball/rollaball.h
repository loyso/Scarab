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
	typedef _tstring String_t;
	typedef zip::ZipArchiveOutput PackageOutput_t;
	typedef std::wregex Regex_t;

	struct Options
	{
		String_t pathToNew, pathToOld, pathToTemp;
		String_t newVersion, oldVersion;
		String_t configFile;
		String_t packageFile;
	};

	struct Config
	{
		typedef std::vector< String_t > StringValues_t;
		typedef std::vector< Regex_t > RegexValues_t;

		StringValues_t dst_folders;
		StringValues_t dst_files;
		StringValues_t dst_ignore_folders;
		StringValues_t dst_ignore_files;
		StringValues_t dst_ignore_changed;
		StringValues_t dst_preserve_removed;
		StringValues_t newIgnoreFolders;
		StringValues_t newIgnoreFiles;

		void BuildRegexps();
		RegexValues_t dst_folders_regex;
		RegexValues_t dst_files_regex;
		RegexValues_t dst_ignore_folders_regex;
		RegexValues_t dst_ignore_files_regex;
		RegexValues_t dst_ignore_changed_regex;
		RegexValues_t dst_preserve_removed_regex;
		RegexValues_t newIgnoreFolders_regex;
		RegexValues_t newIgnoreFiles_regex;

		StringValues_t pack_files_using;
		size_t newFileLimit;
		String_t packedExtension;
	};

	String_t DiffFileName( String_t const& fileName, Config const& config );
	bool MatchName( Config::RegexValues_t const & filters, String_t const& name );

	void ProcessData( Options const &options, Config&config );
};

