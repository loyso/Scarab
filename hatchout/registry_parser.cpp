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
				{
					m_errorMessage << "for new_version";
					return false;
				}
			}
			else if( Cmp( "old_version", key ) )
			{
				if( !ParseStringValue( registry.oldVersion ) )
				{
					m_errorMessage << "for old_version";
					return false;
				}
			}
			else if( Cmp( "file", key ) )
			{
				if( !SkipEndLines() || !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '{' )
				{
					m_errorMessage << "'{' expected after 'file'";
					return false;
				}

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
	if( !m_tokenizer.ParseNext() || !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=' )
	{
		m_errorMessage << "=[Word identifier] expected ";
		return false;
	}

	if( !m_tokenizer.ParseNext() || !m_tokenizer.IsWord() )
	{
		m_errorMessage << "Word identifier expected ";
		return false;
	}
	
	value = m_tokenizer.GetWord();
	return true;
}

bool hatch::RegistryParser::ParseStringValue( String_t& value )
{
	if( !m_tokenizer.ParseNext() || !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=' )
	{
		m_errorMessage << "=[Quoted string] expected ";
		return false;
	}

	if( !m_tokenizer.ParseNext() || !m_tokenizer.IsString() )
	{
		m_errorMessage << "Quoted string expected ";
		return false;
	}

	value = m_tokenizer.GetString();
	return true;
}

bool hatch::RegistryParser::ParseNumValue( String_t& value )
{
	if( !m_tokenizer.ParseNext() || !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=' )
	{
		m_errorMessage << "=[Number] expected ";
		return false;
	}

	if( !m_tokenizer.ParseNext() || !m_tokenizer.IsNumber() )
	{
		m_errorMessage << "Number expected ";
		return false;
	}

	value = m_tokenizer.GetNumber();
	return true;
}

bool hatch::RegistryParser::ParseFile( Registry& registry, RegistryAction& action )
{
	if( !SkipEndLines() )
	{
		m_errorMessage << "Parameters expected after 'file {'";
		return false;
	}

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
				{
					m_errorMessage << "for action";
					return false;
				}

				if( !dung::StringToAction( value.c_str(), action.action ) )
				{
					m_errorMessage << "Unknown action " << value;
					return false;
				}
			}
			else if( Cmp( "diff_path", key ) )
			{
				if( !ParseStringValue( action.diff_path ) )
				{
					m_errorMessage << "for diff_path";
					return false;
				}
			}
			else if( Cmp( "old_path", key ) )
			{
				if( !ParseStringValue( action.old_path ) )
				{
					m_errorMessage << "for old_path";
					return false;
				}
			}
			else if( Cmp( "new_path", key ) )
			{
				if( !ParseStringValue( action.new_path ) )
				{
					m_errorMessage << "for new_path";
					return false;
				}
			}
			else if( Cmp( "diff_method", key ) )
			{
				if( !ParseWordValue( action.diff_method ) )
				{
					m_errorMessage << "for diff_method";
					return false;
				}
			}
			else if( Cmp( "old_sha1", key ) )
			{
				String_t value;
				if( !ParseStringValue( value ) )
				{
					m_errorMessage << "for old_sha1";
					return false;
				}

				if( !dung::StringToSHA1( value.c_str(), action.oldSha1 ))
				{
					m_errorMessage << "Can't convert " << value << " to old_sha1";
					return false;
				}

				SCARAB_ASSERT( dung::SHA1ToString( action.oldSha1 ) == value );
			}
			else if( Cmp( "new_sha1", key ) )
			{
				String_t value;
				if( !ParseStringValue( value ) )
				{
					m_errorMessage << "for new_sha1";
					return false;
				}

				if( !dung::StringToSHA1( value.c_str(), action.newSha1 ))
				{
					m_errorMessage << "Can't convert " << value << " to new_sha1";
					return false;
				}

				SCARAB_ASSERT( dung::SHA1ToString( action.newSha1 ) == value );
			}
			else if( Cmp( "old_size", key ) )
			{
				String_t value;
				if( !ParseNumValue( value ) )
				{
					m_errorMessage << "for old_size";
					return false;
				}
				action.oldSize = atol( value.c_str() );
			}
			else if( Cmp( "new_size", key ) )
			{
				String_t value;
				if( !ParseNumValue( value ) )
				{
					m_errorMessage << "for new_size";
					return false;
				}
				action.newSize = atol( value.c_str() );
			}
		}
		else if( m_tokenizer.IsSymbol() && m_tokenizer.GetSymbol() == '}' )
			return true;

		if( !m_tokenizer.ParseNext() )
		{
			m_errorMessage << "'}' expected for 'file'";
			process = false;
		}
	} while( process );

	return false;
}

hatch::String_t hatch::RegistryParser::ErrorMessage() const
{
	return m_errorMessage.str();
}


hatch::Registry::Registry()
{
}

hatch::Registry::~Registry()
{
	dung::DeleteContainer( actions );
}


hatch::RegistryAction::RegistryAction()
	: action( dung::Action::NONE )
	, newSize( -1 )
	, oldSize( -1 )
{
}

