#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include <rollaball/rollaball.h>

int ParseCommandLine( int argc, _TCHAR** argv, rab::Options& options, rab::Config& config )
{
	namespace po = boost::program_options;

	po::options_description command_line_options("Command line options");
	command_line_options.add_options()
		("help,H", "produce help message")
		("src,S", po::wvalue(&options.pathToNew)->required(), "path to new content folder")
		("dst,D", po::wvalue(&options.pathToOld)->required(), "path to old content folder")
		("tmp,T", po::wvalue(&options.pathToTemp)->default_value(_T("./temp"),"./temp"), "path to temp folder")
		("config,C", po::wvalue(&options.configFile), "name of a configuration file.")
		("registry,R", po::wvalue(&options.registryFile)->default_value(_T("./registry.txt"), "./registry.txt"), "name of output registry file (patch description).")
		("src_ver", po::wvalue(&options.newVersion), "a name for new version")
		("dst_ver", po::wvalue(&options.oldVersion), "a name for old version")
		;

	po::options_description config_file_options("Config file options");
	config_file_options.add_options()
		("dst_folders", po::wvalue(&config.dst_folders), "destination folders to patch")
		("dst_files", po::wvalue(&config.dst_files), "destination files to patch")
		("dst_ignore_folders", po::wvalue(&config.dst_ignore_folders), "skip destination folders")
		("dst_ignore_files", po::wvalue(&config.dst_ignore_files), "skip destination files")
		("pack_files_using", po::wvalue(&config.pack_files_using), "choose pack method for source files: \"mask/method\"")
		("dst_ignore_changed", po::wvalue(&config.dst_ignore_changed), "destination files not to patch, if dst changed")
		("dst_preserve_removed", po::wvalue(&config.dst_preserve_removed), "destination files to preserve, if src removed")
		("src_ignore_files", po::wvalue(&config.src_ignore_files), "skip source files")
		("src_file_limit", po::wvalue(&config.src_file_limit)->default_value(0), "skip source files greater then the limit")		
		("packed_extension", po::wvalue(&config.packedExtension)->default_value(_T("diff"),"diff"), "extension for packed files")
		;

	po::variables_map vm;
	po::store(po::wcommand_line_parser(argc, argv).options(command_line_options).run(), vm);
	po::notify(vm);    

	if (vm.count("help")) 
	{
		std::cout << command_line_options << "\n";
		std::cout << config_file_options << "\n";
	}

	if( !options.configFile.empty() )
	{
		std::ifstream ifs(options.configFile.c_str());
		if (!ifs)
		{
			std::wcout << _T("can not open config file: ") << options.configFile << _T("\n");
			return 1;
		}
		else
		{
			po::store(po::parse_config_file(ifs, config_file_options), vm, true);
			po::notify(vm);
		}
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "");

	// Consider to use Boost.Locale
	// std::locale::global(boost::locale::generator().generate(""));
	// boost::filesystem::path::imbue(std::locale());

#ifdef _MSC_VER
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

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

	rab::ProcessData( options, config );

	return 0;
}
