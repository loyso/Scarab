#pragma once

#include <string>

namespace zip
{
	typedef std::string String_t;
	typedef unsigned char Byte_t;

	class ZipArchiveInput
	{
	public:
		ZipArchiveInput( String_t const& archiveName );

		bool ReadFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size );
		void Close();

	private:
		String_t m_archiveName;
		const char* password;

		typedef void* UnzipFile_t;
		UnzipFile_t uf;
	};
}
