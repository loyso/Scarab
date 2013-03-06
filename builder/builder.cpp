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
	std::string deltaMax_userName;
	std::string deltaMax_licenseKey;
#endif // SCARAB_DELTAMAX

#if SCARAB_XDELTA
	rab::Config::StringValues_t xdelta_packFiles;
	xdelta::Config xdelta_config;
#endif // SCARAB_XDELTA
};

int ParseCommandLine( int argc, _TCHAR** argv, rab::Options& options, rab::Config& config, EncodersConfig& encodersConfig )
{
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
		("quiet,Q", po::bool_switch(&options.quiet)->default_value(false), "quiet mode")
		("produce_temp", po::bool_switch(&options.produceTemp)->default_value(false), "create temp files in addition to archive")
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
		("zip.compression", po::wvalue(&config.zipCompressionLevel)->default_value(0), "1 through 9 (0 corresponds to the NO_COMPRESSION")		
		;

#if SCARAB_DELTAMAX
	// Expose DeltaMAX options.	
	config_file_options.add_options()
		("deltamax.pack_files", po::wvalue(&encodersConfig.deltaMax_packFiles), "files to pack with DeltaMAX")
		("deltamax.user_name", po::value(&encodersConfig.deltaMax_userName), "registered user name")
		("deltamax.license_key", po::value(&encodersConfig.deltaMax_licenseKey), "license key string")
	;
#endif // SCARAB_DELTAMAX

#if SCARAB_XDELTA
	// Expose xdelta options.	
	config_file_options.add_options()
		("xdelta.pack_files", po::wvalue(&encodersConfig.xdelta_packFiles), "files to pack with xdelta")
		("xdelta.DJW", po::bool_switch(&encodersConfig.xdelta_config.DJW)->default_value(false), "use DJW static huffman")
		("xdelta.FGK", po::bool_switch(&encodersConfig.xdelta_config.FGK)->default_value(false), "use FGK adaptive huffman")
		("xdelta.LZMA", po::bool_switch(&encodersConfig.xdelta_config.LZMA)->default_value(false), "use LZMA secondary")
		("xdelta.nodata", po::bool_switch(&encodersConfig.xdelta_config.nodata)->default_value(false), "disable secondary compression of the data section")
		("xdelta.noinst", po::bool_switch(&encodersConfig.xdelta_config.noinst)->default_value(false), "disable secondary compression of the inst section")
		("xdelta.noaddr", po::bool_switch(&encodersConfig.xdelta_config.noaddr)->default_value(false), "disable secondary compression of the addr section")
		("xdelta.adler32", po::bool_switch(&encodersConfig.xdelta_config.adler32)->default_value(false), "enable checksum computation in the encoder")
		("xdelta.adler32_nover", po::bool_switch(&encodersConfig.xdelta_config.adler32_nover)->default_value(false), "disable checksum verification in the decoder")
		("xdelta.be_greedy", po::bool_switch(&encodersConfig.xdelta_config.beGreedy)->default_value(false), "disable the '1.5-pass algorithm', instead use greedy matching. Greedy is off by default.")
		("xdelta.compression", po::value(&encodersConfig.xdelta_config.compression)->default_value(0), "1 through 9 (0 corresponds to the NOCOMPRESS flag, and is independent of compression level")
		;
#endif // SCARAB_XDELTA

	po::variables_map vm;
	po::store(po::wcommand_line_parser(argc, argv).options(command_line_options).run(), vm);

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

		std::wostream nil_out( SCARAB_NEW dung::nil_wbuf );

		if( rollABall.ProcessData( options, config, diffEncoders, options.quiet ? nil_out : std::wcout ) )
			result = 0;

		delete nil_out.rdbuf( NULL );
	}
	catch(std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	std::locale::global(std::locale::classic());
	
	return result;
}
