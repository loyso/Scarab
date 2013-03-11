// Based on a sample part of the MiniZip project.
#include "miniunzip.h"

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

#ifdef WIN32
#	include <direct.h>
#	include <io.h>
#else
#	include <unistd.h>
#	include <utime.h>
#endif

#include "unzip.h"

#define MAXFILENAME (256)

#ifdef WIN32
#	define USEWIN32IOAPI
#	include "iowin32.h"
#endif

/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
void ChangeFileDate( const char *filename, uLong dosdate, tm_unz tmu_date )
{
#ifdef WIN32
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#else
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#endif
}

namespace zip
{
	static const int MAX_ERROR_STRING = 4096;
}

zip::ZipArchiveInput::ZipArchiveInput()
	: m_archiveName()
	, uf()
	, password()
{
}

zip::ZipArchiveInput::~ZipArchiveInput()
{
}

bool zip::ZipArchiveInput::Open( String_t const& archiveName, bool caseSensitive )
{
	m_archiveName = archiveName;
	m_caseSensitive = caseSensitive;

#ifdef USEWIN32IOAPI
	zlib_filefunc64_def ffunc;
#endif

#ifdef USEWIN32IOAPI
#	ifdef SCARAB_WCHAR_MODE
	fill_win32_filefunc64W(&ffunc);
#	else
	fill_win32_filefunc64A(&ffunc);
#	endif
	uf = unzOpen2_64(m_archiveName.c_str(),&ffunc);
#else
	uf = unzOpen64(m_archiveName.c_str());
#endif // USEWIN32IOAPI

	if (uf==NULL)
	{
		m_errorMessage << _T("Can't open ") << m_archiveName << std::endl;
		return false;
	}

	return Index();
}

bool zip::ZipArchiveInput::LocateAndReadFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size )
{
	int err = UNZ_OK;
#ifdef SCARAB_WCHAR_MODE
	std::string fileNameInZip = utf_convert::as_utf8( fileName );
#else
	std::string fileNameInZip = fileName;
#endif
	if ( unzLocateFile( uf, fileNameInZip.c_str(), m_caseSensitive ) != UNZ_OK )
	{
		m_errorMessage << "file " << fileName << " not found in the zipfile" << std::endl;
		return false;
	}

	return ReadCurrentFile( fileName, pMemoryBlock, size );
}

bool zip::ZipArchiveInput::ReadFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size )
{
	String_t fileNameKey = fileName;
	if( !m_caseSensitive )
		for (size_t i = 0; i < fileNameKey.size(); ++i )
			fileNameKey[i] = _ttolower( fileNameKey[i] );

	NameToEntry_t::const_iterator keyValue = m_nameToEntry.find( fileNameKey );
	if( keyValue == m_nameToEntry.end() )
	{
		m_errorMessage << "file " << fileNameKey << " not found in the zip index" << std::endl;
		return false;
	}

	ZipEntry const& zipEntry = keyValue->second;
	unz_file_pos pos;
	pos.pos_in_zip_directory = zipEntry.pos_in_zip_directory;
	pos.num_of_file = zipEntry.num_of_file;

	int err = unzGoToFilePos( uf, &pos );
	if( err != UNZ_OK )
	{
		m_errorMessage << "Can't go to file " << fileName << std::endl;
		return false;
	}

	return ReadCurrentFile( fileName, pMemoryBlock, size );
}

bool zip::ZipArchiveInput::ReadCurrentFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size )
{
	int err = UNZ_OK;

#ifdef SCARAB_WCHAR_MODE
	std::string filename_inzip = utf_convert::as_utf8( fileName );
#else
	std::string filename_inzip = fileName;
#endif

	unz_file_info64 file_info;
	uLong ratio=0;
	err = unzGetCurrentFileInfo64(uf,&file_info,(char*)filename_inzip.c_str(),filename_inzip.size()+1,NULL,0,NULL,0);
	if ( err != UNZ_OK )
	{
		m_errorMessage << "unzGetCurrentFileInfo error in file " << fileName << " error code: " << err << std::endl;
		return false;
	}

	size_t size_buf = (size_t)file_info.uncompressed_size;
	void* buf = malloc(size_buf);
	if (buf == NULL)
	{
		m_errorMessage << "Error allocating memory for file " << fileName << " requested: " << size_buf << std::endl;
		return false;
	}

	err = unzOpenCurrentFilePassword(uf,password);
	if (err != UNZ_OK)
	{
		m_errorMessage << "unzOpenCurrentFilePassword error in file " << fileName << " error code: " << err << std::endl;
		return false;
	}

	int numRead = unzReadCurrentFile(uf,buf,size_buf);
	if( numRead < 0 )
	{
		m_errorMessage << "unzReadCurrentFile error in file " << fileName << " error code: " << err << std::endl;
		err = numRead;
	}
	else
		err = UNZ_OK;

	if (err==UNZ_OK)
	{
		err = unzCloseCurrentFile (uf);
		if (err!=UNZ_OK)
			m_errorMessage << "unzCloseCurrentFile error in file " << fileName << " error code: " << err << std::endl;
	}
	else
		unzCloseCurrentFile(uf); /* don't lose the error */

	if( err != UNZ_OK )
	{
		free(buf);
		return false;
	}

	pMemoryBlock = (Byte_t*)buf;
	size = size_buf;
	return true;
}

bool zip::ZipArchiveInput::Close()
{
	int errClose = unzClose(uf);
	m_nameToEntry.clear();
	return errClose == UNZ_OK;
}

bool zip::ZipArchiveInput::Index()
{
	static const int UNZ_MAXFILENAMEINZIP = 256;

	int err = unzGoToFirstFile(uf);
	if( err != UNZ_OK )
	{
		m_errorMessage << "Can't go to first file" << std::endl;
		return false;
	}

	while (err == UNZ_OK)
	{
		char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
		err = unzGetCurrentFileInfo64(uf, NULL,	szCurrentFileName, sizeof(szCurrentFileName)-1,NULL,0,NULL,0);
		if(err == UNZ_OK)
		{
#ifdef SCARAB_WCHAR_MODE
			String_t fileNameKey = utf_convert::as_wide( szCurrentFileName );
#else
			String_t fileNameKey = szCurrentFileName;
#endif
			if( !m_caseSensitive )
				for (size_t i = 0; i < fileNameKey.size(); ++i )
					fileNameKey[i] = _ttolower( fileNameKey[i] );

			unz_file_pos pos;
			err = unzGetFilePos( uf, &pos );
			if( err != UNZ_OK )
			{
				m_errorMessage << "Can't get file position for " << fileNameKey << std::endl;
				return false;
			}

			ZipEntry zipEntry;
			zipEntry.pos_in_zip_directory = pos.pos_in_zip_directory;
			zipEntry.num_of_file = pos.num_of_file;
			m_nameToEntry.insert( std::make_pair( fileNameKey, zipEntry ) );

			err = unzGoToNextFile(uf);
			if( err != UNZ_OK && err != UNZ_END_OF_LIST_OF_FILE )
			{
				m_errorMessage << "Can't go to next file" << std::endl;
				return false;
			}
		}
		else
		{
			m_errorMessage << "Can't get file info" << std::endl;
			return false;
		}
	}

	return err == UNZ_END_OF_LIST_OF_FILE;
}

_tstring zip::ZipArchiveInput::ErrorMessage() const
{
	return m_errorMessage.str();
}


