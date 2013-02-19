// Based on a sample part of the MiniZip project.

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

// to encrypt a file, we need known the CRC32 of the file before
int getFileCrc(const char* filenameinzip,void*buf,unsigned long size_buf,unsigned long* result_crc)
{
   unsigned long calculate_crc=0;
   int err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   unsigned long total_read = 0;
   if (fin==NULL)
   {
       err = ZIP_ERRNO;
   }

    if (err == ZIP_OK)
        do
        {
            err = ZIP_OK;
            size_read = (int)fread(buf,1,size_buf,fin);
            if (size_read < size_buf)
                if (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            if (size_read>0)
                calculate_crc = crc32(calculate_crc,(Bytef*)buf,size_read);
            total_read += size_read;

        } while ((err == ZIP_OK) && (size_read>0));

    if (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    return err;
}

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

int ZipFiles( int argc, char *argv[] )
{
    int opt_compress_level=Z_DEFAULT_COMPRESSION;
    char zipfilename[MAXFILENAME+16];
    int err=0;
    int size_buf=0;
    void* buf=NULL;
    const char* password=NULL;

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
        return ZIP_INTERNALERROR;

    zipFile zf;
    int errclose;
#ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    zf = zipOpen2_64(zipfilename,false,NULL,&ffunc);
#else
    zf = zipOpen64(zipfilename,false);
#endif

    if (zf == NULL)
        err= ZIP_ERRNO;

    for( int i=0; i<argc && err==ZIP_OK; i++ )
    {
        FILE * fin;
        int size_read;
        const char* filenameinzip = argv[i];
        const char *savefilenameinzip;
        zip_fileinfo zi;
        unsigned long crcFile=0;
        int zip64 = 0;

        zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
        zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
        zi.dosDate = 0;
        zi.internal_fa = 0;
        zi.external_fa = 0;
        filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);
        if ((password != NULL) && (err==ZIP_OK))
            err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

        zip64 = isLargeFile(filenameinzip);

        /* The path name saved, should not include a leading slash. */
        /*if it did, windows/xp and dynazip couldn't read the zip file. */
        savefilenameinzip = filenameinzip;
        while( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
        {
            savefilenameinzip++;
        }

        err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                            NULL,0,NULL,0,NULL /* comment*/,
                            (opt_compress_level != 0) ? Z_DEFLATED : 0,
                            opt_compress_level,0,
                            -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                            password,crcFile, zip64);

        if (err != ZIP_OK)
            printf("error in opening %s in zipfile\n",filenameinzip);
        else
        {
            fin = FOPEN_FUNC(filenameinzip,"rb");
            if (fin==NULL)
            {
                err=ZIP_ERRNO;
                printf("error in opening %s for reading\n",filenameinzip);
            }
        }

        if (err == ZIP_OK)
            do
            {
                err = ZIP_OK;
                size_read = (int)fread(buf,1,size_buf,fin);
                if (size_read < size_buf)
                    if (feof(fin)==0)
                {
                    printf("error in reading %s\n",filenameinzip);
                    err = ZIP_ERRNO;
                }

                if (size_read>0)
                {
                    err = zipWriteInFileInZip (zf,buf,size_read);
                    if (err<0)
                        printf("error in writing %s in the zipfile\n", filenameinzip);
                }
            } 
			while ((err == ZIP_OK) && (size_read>0));

        if (fin)
            fclose(fin);

        if (err<0)
            err=ZIP_ERRNO;
        else
        {
            err = zipCloseFileInZip(zf);
            if (err!=ZIP_OK)
                printf("error in closing %s in the zipfile\n", filenameinzip);
        }
    }
    errclose = zipClose(zf,NULL);
    if (errclose != ZIP_OK)
        printf("error in closing %s\n",zipfilename);

    free(buf);
    return 0;
}
