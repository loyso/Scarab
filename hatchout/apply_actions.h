#pragma once

#include "hatchout.h"
#include "registry_parser.h"

#include <unordered_map>

namespace zip
{
	class ZipArchiveInput;
}

namespace dung
{
	class DiffDecoder_i;
	class DiffDecoderExternal_i;
}

namespace hatch
{
	class DiffDecoders
	{
	public:
		DiffDecoders();
		~DiffDecoders();

		void AddDecoder( dung::DiffDecoder_i& diffDecoder, const char* decoderName );
		void AddExternalDecoder( dung::DiffDecoderExternal_i& diffDecoder, const char* decoderName );

		dung::DiffDecoder_i* FindDecoder( String_t const& decoderName ) const;
		dung::DiffDecoderExternal_i* FindExternalDecoder( String_t const& decoderName ) const;

	private:
		typedef std::unordered_map< String_t, dung::DiffDecoder_i* > NameToDecoder_t;
		NameToDecoder_t m_nameToDecoder;

		typedef std::unordered_map< String_t, dung::DiffDecoderExternal_i* > NameToDecoderExternal_t;
		NameToDecoderExternal_t m_nameToDecoderExternal;
	};

	bool ApplyActions( Options const& options, DiffDecoders const& diffDecoders, Registry const& registry, zip::ZipArchiveInput& zipInput, LogOutput_t& out );
}
