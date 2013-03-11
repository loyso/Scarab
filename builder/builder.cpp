#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
namespace po = boost::program_options;

#include <rollaball/rollaball.h>
#include <rollaball/filediff.h>

#include <diffmethods/diff_deltamax.h>
#include <diffmethods/diff_xdelta.h>

struct EncodersConfig
{
#if SCARAB_DELTAMAX
	rab::Config::StringValues_t deltaMax_packFiles;
	_tstring deltaMax_userName;
	_tstring deltaMax_licenseKey;
#endif // SCARAB_DELTAMAX

#if SCARAB_XDELTA
	rab::Config::StringValues_t xdelta_packFiles;
	xdelta::Config xdelta_config;
#endif // SCARAB_XDELTA
};

#ifdef SCARAB_WCHAR_MODE
#	define COMMAND_LINE_PARSER wcommand_line_parser
#	define PO_VALUE po::wvalue
#else
#	define COMMAND_LINE_PARSER command_line_parser
#	define PO_VALUE po::value
#endif

int ParseCommandLine( int argc, _TCHAR** argv, rab::Options& options, rab::Config& config, EncodersConfig& encodersConfig )
{
	po::options_description command_line_options("Command line options");
	command_line_options.add_options()
		("help,H", "produce help message")
		("new,N", PO_VALUE(&options.pathToNew)->required(), "path to new content folder")
		("old,O", PO_VALUE(&options.pathToOld)->required(), "path to old content folder")
		("tmp,T", PO_VALUE(&options.pathToTemp)->default_value(_T("temp"),"temp"), "path to temp folder")
		("config,C", PO_VALUE(&options.configFile), "name of a configuration file.")
		("package,P", PO_VALUE(&options.packageFile)->default_value(_T("package.zip"), "package.zip"), "name of output package file.")
		("new_ver", PO_VALUE(&options.newVersion), "a name for new version")
		("old_ver", PO_VALUE(&options.oldVersion), "a name for old version")
		("quiet,Q", po::bool_switch(&options.quiet)->default_value(false), "quiet mode")
		("produce_temp", po::bool_switch(&options.produceTemp)->default_value(false), "create temp files in addition to archive")
		;

	po::options_description config_file_options("Config file options");
	config_file_options.add_options()
		("include_folders", PO_VALUE(&config.includeFolders), "folders to process")
		("include_files", PO_VALUE(&config.includeFiles), "files to process")
		("ignore_folders", PO_VALUE(&config.ignoreFolders), "skip destination folders")
		("ignore_files", PO_VALUE(&config.ignoreFiles), "skip destination files")
		("old_skip_changed", PO_VALUE(&config.oldSkipChanged), "destination files not to patch, if dst changed")
		("old_preserve_removed", PO_VALUE(&config.oldPreserveRemoved), "destination files to preserve, if src removed")
		("new_ignore_folders", PO_VALUE(&config.newIgnoreFolders), "skip source folders")
		("new_ignore_files", PO_VALUE(&config.newIgnoreFiles), "skip source files")
		("new_override_files", PO_VALUE(&config.newOverrideFiles), "force override new files")
		("new_file_limit", PO_VALUE(&config.newFileLimit)->default_value(0), "skip source files greater then the limit")		
		("packed_extension", PO_VALUE(&config.packedExtension)->default_value(_T("diff"),"diff"), "extension for packed files")
		("zip.compression", PO_VALUE(&config.zipCompressionLevel)->default_value(9), "1 through 9 (0 corresponds to STORE)")		
		;

#if SCARAB_DELTAMAX
	// Expose DeltaMAX options.	
	config_file_options.add_options()
		("deltamax.pack_files", PO_VALUE(&encodersConfig.deltaMax_packFiles), "files to pack with DeltaMAX")
		("deltamax.user_name", PO_VALUE(&encodersConfig.deltaMax_userName), "registered user name")
		("deltamax.license_key", PO_VALUE(&encodersConfig.deltaMax_licenseKey), "license key string")
	;
#endif // SCARAB_DELTAMAX

#if SCARAB_XDELTA
	// Expose xdelta options.	
	config_file_options.add_options()
		("xdelta.pack_files", PO_VALUE(&encodersConfig.xdelta_packFiles), "files to pack with xdelta")
		("xdelta.DJW", po::bool_switch(&encodersConfig.xdelta_config.DJW)->default_value(false), "use DJW static huffman")
		("xdelta.FGK", po::bool_switch(&encodersConfig.xdelta_config.FGK)->default_value(false), "use FGK adaptive huffman")
		("xdelta.LZMA", po::bool_switch(&encodersConfig.xdelta_config.LZMA)->default_value(false), "use LZMA secondary")
		("xdelta.nodata", po::bool_switch(&encodersConfig.xdelta_config.nodata)->default_value(false), "disable secondary compression of the data section")
		("xdelta.noinst", po::bool_switch(&encodersConfig.xdelta_config.noinst)->default_value(false), "disable secondary compression of the inst section")
		("xdelta.noaddr", po::bool_switch(&encodersConfig.xdelta_config.noaddr)->default_value(false), "disable secondary compression of the addr section")
		("xdelta.adler32", po::bool_switch(&encodersConfig.xdelta_config.adler32)->default_value(false), "enable checksum computation in the encoder")
		("xdelta.adler32_nover", po::bool_switch(&encodersConfig.xdelta_config.adler32_nover)->default_value(false), "disable checksum verification in the decoder")
		("xdelta.be_greedy", po::bool_switch(&encodersConfig.xdelta_config.beGreedy)->default_value(false), "disable the '1.5-pass algorithm', instead use greedy matching. Greedy is off by default.")
		("xdelta.compression", po::value(&encodersConfig.xdelta_config.compression)->default_value(0), "1 through 9 (0 corresponds to the NOCOMPRESS flag, and is independent of compression level)")
		;
#endif // SCARAB_XDELTA

	po::variables_map vm;
	po::store(po::COMMAND_LINE_PARSER(argc, argv).options(command_line_options).run(), vm);

	if (argc == 1 || vm.count("help")) 
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
			_tcout << _T("can not open config file: ") << options.configFile << _T("\n");
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
	rab::DiffEncoders diffEncoders;

	try
	{
		EncodersConfig encodersConfig;
		result = ParseCommandLine( argc, argv, options, config, encodersConfig );
		if( result )
			return result;

		rab::RollABall rollABall;

#if SCARAB_XDELTA
		// Plug in xdelta encoder.
		xdelta::XdeltaEncoder xdeltaEncoder( encodersConfig.xdelta_config );
		if( encodersConfig.xdelta_packFiles.empty() )
			encodersConfig.xdelta_packFiles.push_back( _T(".*") );
		diffEncoders.AddEncoder( xdeltaEncoder, _T("xdelta"), encodersConfig.xdelta_packFiles );
#endif // SCARAB_XDELTA

#if SCARAB_DELTAMAX
		// Plug in DeltaMAX encoder.
		deltamax::DeltaMaxEncoder deltaMaxEncoder;
		deltaMaxEncoder.SetUserLicense( encodersConfig.deltaMax_userName.c_str(), encodersConfig.deltaMax_licenseKey.c_str() );
		diffEncoders.AddExternalEncoder( deltaMaxEncoder, _T("deltamax"), encodersConfig.deltaMax_packFiles );
#endif // SCARAB_DELTAMAX

		_tostream nil_out( SCARAB_NEW dung::nil_buf );

		if( rollABall.ProcessData( options, config, diffEncoders, options.quiet ? nil_out : _tcout ) )
			result = 0;

		delete nil_out.rdbuf( NULL );
	}
	catch(std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	std::locale::global(std::locale::classic());
	
	return result;
}
