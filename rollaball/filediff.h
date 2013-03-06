#pragma once

#include "rollaball.h"

namespace dung
{
	class DiffEncoder_i;
	class DiffEncoderExternal_i;
}

namespace rab
{
	struct FolderInfo;
	struct Config;
}

namespace rab
{
	class DiffEncoders
	{
	public:
		DiffEncoders();
		~DiffEncoders();

		void AddEncoder( dung::DiffEncoder_i& diffEncoder, DiffMethod_t const& encoderName, Config::StringValues_t const& packFiles );
		void AddExternalEncoder( dung::DiffEncoderExternal_i& diffEncoder, DiffMethod_t const& encoderName, Config::StringValues_t const& packFiles );

		dung::DiffEncoder_i* FindEncoder( String_t const& fileName, DiffMethod_t& encoderName ) const;
		dung::DiffEncoderExternal_i* FindExternalEncoder( String_t const& fileName, DiffMethod_t& encoderName ) const;

	private:
		struct EncoderEntry
		{
			DiffMethod_t m_encoderName;
			Config::RegexValues_t m_packFiles;
			dung::DiffEncoder_i* m_pDiffEncoder;
		};

		struct ExternalEncoderEntry
		{
			DiffMethod_t m_encoderName;
			Config::RegexValues_t m_packFiles;
			dung::DiffEncoderExternal_i* m_pDiffEncoder;
		};

		typedef std::vector< EncoderEntry* > DiffEncoders_t;
		DiffEncoders_t m_diffEncoders;

		typedef std::vector< ExternalEncoderEntry* > DiffExternalEncoders_t;
		DiffExternalEncoders_t m_diffEncodersExternal;
	};

	bool BuildDiffs( Options const& options, Config const& config, DiffEncoders const& diffEncoders, FolderInfo const& rootFolder, 
		PackageOutput_t& output, LogOutput_t& out );
}
