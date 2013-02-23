#include "registry_parser.h"

namespace hatch
{
	bool Cmp( const char* t1, const char* t2 )
	{
		return strcmp( t1, t2) == 0;
	}
}

hatch::RegistryParser::RegistryParser()
{
}

hatch::RegistryParser::~RegistryParser()
{
}

void hatch::RegistryParser::Open( const char* text, size_t size )
{
	m_tokenizer.SetUtf8Charsets();
	m_tokenizer.SetCharacterRule( '=', dung::PARSING_RULE_SYMBOL );
	m_tokenizer.SetCharacterRule( '{', dung::PARSING_RULE_SYMBOL );
	m_tokenizer.SetCharacterRule( '}', dung::PARSING_RULE_SYMBOL );

	m_tokenizer.Open( text, size );
}

void hatch::RegistryParser::Close()
{
	m_tokenizer.Close();
}

bool hatch::RegistryParser::SkipEndLines()
{
	do
	{
		if( !m_tokenizer.ParseNext() )
			return false;
	} while ( m_tokenizer.IsSymbol() && m_tokenizer.GetSymbol() == '\n' );

	return true;
}

bool hatch::RegistryParser::Parse( Registry& registry )
{
	if( !SkipEndLines() )
		return false;

	bool process = true;
	do
	{
		if( m_tokenizer.IsWord() )
		{
			const char* key = m_tokenizer.GetWord();
			if( Cmp( "new_version", key ) )
			{
				if( !ParseWordValue( registry.newVersion ) )
					return false;
			}
			else if( Cmp( "old_version", key ) )
			{
				if( !ParseWordValue( registry.oldVersion ) )
					return false;
			}
			else if( Cmp( "file", key ) )
			{
				if( !SkipEndLines() )
					return false;

				if( !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '{' )
					return false;

				RegistryAction* pAction = SCARAB_NEW RegistryAction;
				registry.actions.push_back( pAction );

				if( !ParseFile( registry, *pAction ) )
					return false;
			}
		}

		if( !m_tokenizer.ParseNext() )
			process = false;
	} while( process );

	return true;
}

bool hatch::RegistryParser::ParseWordValue( String_t& value )
{
	if( !m_tokenizer.ParseNext() )
		return false;

	if( !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=' )
		return false;

	if( !m_tokenizer.ParseNext() )
		return false;

	if( !m_tokenizer.IsWord() )
		return false;
	
	value = m_tokenizer.GetWord();
	return true;
}

bool hatch::RegistryParser::ParseFile( Registry& registry, RegistryAction& action )
{
	if( !SkipEndLines() )
		return false;

	bool process = true;
	do
	{
		if( m_tokenizer.IsWord() )
		{
			const char* key = m_tokenizer.GetWord();
			if( Cmp( "action", key ) )
			{
				String_t value;
				if( !ParseWordValue( value ) )
					return false;
			}
			else if( Cmp( "diff_path", key ) )
			{
				String_t value;
				if( !ParseWordValue( value ) )
					return false;
			}
			else if( Cmp( "old_path", key ) )
			{
				String_t value;
				if( !ParseWordValue( value ) )
					return false;
			}
			else if( Cmp( "new_path", key ) )
			{
				String_t value;
				if( !ParseWordValue( value ) )
					return false;
			}
			else if( Cmp( "old_sha1", key ) )
			{
				String_t value;
				if( !ParseWordValue( value ) )
					return false;
			}
			else if( Cmp( "new_sha1", key ) )
			{
				String_t value;
				if( !ParseWordValue( value ) )
					return false;
			}
			else if( Cmp( "old_size", key ) )
			{
				String_t value;
				if( !ParseWordValue( value ) )
					return false;
			}
			else if( Cmp( "new_size", key ) )
			{
				String_t value;
				if( !ParseWordValue( value ) )
					return false;
			}
		}
		else if( m_tokenizer.IsSymbol() && m_tokenizer.GetSymbol() == '}' )
			return true;

		if( !m_tokenizer.ParseNext() )
			process = false;
	} while( process );

	return true;
}


hatch::Registry::Registry()
{
}

hatch::Registry::~Registry()
{
	DeleteContainer( actions );
}
