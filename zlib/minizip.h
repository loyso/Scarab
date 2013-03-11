#pragma once

#include <dung/dung.h>

#include <string>
#include <sstream>

namespace zip
{
	typedef _tstring String_t;

	namespace CompressionLevel
	{
		enum Enum
		{
			  NO_COMPRESSION         = 0
			, BEST_SPEED             = 1
			, BEST_COMPRESSION       = 9
			, DEFAULT_COMPRESSION    = -1
		};
	}

	class ZipArchiveOutput
	{
	public:
		ZipArchiveOutput();
		~ZipArchiveOutput();

		bool Open( String_t const& archiveName, bool utf8fileNames, int compressionLevel );
		bool WriteFile( String_t const& fileName, const void* pMemoryBlock, size_t size );
		bool Close();

		_tstring ErrorMessage() const;

	private:
		String_t m_archiveName;
		bool m_utf8fileNames;
		
		_tstringstream m_errorMessage;
		int err;
		const char* password;
		int opt_compress_level;

		typedef void* ZipFile_t;
		ZipFile_t zf;
	};
}

