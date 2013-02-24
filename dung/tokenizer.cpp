#include "tokenizer.h"

#include "memory.h"

namespace dung
{
	static const int WORD_MAX_LENGTH = 512;
	static const int STRING_MAX_LENGTH = 24*1024;
	static const int NUMBER_MAX_LENGTH = 128;
}

dung::CharacterSet::CharacterSet()
{
	Clear();
	return;
}

dung::CharacterSet::~CharacterSet()
{
}

void dung::CharacterSet::AddChar( unsigned char ch )
{
	mh[ ch ] = 1;
	return;
}

void dung::CharacterSet::AddChar( unsigned char chMin, unsigned char chMax )
{
	for( int i = chMin;  i <= chMax;  i++ )
		mh[ i ] = 1;
}

void dung::CharacterSet::RemoveChar( unsigned char ch )
{
	mh[ ch ] = 0;
	return;
}

void dung::CharacterSet::RemoveChar( unsigned char chMin, unsigned char chMax )
{
	for( int i = chMin;  i <= chMax;  i++ )
		mh[ i ] = 0;
}

bool dung::CharacterSet::IsHere( unsigned char ch ) const
{
	return mh[ ch ] != 0;
}

void dung::CharacterSet::Clear()
{
	memset( mh, 0, sizeof(char) * 256 );
}


dung::TextTokenizer::TextTokenizer() :
	m_szText( NULL ),
	m_pTextEnd( NULL ),
	m_lastParsed( dung::TextTokenizer::LAST_PARSED_NOTHING ),
	m_szWord( NULL ),
	m_szString( NULL ),
	m_szNumber( NULL ),
	mhSymbol( '\0' ),
	m_pPos( NULL )
{
}

dung::TextTokenizer::~TextTokenizer()
{
	Close();
}

bool dung::TextTokenizer::Open( const char *szText, const char *szTextEnd )
{
	if( !szText || !szTextEnd || szTextEnd <= szText )
	{
		SCARAB_ASSERT( false && "TextTokenizer::Open( const char *, const char *) : bad ptr" );
		return false;
	}

	if( m_szText )
	{
		SCARAB_ASSERT( false && "TextTokenizer::Open : reentry. Was opened yet." );
		return false;
	}

	m_szText = szText;
	m_pTextEnd = szTextEnd;
	m_pPos = m_szText;

	SCARAB_ASSERT( !m_szWord && !m_szString );
	m_szWord = SCARAB_NEW char[ WORD_MAX_LENGTH ];
	*m_szWord = '\0';
	m_szString = SCARAB_NEW char[ STRING_MAX_LENGTH ];
	*m_szString = '\0';
	m_szNumber = SCARAB_NEW char[ NUMBER_MAX_LENGTH ];
	*m_szNumber = '\0';

	return true;
}

bool dung::TextTokenizer::Open( const char *szText, int iTextLength )
{
	if( iTextLength <= 0 )
	{
		SCARAB_ASSERT( false && "TextTokenizer::Open( const char *, int ) : bad length." );
		return false;
	}

	if( !szText )
	{
		SCARAB_ASSERT( false && "TextTokenizer::Open( const char *, int ) : bad ptr." );
		return false;
	}

	if( m_szText )
	{
		SCARAB_ASSERT( false && "TextTokenizer::Open( const char *, int ) : reentry. Was opened yet." );
		return false;
	}

	return Open( szText, szText + iTextLength );
}

void dung::TextTokenizer::Close()
{
	m_szText = m_pTextEnd = m_pPos = NULL;
	mhSymbol = '\0';
	m_lastParsed = LAST_PARSED_NOTHING;
	
	delete[] m_szWord;
	m_szWord = NULL;
	
	delete[] m_szString;
	m_szString = NULL;

	delete[] m_szNumber;
	m_szNumber = NULL;
}

bool dung::TextTokenizer::ParseNext()
{
	m_lastParsed = LAST_PARSED_NOTHING;

	if( !m_szText )
	{
		SCARAB_ASSERT( false && "TextTokenizer::ParseNext : text wasn't opened." );
		return false;
	}

	if( !ParseFirstChar() )
		return false;

	SCARAB_ASSERT( m_lastParsed != LAST_PARSED_NOTHING );

	switch( m_lastParsed )
	{
	case LAST_PARSED_WORD:
		ParseWord(); break;

	case LAST_PARSED_STRING:
		ParseString(); break;

	case LAST_PARSED_NUMBER:
		ParseNumber(); break;

	case LAST_PARSED_SYMBOL:
		ParseSymbol(); break;
	};

	return( m_lastParsed != LAST_PARSED_NOTHING );
}

const char* dung::TextTokenizer::GetWord() const
{
	return m_szWord ? m_szWord : "";
}

bool dung::TextTokenizer::IsWord() const
{
	return( m_lastParsed == LAST_PARSED_WORD );
}

const char* dung::TextTokenizer::GetString() const
{
	return m_szString ? m_szString : "";
}

bool dung::TextTokenizer::IsString() const
{
	return( m_lastParsed == LAST_PARSED_STRING );
}

const char* dung::TextTokenizer::GetNumber() const
{
	return m_szNumber;
}

bool dung::TextTokenizer::IsNumber() const
{
	return( m_lastParsed == LAST_PARSED_NUMBER );
}

char dung::TextTokenizer::GetSymbol() const
{
	return mhSymbol;
}

bool dung::TextTokenizer::IsSymbol() const
{
	return( m_lastParsed == LAST_PARSED_SYMBOL );
}

void dung::TextTokenizer::SetCharacterRule( char ch, CharacterParsingRule rule )
{
	SetCharacterRule( ch, ch, rule );
}

void dung::TextTokenizer::SetCharacterRule( char chMin, char chMax, CharacterParsingRule rule )
{
	switch( rule )
	{
	case PARSING_RULE_WORD_START:
		m_wordStartSet.AddChar( chMin, chMax );
		m_stringQuotaSet.RemoveChar( chMin, chMax );
		m_numberStartSet.RemoveChar( chMin, chMax );
		m_symbolSet.RemoveChar( chMin, chMax );
		break;

	case PARSING_RULE_WORD_CONTENT:
		m_wordSet.AddChar( chMin, chMax );
		break;

	case PARSING_RULE_STRING_QUOTA:
		m_stringQuotaSet.AddChar( chMin, chMax );
		m_wordStartSet.RemoveChar( chMin, chMax );
		m_numberStartSet.RemoveChar( chMin, chMax );
		m_symbolSet.RemoveChar( chMin, chMax );
		break;

	case PARSING_RULE_NUMBER_START:
		m_numberStartSet.AddChar( chMin, chMax );
		m_stringQuotaSet.RemoveChar( chMin, chMax );
		m_wordStartSet.RemoveChar( chMin, chMax );
		m_symbolSet.RemoveChar( chMin, chMax );
		break;

	case PARSING_RULE_NUMBER_CONTENT:
		m_numberSet.AddChar( chMin, chMax );
		break;

	case PARSING_RULE_SYMBOL:
		m_symbolSet.AddChar( chMin, chMax );
		m_stringQuotaSet.RemoveChar( chMin, chMax );
		m_wordStartSet.RemoveChar( chMin, chMax );
		m_numberStartSet.RemoveChar( chMin, chMax );
		m_wordSet.RemoveChar( chMin, chMax );
		m_numberSet.RemoveChar( chMin, chMax );
		break;

	case PARSING_RULE_IGNORE:
        m_wordStartSet.RemoveChar( chMin, chMax );        
		m_stringQuotaSet.RemoveChar( chMin, chMax );        
		m_numberStartSet.RemoveChar( chMin, chMax );        
		m_symbolSet.RemoveChar( chMin, chMax );   
		break;

	default:
		SCARAB_ASSERT( false && "TextTokenizer::SetCharacterRule : unknown rule.");
	}
}


void dung::TextTokenizer::ClearCharsets()
{
	m_wordStartSet.Clear();
	m_stringQuotaSet.Clear();
	m_numberStartSet.Clear();
	m_symbolSet.Clear();
	m_wordSet.Clear();
	m_numberSet.Clear();
}

void dung::TextTokenizer::SetDefaultCharsets()
{
	m_wordStartSet.Clear();
	m_wordStartSet.AddChar( 'A', 'Z' );
	m_wordStartSet.AddChar( 'a', 'z' );
	m_wordStartSet.AddChar( '_' );

	m_stringQuotaSet.Clear();
	m_stringQuotaSet.AddChar( '\"' );

	m_numberStartSet.Clear();
	m_numberStartSet.AddChar( '0', '9' );
	m_numberStartSet.AddChar( '+' );
	m_numberStartSet.AddChar( '-' );

	m_symbolSet.Clear();
	m_symbolSet.AddChar( '\n' );

	m_wordSet.Clear();
	m_wordSet.AddChar( 'A', 'Z' );
	m_wordSet.AddChar( 'a', 'z' );
	m_wordSet.AddChar( '_' );
	m_wordSet.AddChar( '0', '9' );

    m_numberSet.Clear();
	m_numberSet.AddChar( '0', '9' );
	m_numberSet.AddChar( '.' );
}

bool dung::TextTokenizer::ParseFirstChar()
{
	SCARAB_ASSERT( m_pPos ); 
	m_lastParsed = LAST_PARSED_NOTHING;

	// ignore junk characters
	for( ; m_pPos < m_pTextEnd && IsIgnorableChar( *m_pPos ); m_pPos++ )
	{}

	if( m_pPos >= m_pTextEnd )
		return false;

	mhFirstChar = *m_pPos++;

	if( IsStringQuotaChar( mhFirstChar ) )
		m_lastParsed = LAST_PARSED_STRING;
	else if( IsWordStartChar( mhFirstChar ) )
		m_lastParsed = LAST_PARSED_WORD;
	else if( IsNumberStartChar( mhFirstChar ) )
		m_lastParsed = LAST_PARSED_NUMBER;
	else if( IsSymbolChar( mhFirstChar ) )
		m_lastParsed = LAST_PARSED_SYMBOL;
	else
	{
		SCARAB_ASSERT( false &&	"TextTokenizer::ParseFirstChar : unexecutable code executes." );
		return false;
	}

	return true;
}

void dung::TextTokenizer::ParseWord()
{
	SCARAB_ASSERT( m_lastParsed == LAST_PARSED_WORD );

	char * pBuf = m_szWord;
	char * pBufEnd = m_szWord + WORD_MAX_LENGTH;

	*pBuf++ = mhFirstChar;

	for( ; m_pPos < m_pTextEnd && IsWordChar( *m_pPos ); m_pPos++ )
	{
		if( pBuf < pBufEnd )
			*pBuf++ = *m_pPos;
	}

	if( pBuf != pBufEnd )
		*pBuf = '\0';
	else
	{
		*(pBuf - 1) = '\0';
		SCARAB_ASSERT( false && "TextTokenizer::ParseWord : word buffer overflow.");
	}
}

void dung::TextTokenizer::ParseString()
{
	SCARAB_ASSERT( m_lastParsed == LAST_PARSED_STRING );

	char * pBuf = m_szString;
	char * pBufEnd = m_szString + STRING_MAX_LENGTH;

	while( m_pPos < m_pTextEnd )
	{
		if( IsStringQuotaChar( *m_pPos ) )
		{
			char* pForward = (char*) m_pPos + 1;
			if( pForward < m_pTextEnd && IsStringQuotaChar( *pForward ) )
			{//	double quote - copy as single
				m_pPos = pForward;
			} else // single quote - end of string
				break;
		}
		if( pBuf < pBufEnd )
			*pBuf++ = *m_pPos;
		m_pPos++;
	}

	if( m_pPos < m_pTextEnd )
		*m_pPos++;

	if( pBuf != pBufEnd )
		*pBuf = '\0';
	else
	{
		*(pBuf - 1) = '\0';
		SCARAB_ASSERT( false && "TextTokenizer::ParseString : string buffer overflow.");
	}
}

void dung::TextTokenizer::ParseNumber()
{
	SCARAB_ASSERT( m_lastParsed == LAST_PARSED_NUMBER );

	char * pBuf = m_szNumber;
	char * pBufEnd = m_szNumber + NUMBER_MAX_LENGTH;

	*pBuf++ = mhFirstChar;

	for( ; m_pPos < pBufEnd && IsNumberChar( *m_pPos ); m_pPos++ )
	{
		if( pBuf < pBufEnd )
			*pBuf++ = *m_pPos;
	}

	if( pBuf != pBufEnd )
		*pBuf = '\0';
	else
	{
		*(pBuf - 1) = '\0';
		SCARAB_ASSERT( false && "TextTokenizer::ParseNumber : number buffer overflow.");
	}
}

void dung::TextTokenizer::ParseSymbol()
{
	SCARAB_ASSERT( m_lastParsed == LAST_PARSED_SYMBOL );

	mhSymbol = mhFirstChar;
}

bool dung::TextTokenizer::IsIgnorableChar( char ch ) const
{
	return !( IsWordStartChar( ch ) || IsStringQuotaChar( ch ) ||
		IsNumberStartChar( ch ) || IsSymbolChar( ch ) );
}

bool dung::TextTokenizer::IsWordStartChar( char ch ) const
{
	return m_wordStartSet.IsHere( ch );
}

bool dung::TextTokenizer::IsStringQuotaChar( char ch ) const
{
	return m_stringQuotaSet.IsHere( ch );
}


bool dung::TextTokenizer::IsNumberStartChar( char ch ) const
{
	return m_numberStartSet.IsHere( ch );
}

bool dung::TextTokenizer::IsSymbolChar( char ch ) const
{
	return m_symbolSet.IsHere( ch );
}

bool dung::TextTokenizer::IsWordChar( char ch ) const
{
	return m_wordSet.IsHere( ch );
}

bool dung::TextTokenizer::IsNumberChar( char ch ) const
{
	return m_numberSet.IsHere( ch );
}
