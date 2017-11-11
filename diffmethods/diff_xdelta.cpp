#include "diff_xdelta.h"

#if SCARAB_XDELTA

#include <dung/dung.h>

#pragma comment(linker, "/defaultlib:xdelta.lib")
#pragma message("Automatically linking with xdelta.lib")

extern "C"
{
// xdelta.lib must be compiled with the same logic. See Scarab/xdelta/xdelta.vcxproj
// for SIZEOF_SIZE_T macro definition while compiling xdelta3.c
#if defined(_M_IA64) || defined(_M_X64)
#    define SIZEOF_SIZE_T 8
#elif defined(_M_IX86)
#    define SIZEOF_SIZE_T 4
#else
#    error "Unsupported MSVC platform"
#endif

#include <external/xdelta/xdelta3/xdelta3.h>
#undef NEW
#undef DELETE
#undef MOVE
}

namespace xdelta
{

const int FILE_BUFFER_SIZE = 64 * XD3_ALLOCSIZE; // 1 mb

int code(bool encode, FILE* inFile, FILE* srcFile, FILE* outFile, int flags, int bufSize, _tstring& outError)
{
	int r, ret;
	struct stat statbuf;
	xd3_stream stream;
	xd3_config config;
	xd3_source source;
	void* inputBuf;
	int inputBufRead;

	if (bufSize < XD3_ALLOCSIZE)
		bufSize = XD3_ALLOCSIZE;

	memset(&stream, 0, sizeof(stream));
	memset(&source, 0, sizeof(source));

	xd3_init_config(&config, flags);
	config.winsize = bufSize;
	xd3_config_stream(&stream, &config);

	if (srcFile)
	{
		r = fstat(_fileno(srcFile), &statbuf);
		if (r)
			return r;

		source.blksize = bufSize;
		source.curblk = (const uint8_t*)malloc(source.blksize);

		/* Load 1st block of stream. */
		r = fseek(srcFile, 0, SEEK_SET);
		if (r)
			return r;
		source.onblk = (usize_t)fread((void*)source.curblk, 1, source.blksize, srcFile);
		source.curblkno = 0;
		/* Set the stream. */
		xd3_set_source(&stream, &source);
	}

	inputBuf = malloc(bufSize);

	fseek(inFile, 0, SEEK_SET);
	do
	{
		inputBufRead = (int)fread(inputBuf, 1, bufSize, inFile);
		if (inputBufRead < bufSize)
			xd3_set_flags(&stream, XD3_FLUSH | stream.flags);
		xd3_avail_input(&stream, (const uint8_t*)inputBuf, inputBufRead);

	process:
		if (encode)
			ret = xd3_encode_input(&stream);
		else
			ret = xd3_decode_input(&stream);

		switch (ret)
		{
			case XD3_INPUT:
				continue;

			case XD3_OUTPUT:
			{
				r = (int)fwrite(stream.next_out, 1, stream.avail_out, outFile);
				if (r != (int)stream.avail_out)
					return r;
				xd3_consume_output(&stream);
				goto process;
			}

			case XD3_GETSRCBLK:
			{
				if (srcFile)
				{
					r = fseek(srcFile, source.blksize * (usize_t)source.getblkno, SEEK_SET);
					if (r)
						return r;
					source.onblk = (usize_t)fread((void*)source.curblk, 1, source.blksize, srcFile);
					source.curblkno = source.getblkno;
				}
				goto process;
			}

			case XD3_GOTHEADER:
				goto process;

			case XD3_WINSTART:
				goto process;

			case XD3_WINFINISH:
				goto process;

			default:
			{
#ifdef SCARAB_WCHAR_MODE
				outError = utf_convert::as_wide(stream.msg);
#else
				outError = stream.msg;
#endif
				return ret;
			}
		}
	} while (inputBufRead == bufSize);

	free(inputBuf);

	free((void*)source.curblk);
	xd3_close_stream(&stream);
	xd3_free_stream(&stream);

	return 0;
};

FILE* openFile(const char* fileName, bool read)
{
	return fopen(fileName, read ? "rb" : "wb");
}

FILE* openFile(const wchar_t* fileName, bool read)
{
	return _wfopen(fileName, read ? L"rb" : L"wb");
}

} // namespace xdelta


xdelta::XdeltaEncoder::XdeltaEncoder(Config const& config)
	: m_config(config)
	, m_errorCode()
{
}

xdelta::XdeltaEncoder::~XdeltaEncoder()
{
}

bool xdelta::XdeltaEncoder::EncodeDiffFile(const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName)
{
	int flags = MakeFlags(m_config);

	FILE* inFile = openFile(newFileName, true);
	FILE* srcFile = openFile(oldFileName, true);
	FILE* outFile = openFile(diffFileName, false);

	m_errorCode = xdelta::code(true, inFile, srcFile, outFile, flags, FILE_BUFFER_SIZE, m_errorMessage);

	fclose(outFile);
	fclose(srcFile);
	fclose(inFile);

	return m_errorCode == 0;
}

void xdelta::XdeltaEncoder::GetErrorMessage(_tstring& errorMessage) const
{
	if (m_errorCode == 0)
		return;

	errorMessage = _T("Can't encode memory with xdelta. Error message: ");
	errorMessage += m_errorMessage;
	errorMessage = _T(" Error code: ");
	errorMessage += m_errorCode;
}

int xdelta::XdeltaEncoder::MakeFlags(Config const& config)
{
	int flags = 0;

	if (config.DJW) flags |= XD3_SEC_DJW;
	if (config.FGK) flags |= XD3_SEC_FGK;
	if (config.LZMA) flags |= XD3_SEC_LZMA;

	if (config.nodata) flags |= XD3_SEC_NODATA;
	if (config.noinst) flags |= XD3_SEC_NOINST;
	if (config.noaddr) flags |= XD3_SEC_NOADDR;

	if (config.adler32) flags |= XD3_ADLER32;
	if (config.adler32_nover) flags |= XD3_ADLER32_NOVER;

	if (config.beGreedy) flags |= XD3_BEGREEDY;

	if (config.compression != 0) flags |= ((config.compression << XD3_COMPLEVEL_SHIFT) & XD3_COMPLEVEL_MASK);

	return flags;
}


xdelta::XdeltaDecoder::XdeltaDecoder()
	: m_errorCode()
{
}

xdelta::XdeltaDecoder::~XdeltaDecoder()
{
}

bool xdelta::XdeltaDecoder::DecodeDiffFile(const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName)
{
	FILE* inFile = openFile(diffFileName, true);
	FILE* srcFile = openFile(oldFileName, true);
	FILE* outFile = openFile(newFileName, false);

	m_errorCode = xdelta::code(false, inFile, srcFile, outFile, XD3_ADLER32, FILE_BUFFER_SIZE, m_errorMessage);

	fclose(outFile);
	fclose(srcFile);
	fclose(inFile);

	return m_errorCode == 0;
}

void xdelta::XdeltaDecoder::GetErrorMessage(_tstring& errorMessage) const
{
	if (m_errorCode == 0)
		return;

	errorMessage = _T("Can't decode memory with xdelta. Error message: ");
	errorMessage += m_errorMessage;
	errorMessage = _T(" Error code: ");
	errorMessage += m_errorCode;
}

#endif // SCARAB_XDELTA
