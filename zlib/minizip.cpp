// Based on a sample part of the MiniZip project.
#include "minizip.h"

#ifndef WIN32
#	ifndef __USE_FILE_OFFSET64
#		define __USE_FILE_OFFSET64
#	endif
#	ifndef __USE_LARGEFILE64
#		define __USE_LARGEFILE64
#	endif
#	ifndef _LARGEFILE64_SOURCE
#		define _LARGEFILE64_SOURCE
#	endif
#	ifndef _FILE_OFFSET_BIT
#		define _FILE_OFFSET_BIT 64
#	endif
#endif

#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

namespace loc = boost::locale;
namespace fs = boost::filesystem;

#ifdef WIN32
#	include <direct.h>
#	include <io.h>
#else
#	include <unistd.h>
#	include <utime.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#endif

#include "zip.h"

#ifdef WIN32
#	define USEWIN32IOAPI
#	include "iowin32.h"
#endif

#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef WIN32
uLong filetime( const char *f, tm_zip *tmzip, uLong *dt )
{
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}
#else
uLong filetime( char *f, tm_zip *tmzip, uLong *dt )
{
  int ret=0;
  struct stat s;        /* results of stat() */
  struct tm* filedate;
  time_t tm_t=0;

  if (strcmp(f,"-")!=0)
  {
    char name[MAXFILENAME+1];
    int len = strlen(f);
    if (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    if (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stating a file with / appended */
    if (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  return ret;
}
#endif

int isLargeFile(const char* filename)
{
  int largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  if(pFile != NULL)
  {
	  int n = FSEEKO_FUNC(pFile, 0, SEEK_END);
	  pos = FTELLO_FUNC(pFile);

	  if(pos >= 0xffffffff)
		  largeFile = 1;

	  fclose(pFile);
  }

 return largeFile;
}

namespace zip
{
	static const int MAX_ERROR_STRING = 4096;
}

zip::ZipArchiveOutput::ZipArchiveOutput()
	: m_archiveName()
	, m_utf8fileNames( false )
	, opt_compress_level( CompressionLevel::NO_COMPRESSION )
	, password()
	, err( ZIP_OK )
	, zf()
{
	m_errorMessage = new char[ MAX_ERROR_STRING ];
}

zip::ZipArchiveOutput::~ZipArchiveOutput()
{
	delete[] m_errorMessage;
}

bool zip::ZipArchiveOutput::Open( String_t const& archiveName, bool utf8fileNames, int compressionLevel )
{
	m_archiveName = archiveName;
	m_utf8fileNames = utf8fileNames;
	opt_compress_level = compressionLevel;

	const fs::path& archiveNamePath = archiveName;
	std::string zipArchiveName = archiveNamePath.generic_string();
#ifdef USEWIN32IOAPI
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64A(&ffunc);
	zf = zipOpen2_64(zipArchiveName.c_str(),false,NULL,&ffunc);
#else
	zf = zipOpen64(zipArchiveName.c_str(),false);
#endif

	if (zf == NULL)
	{
		err = ZIP_ERRNO;
		sprintf( m_errorMessage, "Can't open %s\n", m_archiveName.c_str() );
		return false;
	}
	
	return true;
}

bool zip::ZipArchiveOutput::WriteFile( String_t const& fileName, const void* pMemoryBlock, size_t size )
{
    const boost::filesystem::path& filenamePath = fileName;
	std::string c_str = filenamePath.generic_string();
	const char* filenameinzip = c_str.c_str();
    zip_fileinfo zi;
    unsigned long crcFile=0;
    int zip64 = 0;

    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;
    filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);
    if( password != NULL && err == ZIP_OK )
	{
		unsigned long crcFile=0;
		crcFile = crc32(crcFile,(Bytef*)pMemoryBlock,size);
	}

    zip64 = false; // isLargeFile(filenameinzip);
	
	std::string savefilenameinzip = loc::conv::utf_to_utf<char>( filenamePath.generic_wstring() );

	const uLong zipVersion = 36; // pkzip 6.3.* started to use utf-8 filenames
	const uLong zipUtf8FilenamesEncoding = 1 << 11;

    err = zipOpenNewFileInZip4_64(zf,savefilenameinzip.c_str(),&zi,
                        NULL,0,NULL,0,NULL /* comment*/,
                        (opt_compress_level != 0) ? Z_DEFLATED : 0,
                        opt_compress_level,0,
                        -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                        password,crcFile, zipVersion, zipUtf8FilenamesEncoding, zip64);

    if (err != ZIP_OK)
        sprintf( m_errorMessage, "error in creating new %s in zipfile\n", filenameinzip );

    if (err == ZIP_OK)
		err = zipWriteInFileInZip (zf,pMemoryBlock,size);
	
	if (err<0)
		sprintf( m_errorMessage, "error in writing %s in the zipfile\n", filenameinzip);

    if (err<0)
        err=ZIP_ERRNO;
    else
    {
        err = zipCloseFileInZip(zf);
        if (err!=ZIP_OK)
            sprintf( m_errorMessage, "error in closing %s in the zipfile\n", filenameinzip);
    }

	return err == ZIP_OK;
}

bool zip::ZipArchiveOutput::Close()
{
	int errclose = zipClose(zf, NULL);
	return errclose == ZIP_OK;
}

const char* zip::ZipArchiveOutput::ErrorMessage() const
{
	return m_errorMessage;
}
