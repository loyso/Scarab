#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include <hatchout/hatchout.h>

int ParseCommandLine( int argc, _TCHAR** argv, hatch::Options& options )
{
	namespace po = boost::program_options;

	po::options_description command_line_options("Command line options");
	command_line_options.add_options()
		("help,H", "produce help message")
		("package,P", po::value(&options.pathToPackage)->required(), "path to package file")
		("old,O", po::value(&options.pathToOld)->required(), "path to old content folder")
		("quiet,Q", po::bool_switch(&options.quiet)->default_value(false), "quiet mode")
		("reportFile,q", po::bool_switch(&options.reportFile)->default_value(false), "report each file")
		("verbose,V", po::bool_switch(&options.verbose)->default_value(false), "report everything")
		("stopIfError,S", po::bool_switch(&options.stopIfError)->default_value(false), "stop the process on first error")
		("checkOldSize,W", po::bool_switch(&options.checkOldSize)->default_value(false), "check old file size")
		("checkOldSha1,C", po::bool_switch(&options.checkOldSha1)->default_value(false), "check old file sha1 hash")
		;

	po::variables_map vm;
	po::store(po::wcommand_line_parser(argc, argv).options(command_line_options).run(), vm);

	if (vm.count("help")) 
		std::cout << command_line_options << "\n";

	po::notify(vm);    

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _MSC_VER
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	setlocale(LC_ALL, ""); // WinConsole to show wchar_t

	int result = 1;

	hatch::Options options;

	try
	{
		result = ParseCommandLine( argc, argv, options );
		if( result )
			return result;

		if( !hatch::ProcessData( options, std::cout ) )
			return 1;
	}
	catch(std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	return 0;
}
