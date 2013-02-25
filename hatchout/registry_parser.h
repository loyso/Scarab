#pragma once

#include <dung/tokenizer.h>
#include <dung/sha1.h>
#include <dung/registry.h>

#include <string>
#include <sstream>
#include <vector>

namespace hatch
{
	struct Registry;
	struct RegistryAction;
}

namespace hatch
{
	typedef std::string String_t;
	typedef std::stringstream StringStream_t;

	class RegistryParser
	{
	public:
		RegistryParser();
		~RegistryParser();
	
		void Open( const char* text, size_t size );
		void Close();

		bool Parse( Registry& registry );
		String_t ErrorMessage() const;

	private:
		bool ParseWordValue( String_t& value );
		bool ParseStringValue( String_t& value );
		bool ParseNumValue( String_t& value );
		
		bool ParseFile( Registry& registry, RegistryAction& action );
		bool SkipEndLines();

		dung::TextTokenizer m_tokenizer;
		
		StringStream_t m_errorMessage;
	};

	struct Registry
	{
		Registry();
		~Registry();

		String_t newVersion;
		String_t oldVersion;

		typedef std::vector< RegistryAction* > Actions_t;
		Actions_t actions;
	};

	struct RegistryAction
	{
		RegistryAction();

		dung::Action::Enum action;
		dung::Sha1 newSha1, oldSha1;
		int newSize, oldSize;
		
		String_t diff_path;
		String_t new_path;
		String_t old_path;
	};
}
