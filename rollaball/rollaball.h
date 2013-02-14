#pragma once

#include <string>
#include <vector>

namespace rab
{
	typedef std::vector< std::string > ConfigValues_t;

	struct Options
	{
		std::string src, dst, tmp;
		std::string src_ver, dst_ver;
		std::string config_file;
	};

	struct Config
	{
		ConfigValues_t dst_folders;
		ConfigValues_t dst_files;
		ConfigValues_t dst_ignore_folders;
		ConfigValues_t dst_ignore_files;
		ConfigValues_t pack_files_using;
		ConfigValues_t dst_ignore_changed;
		ConfigValues_t dst_preserve_removed;
		ConfigValues_t src_ignore_files;
		int src_file_limit;
		std::string packed_extension;
	};
};

