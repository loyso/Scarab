#pragma once

#include "dung.h"

namespace dung
{
	class CharacterSet
	{
	public:
		CharacterSet();
		~CharacterSet();

		void AddChar ( unsigned char ch );
		void AddChar ( unsigned char chMin, unsigned char chMax );
		void RemoveChar ( unsigned char ch );
		void RemoveChar ( unsigned char chMin, unsigned char chMax );
		bool IsHere ( unsigned char ch ) const;
		void Clear();

	private:
		unsigned char mh[256];
	};

	enum CharacterParsingRule
	{
		PARSING_RULE_WORD_START = 1,
		PARSING_RULE_WORD_CONTENT,
		PARSING_RULE_STRING_QUOTA,
		PARSING_RULE_NUMBER_START,
		PARSING_RULE_NUMBER_CONTENT,
		PARSING_RULE_SYMBOL,
		PARSING_RULE_IGNORE
	};

	class TextTokenizer
	{
	public:
		TextTokenizer();
		virtual ~TextTokenizer();

		bool Open( const char *szText, const char *szTextEnd );
		bool Open( const char *szText, int iTextLength );
		void Close();

		bool ParseNext();

		bool IsWord() const;
		const char* GetWord() const;

		bool IsString() const;
		const char* GetString() const;

		bool IsNumber() const;
		const char* GetNumber() const;

		bool IsSymbol() const;
		char GetSymbol() const;

		void SetCharacterRule( char ch, CharacterParsingRule rule );
		void SetCharacterRule( char chMin, char chMax, CharacterParsingRule rule );

		void ClearCharsets();
		void SetDefaultCharsets();

	private:
		bool ParseFirstChar();

		bool IsIgnorableChar( char ch ) const;
		bool IsWordStartChar( char ch ) const;
		bool IsStringQuotaChar( char ch ) const;
		bool IsNumberStartChar( char ch ) const;
		bool IsSymbolChar( char ch ) const;
		bool IsWordChar( char ch ) const;
		bool IsNumberChar( char ch ) const;

		void ParseWord();
		void ParseString();
		void ParseNumber();
		void ParseSymbol();

	private:
		enum LastParsed 
		{
			LAST_PARSED_NOTHING,
			LAST_PARSED_WORD,
			LAST_PARSED_STRING,
			LAST_PARSED_NUMBER,
			LAST_PARSED_SYMBOL
		};

		const char *m_szText;		
		const char *m_pTextEnd;		
		LastParsed m_lastParsed;	
		char *m_szWord;				
		char *m_szString;			
		char *m_szNumber;			
		char mhSymbol;				
		const char *m_pPos;			
		char mhFirstChar;			

		CharacterSet m_wordStartSet;	
		CharacterSet m_stringQuotaSet;	
		CharacterSet m_numberStartSet;	
		CharacterSet m_symbolSet;		
		CharacterSet m_wordSet;			
		CharacterSet m_numberSet;		
	};
}