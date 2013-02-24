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

#define CASESENSITIVITY (0)
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


int zip::ZipCreateDirectory( const char* dirname )
{
    int ret=0;
#ifdef WIN32
    ret = _mkdir(dirname);
#else
    ret = mkdir (dirname,0775);
#endif
    return ret;
}

bool zip::ZipCreateDirectories( const char *newdir )
{
	char *buffer ;
	char *p;
	int  len = (int)strlen(newdir);

	if (len <= 0)
		return false;

	buffer = (char*)malloc(len+1);
	if (buffer==NULL)
	{
		printf("Error allocating memory\n");
		return false;
	}
	strcpy(buffer,newdir);

	if (buffer[len-1] == '/') {
		buffer[len-1] = '\0';
	}

	if (ZipCreateDirectory(buffer) == 0)
	{
		free(buffer);
		return true;
	}

	p = buffer+1;
	while (1)
	{
		char hold;

		while(*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if ((ZipCreateDirectory(buffer) == -1) && (errno == ENOENT))
		{
			printf("couldn't create directory %s\n",buffer);
			free(buffer);
			return false;
		}
		if (hold == 0)
			break;
		*p++ = hold;
	}
	free(buffer);
	return true;
}

zip::ZipArchiveInput::ZipArchiveInput()
	: m_archiveName()
	, uf()
	, password()
{
}

bool zip::ZipArchiveInput::Open( String_t const& archiveName )
{
	m_archiveName = archiveName;
#ifdef USEWIN32IOAPI
	zlib_filefunc64_def ffunc;
#endif

#ifdef USEWIN32IOAPI
	fill_win32_filefunc64A(&ffunc);
	uf = unzOpen2_64(m_archiveName.c_str(),&ffunc);
#else
	uf = unzOpen64(m_archiveName.c_str());
#endif

	if (uf==NULL)
	{
		printf("Cannot open %s\n",m_archiveName.c_str());
		return false;
	}

	printf("%s opened\n",m_archiveName.c_str());

	return Index();
}

bool zip::ZipArchiveInput::LocateAndReadFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size )
{
	int err = UNZ_OK;
	if (unzLocateFile(uf,fileName.c_str(),CASESENSITIVITY)!=UNZ_OK)
	{
		printf("file %s not found in the zipfile\n",fileName.c_str());
		return false;
	}

	return ReadCurrentFile( fileName, pMemoryBlock, size );
}

bool zip::ZipArchiveInput::ReadFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size )
{
	NameToEntry_t::const_iterator keyValue = m_nameToEntry.find( fileName );
	if( keyValue == m_nameToEntry.end() )
		return false;

	ZipEntry const& zipEntry = keyValue->second;
	unz_file_pos pos;
	pos.pos_in_zip_directory = zipEntry.pos_in_zip_directory;
	pos.num_of_file = zipEntry.num_of_file;

	int err = unzGoToFilePos( uf, &pos );
	if( err != UNZ_OK )
	{
		printf("file %s not found in the index\n", fileName.c_str());
		return false;
	}

	return ReadCurrentFile( fileName, pMemoryBlock, size );
}

bool zip::ZipArchiveInput::ReadCurrentFile( String_t const& fileName, Byte_t*& pMemoryBlock, size_t& size )
{
	int err = UNZ_OK;

	char filename_inzip[256];
	strncpy( filename_inzip, fileName.c_str(), sizeof(filename_inzip) );

	unz_file_info64 file_info;
	uLong ratio=0;
	err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

	if (err!=UNZ_OK)
	{
		printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
		return false;
	}

	size_t size_buf = (size_t)file_info.uncompressed_size;
	void* buf = (void*)malloc(size_buf);
	if (buf==NULL)
	{
		printf("Error allocating memory\n");
		return false;
	}

	err = unzOpenCurrentFilePassword(uf,password);
	if (err!=UNZ_OK)
	{
		printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
	}

	printf(" extracting: %s\n", fileName.c_str());

	int numRead = unzReadCurrentFile(uf,buf,size_buf);
	if (numRead<0)
	{
		printf("error %d with zipfile in unzReadCurrentFile\n",err);
		err = numRead;
	}
	else
		err = UNZ_OK;

	if (err==UNZ_OK)
	{
		err = unzCloseCurrentFile (uf);
		if (err!=UNZ_OK)
		{
			printf("error %d with zipfile in unzCloseCurrentFile\n",err);
		}
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

void zip::ZipArchiveInput::Close()
{
	unzClose(uf);
}

bool zip::ZipArchiveInput::Index()
{
	static const int UNZ_MAXFILENAMEINZIP = 256;

	int err = unzGoToFirstFile(uf);

	while (err == UNZ_OK)
	{
		char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
		err = unzGetCurrentFileInfo64(uf, NULL,	szCurrentFileName, sizeof(szCurrentFileName)-1,NULL,0,NULL,0);
		if(err == UNZ_OK)
		{
			unz_file_pos pos;
			err = unzGetFilePos( uf, &pos );
			if( err != UNZ_OK )
			{
				return false;
			}

			ZipEntry zipEntry;
			zipEntry.pos_in_zip_directory = pos.pos_in_zip_directory;
			zipEntry.num_of_file = pos.num_of_file;
			m_nameToEntry.insert( std::make_pair( szCurrentFileName, zipEntry ) );

			err = unzGoToNextFile(uf);
		}
	}

	return err == UNZ_END_OF_LIST_OF_FILE;
}


