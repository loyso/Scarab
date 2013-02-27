#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

#include <rollaball/rollaball.h>

int ParseCommandLine( int argc, _TCHAR** argv, rab::Options& options, rab::Config& config )
{
	namespace po = boost::program_options;

	po::options_description command_line_options("Command line options");
	command_line_options.add_options()
		("help,H", "produce help message")
		("new,N", po::wvalue(&options.pathToNew)->required(), "path to new content folder")
		("old,O", po::wvalue(&options.pathToOld)->required(), "path to old content folder")
		("tmp,T", po::wvalue(&options.pathToTemp)->default_value(_T("temp"),"temp"), "path to temp folder")
		("config,C", po::wvalue(&options.configFile), "name of a configuration file.")
		("package,P", po::wvalue(&options.packageFile)->default_value(_T("package.zip"), "package.zip"), "name of output package file.")
		("new_ver", po::wvalue(&options.newVersion), "a name for new version")
		("old_ver", po::wvalue(&options.oldVersion), "a name for old version")
		;

	po::options_description config_file_options("Config file options");
	config_file_options.add_options()
		("include_folders", po::wvalue(&config.includeFolders), "folders to process")
		("include_files", po::wvalue(&config.includeFiles), "files to process")
		("ignore_folders", po::wvalue(&config.ignoreFolders), "skip destination folders")
		("ignore_files", po::wvalue(&config.ignoreFiles), "skip destination files")
		("old_skip_changed", po::wvalue(&config.oldSkipChanged), "destination files not to patch, if dst changed")
		("old_preserve_removed", po::wvalue(&config.oldPreserveRemoved), "destination files to preserve, if src removed")
		("new_ignore_folders", po::wvalue(&config.newIgnoreFolders), "skip source folders")
		("new_ignore_files", po::wvalue(&config.newIgnoreFiles), "skip source files")
		("new_override_files", po::wvalue(&config.newOverrideFiles), "force override new files")
		("new_file_limit", po::wvalue(&config.newFileLimit)->default_value(0), "skip source files greater then the limit")		
		("packed_extension", po::wvalue(&config.packedExtension)->default_value(_T("diff"),"diff"), "extension for packed files")
		("pack_files_using", po::wvalue(&config.pack_files_using), "choose pack method for source files: \"mask/method\"")
		;

	po::variables_map vm;
	po::store(po::wcommand_line_parser(argc, argv).options(command_line_options).run(), vm);

	if (vm.count("help")) 
	{
		std::cout << command_line_options << "\n";
		std::cout << config_file_options << "\n";
	}
	po::notify(vm);    

	std::ifstream ifs;
	if( !options.configFile.empty() )
	{
		ifs.open(options.configFile.c_str());
		if (!ifs)
		{
			std::wcout << _T("can not open config file: ") << options.configFile << _T("\n");
			return 1;
		}
	}

	po::store(po::parse_config_file(ifs, config_file_options), vm, true);
	po::notify(vm);

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _MSC_VER
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	setlocale(LC_ALL, ""); // WinConsole to show wchar_t

	std::locale loc2 = std::locale::global(boost::locale::generator().generate(""));
	// boost::filesystem::path::imbue(std::locale());

	int result = 1;

	rab::Options options;
	rab::Config config;

	try
	{
		result = ParseCommandLine( argc, argv, options, config );
		if( result )
			return result;

		rab::ProcessData( options, config );
	}
	catch(std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	std::locale::global(std::locale::classic());
	
	return 0;
}
