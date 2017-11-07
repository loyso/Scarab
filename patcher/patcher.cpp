#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include <hatchout/hatchout.h>
#include <hatchout/apply_actions.h>

#include <diffmethods/diff_deltamax.h>
#include <diffmethods/diff_xdelta.h>

struct DecodersConfig
{
	_tstring deltaMax_userName;
	_tstring deltaMax_licenseKey;
};

#ifdef SCARAB_WCHAR_MODE
#	define COMMAND_LINE_PARSER wcommand_line_parser
#	define PO_VALUE po::wvalue
#else
#	define COMMAND_LINE_PARSER command_line_parser
#	define PO_VALUE po::value
#endif

int ParseCommandLine( int argc, _TCHAR** argv, hatch::Options& options, DecodersConfig& decodersConfig )
{
	namespace po = boost::program_options;

	po::options_description command_line_options("Command line options");
	command_line_options.add_options()
		("help,H", "produce help message")
		("package,P", PO_VALUE(&options.pathToPackage)->required(), "path to package file")
		("old,O", PO_VALUE(&options.pathToOld)->required(), "path to old content folder")
		("quiet,Q", po::bool_switch(&options.quiet)->default_value(false), "quiet mode")
		("reportFile,q", po::bool_switch(&options.reportFile)->default_value(false), "report each file")
		("verbose,V", po::bool_switch(&options.verbose)->default_value(false), "report everything")
		("stopIfError,S", po::bool_switch(&options.stopIfError)->default_value(false), "stop the process on first error")
		("checkOldSize,W", po::bool_switch(&options.checkOldSize)->default_value(false), "check old file size")
		("checkOldSha1,C", po::bool_switch(&options.checkOldSha1)->default_value(false), "check old file sha1 hash")
		;

#if SCARAB_DELTAMAX
	// Expose DeltaMAX options.	
	command_line_options.add_options()
		("deltamax.user_name", PO_VALUE(&decodersConfig.deltaMax_userName), "registered user name")
		("deltamax.license_key", PO_VALUE(&decodersConfig.deltaMax_licenseKey), "license key string")
		;
#endif // DELTAMAX

	po::variables_map vm;
	po::store(po::COMMAND_LINE_PARSER(argc, argv).options(command_line_options).run(), vm);

	if (argc == 1 || vm.count("help")) 
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
	hatch::DiffDecoders diffDecoders;

	try
	{
		DecodersConfig decodersConfig;
		result = ParseCommandLine( argc, argv, options, decodersConfig );
		if( result )
			return result;

		hatch::HatchOut hatchOut;

#if SCARAB_XDELTA
		// Plug in Xdelta decoder.
		xdelta::XdeltaDecoderExternal xdeltaDecoder;
		diffDecoders.AddExternalDecoder(xdeltaDecoder, _T("xdelta"));
#else
		// Plug in Xdelta decoder.
		xdelta::XdeltaDecoder xdeltaDecoder;
		diffDecoders.AddDecoder( xdeltaDecoder, _T("xdelta") );
#endif // SCARAB_XDELTA

#if SCARAB_DELTAMAX
		// Plug in DeltaMAX decoder.
		deltamax::DeltaMaxDecoder deltaMaxDecoder;
		deltaMaxDecoder.SetUserLicense( decodersConfig.deltaMax_userName.c_str(), decodersConfig.deltaMax_licenseKey.c_str() );
		diffDecoders.AddExternalDecoder( deltaMaxDecoder, _T("deltamax") );
#endif // SCARAB_DELTAMAX

		_tostream nil_out( SCARAB_NEW dung::nil_buf );

		if( hatchOut.ProcessData( options, diffDecoders, options.quiet ? nil_out : _tcout ) )
			result = 0;

		delete nil_out.rdbuf( NULL );
	}
	catch(std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	return result;
}
