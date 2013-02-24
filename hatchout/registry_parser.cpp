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
	m_tokenizer.SetDefaultCharsets();
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
				if( !ParseStringValue( registry.newVersion ) )
					return false;
			}
			else if( Cmp( "old_version", key ) )
			{
				if( !ParseStringValue( registry.oldVersion ) )
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

bool hatch::RegistryParser::ParseStringValue( String_t& value )
{
	if( !m_tokenizer.ParseNext() )
		return false;

	if( !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=' )
		return false;

	if( !m_tokenizer.ParseNext() )
		return false;

	if( !m_tokenizer.IsString() )
		return false;

	value = m_tokenizer.GetString();
	return true;
}

bool hatch::RegistryParser::ParseNumValue( String_t& value )
{
	if( !m_tokenizer.ParseNext() )
		return false;

	if( !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=' )
		return false;

	if( !m_tokenizer.ParseNext() )
		return false;

	if( !m_tokenizer.IsNumber() )
		return false;

	value = m_tokenizer.GetNumber();
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

				if( !dung::StringToAction( value.c_str(), action.action ) )
					return false;
			}
			else if( Cmp( "diff_path", key ) )
			{
				if( !ParseStringValue( action.diff_path ) )
					return false;				
			}
			else if( Cmp( "old_path", key ) )
			{
				if( !ParseStringValue( action.old_path ) )
					return false;
			}
			else if( Cmp( "new_path", key ) )
			{
				if( !ParseStringValue( action.new_path ) )
					return false;
			}
			else if( Cmp( "old_sha1", key ) )
			{
				String_t value;
				if( !ParseStringValue( value ) )
					return false;

				if( !dung::StringToSHA1( value.c_str(), action.oldSha1 ))
					return false;

				SCARAB_ASSERT( dung::SHA1ToString( action.oldSha1 ) == value );
			}
			else if( Cmp( "new_sha1", key ) )
			{
				String_t value;
				if( !ParseStringValue( value ) )
					return false;

				if( !dung::StringToSHA1( value.c_str(), action.newSha1 ))
					return false;

				SCARAB_ASSERT( dung::SHA1ToString( action.newSha1 ) == value );
			}
			else if( Cmp( "old_size", key ) )
			{
				String_t value;
				if( !ParseNumValue( value ) )
					return false;
				action.oldSize = atol( value.c_str() );
			}
			else if( Cmp( "new_size", key ) )
			{
				String_t value;
				if( !ParseNumValue( value ) )
					return false;
				action.newSize = atol( value.c_str() );
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


hatch::RegistryAction::RegistryAction()
	: action( dung::Action::NONE )
	, newSize( -1 )
	, oldSize( -1 )
{
}

