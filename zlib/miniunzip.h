#pragma once

#include <string>
#include <unordered_map>

namespace zip
{
	typedef std::string String_t;
	typedef unsigned char Byte_t;

	class ZipArchiveInput
	{
	public:
		ZipArchiveInput();
		~ZipArchiveInput();

		bool Open( String_t const& archiveName, bool caseSensitive );
		bool Close();

		bool LocateAndReadFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size );
		bool ReadFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size );

		const char* ErrorMessage() const;

	private:
		bool Index();
		bool ReadCurrentFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size );

		struct ZipEntry
		{
			unsigned long pos_in_zip_directory;   // offset in zip file directory
			unsigned long num_of_file;            // # of file
		};

		typedef std::unordered_map< String_t, ZipEntry > NameToEntry_t;
		NameToEntry_t m_nameToEntry;

		String_t m_archiveName;
		const char* password;

		typedef void* UnzipFile_t;
		UnzipFile_t uf;

		char* m_errorMessage;
		bool m_caseSensitive;
	};

	int ZipCreateDirectory( const char* path );
	bool ZipCreateDirectories( const char* path );
	bool FileSize( const char* path, size_t& size );
}
