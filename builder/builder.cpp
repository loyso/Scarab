#include <tchar.h>

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include <rollaball/rollaball.h>

int ParseCommandLine( int argc, wchar_t** argv, rab::Options& options, rab::Config& config )
{
	namespace po = boost::program_options;

	po::options_description command_line_options("Command line options");
	command_line_options.add_options()
		("help,H", "produce help message")
		("src,S", po::value(&options.src), "path to new content folder")
		("dst,D", po::value(&options.dst), "path to old content folder")
		("tmp,T", po::value(&options.tmp)->default_value("./temp"), "path to temp folder")
		("config,C", po::value(&options.config_file), "name of a configuration file.")
		("src_ver", po::value(&options.src_ver), "a name for new version")
		("dst_ver", po::value(&options.dst_ver), "a name for old version")
		;

	po::options_description config_file_options("Config file options");
	config_file_options.add_options()
		("dst_folders", po::value(&config.dst_folders), "destination folders to patch")
		("dst_files", po::value(&config.dst_files), "destination files to patch")
		("dst_ignore_folders", po::value(&config.dst_ignore_folders), "skip destination folders")
		("dst_ignore_files", po::value(&config.dst_ignore_files), "skip destination files")
		("pack_files_using", po::value(&config.pack_files_using), "choose pack method for source files: \"mask/method\"")
		("dst_ignore_changed", po::value(&config.dst_ignore_changed), "destination files not to patch, if dst changed")
		("dst_preserve_removed", po::value(&config.dst_preserve_removed), "destination files to preserve, if src removed")
		("src_ignore_files", po::value(&config.src_ignore_files), "skip source files")
		("src_file_limit", po::value(&config.src_file_limit)->default_value(0), "skip source files greater then the limit")		
		("packed_extension", po::value(&config.packed_extension)->default_value("diff"), "extension for packed files")
		;

	po::variables_map vm;
	po::store(po::wcommand_line_parser(argc, argv).options(command_line_options).run(), vm);
	po::notify(vm);    

	if (vm.count("help")) 
	{
		std::cout << command_line_options << "\n";
		std::cout << config_file_options << "\n";
	}

	if( !options.config_file.empty() )
	{
		std::ifstream ifs(options.config_file.c_str());
		if (!ifs)
		{
			std::cout << "can not open config file: " << options.config_file << "\n";
			return 1;
		}
		else
		{
			po::store(po::parse_config_file(ifs, config_file_options), vm);
			po::notify(vm);
		}
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int result = 1;

	rab::Options options;
	rab::Config config;

	try
	{
		result = ParseCommandLine( argc, argv, options, config );
	}
	catch(std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	if( result )
		return result;

	return 0;
}
