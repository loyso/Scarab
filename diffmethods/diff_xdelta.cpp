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
}

xdelta::XdeltaEncoder::XdeltaEncoder( Config const& config )
	: m_config( config )
	, m_errorCode()
{
}

xdelta::XdeltaEncoder::~XdeltaEncoder()
{
}

bool xdelta::XdeltaEncoder::EncodeDiffMemoryBlock( const dung::Byte_t* newBlock, size_t newSize, const dung::Byte_t* oldBlock, size_t oldSize, dung::Byte_t*& diffBlock, size_t& diffSize )
{
	const size_t reservedSize = dung::max( newSize, size_t(1024) );

	diffBlock = SCARAB_NEW dung::Byte_t[ reservedSize ];
	uint32_t diffSize32 = 0;

	int flags = MakeFlags( m_config );
	m_errorCode = xd3_encode_memory( newBlock, (usize_t)newSize, oldBlock, (usize_t)oldSize, diffBlock, &diffSize32, (usize_t)reservedSize, flags );

	diffSize = diffSize32;
	return m_errorCode == 0;
}

void xdelta::XdeltaEncoder::GetErrorMessage( _tstring& errorMessage ) const
{
	if( m_errorCode == 0 )
		return;

	errorMessage = _T("Can't encode memory with xdelta. Error code: ");
	errorMessage += m_errorCode;
}

int xdelta::XdeltaEncoder::MakeFlags( Config const& config )
{
	int flags = 0;

	if( config.DJW ) flags |= XD3_SEC_DJW;
	if( config.FGK ) flags |= XD3_SEC_FGK;
	if( config.LZMA ) flags |= XD3_SEC_LZMA;

	if( config.nodata ) flags |= XD3_SEC_NODATA;
	if( config.noinst ) flags |= XD3_SEC_NOINST;
	if( config.noaddr ) flags |= XD3_SEC_NOADDR;

	if( config.adler32 ) flags |= XD3_ADLER32;
	if( config.adler32_nover ) flags |= XD3_ADLER32_NOVER;

	if( config.beGreedy ) flags |= XD3_BEGREEDY;

	if( config.compression != 0 ) flags |= ( ( config.compression << XD3_COMPLEVEL_SHIFT ) & XD3_COMPLEVEL_MASK );

	return flags;
}

xdelta::XdeltaEncoderExternal::XdeltaEncoderExternal(Config const& config)
	: m_config(config)
	, m_errorCode()
{
}

xdelta::XdeltaEncoderExternal::~XdeltaEncoderExternal()
{
}

int code(bool encode, FILE*  InFile, FILE*  SrcFile, FILE* OutFile, int BufSize)
{
	int r, ret;
	struct stat statbuf;
	xd3_stream stream;
	xd3_config config;
	xd3_source source;
	void* Input_Buf;
	int Input_Buf_Read;

	if (BufSize < XD3_ALLOCSIZE)
		BufSize = XD3_ALLOCSIZE;

	memset(&stream, 0, sizeof(stream));
	memset(&source, 0, sizeof(source));

	xd3_init_config(&config, XD3_ADLER32);
	config.winsize = BufSize;
	xd3_config_stream(&stream, &config);

	if (SrcFile)
	{
		r = fstat(_fileno(SrcFile), &statbuf);
		if (r)
			return r;

		source.blksize = BufSize;
		source.curblk = (const uint8_t*)malloc(source.blksize);

		/* Load 1st block of stream. */
		r = fseek(SrcFile, 0, SEEK_SET);
		if (r)
			return r;
		source.onblk = (usize_t)fread((void*)source.curblk, 1, source.blksize, SrcFile);
		source.curblkno = 0;
		/* Set the stream. */
		xd3_set_source(&stream, &source);
	}

	Input_Buf = malloc(BufSize);

	fseek(InFile, 0, SEEK_SET);
	do
	{
		Input_Buf_Read = (int)fread(Input_Buf, 1, BufSize, InFile);
		if (Input_Buf_Read < BufSize)
		{
			xd3_set_flags(&stream, XD3_FLUSH | stream.flags);
		}
		xd3_avail_input(&stream, (const uint8_t*)Input_Buf, Input_Buf_Read);

	process:
		if (encode)
			ret = xd3_encode_input(&stream);
		else
			ret = xd3_decode_input(&stream);

		switch (ret)
		{
		case XD3_INPUT:
		{
			fprintf(stderr, "XD3_INPUT\n");
			continue;
		}

		case XD3_OUTPUT:
		{
			fprintf(stderr, "XD3_OUTPUT\n");
			r = (int)fwrite(stream.next_out, 1, stream.avail_out, OutFile);
			if (r != (int)stream.avail_out)
				return r;
			xd3_consume_output(&stream);
			goto process;
		}

		case XD3_GETSRCBLK:
		{
			fprintf(stderr, "XD3_GETSRCBLK %I64u\n", source.getblkno);
			if (SrcFile)
			{
				r = fseek(SrcFile, source.blksize * (usize_t)source.getblkno, SEEK_SET);
				if (r)
					return r;
				source.onblk = (usize_t)fread((void*)source.curblk, 1, source.blksize, SrcFile);
				source.curblkno = source.getblkno;
			}
			goto process;
		}

		case XD3_GOTHEADER:
		{
			fprintf(stderr, "XD3_GOTHEADER\n");
			goto process;
		}

		case XD3_WINSTART:
		{
			fprintf(stderr, "XD3_WINSTART\n");
			goto process;
		}

		case XD3_WINFINISH:
		{
			fprintf(stderr, "XD3_WINFINISH\n");
			goto process;
		}

		default:
		{
			fprintf(stderr, "!!! INVALID %s %d !!!\n",
				stream.msg, ret);
			return ret;
		}

		}

	} while (Input_Buf_Read == BufSize);

	free(Input_Buf);

	free((void*)source.curblk);
	xd3_close_stream(&stream);
	xd3_free_stream(&stream);

	return 0;
};

bool xdelta::XdeltaEncoderExternal::EncodeDiffFile(const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName)
{
	int flags = MakeFlags(m_config);

	FILE* InFile = _wfopen(newFileName, L"rb");
	FILE* SrcFile = _wfopen(oldFileName, L"rb");
	FILE* OutFile = _wfopen(diffFileName, L"wb");

	m_errorCode = code(true, InFile, SrcFile, OutFile, 0x1000);

	fclose(OutFile);
	fclose(SrcFile);
	fclose(InFile);

	return m_errorCode == 0;
}

void xdelta::XdeltaEncoderExternal::GetErrorMessage(_tstring& errorMessage) const
{
	if (m_errorCode == 0)
		return;

	errorMessage = _T("Can't encode memory with xdelta. Error code: ");
	errorMessage += m_errorCode;
}

int xdelta::XdeltaEncoderExternal::MakeFlags(Config const& config)
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

bool xdelta::XdeltaDecoder::DecodeDiffMemoryBlock( const dung::Byte_t* oldBlock, size_t oldSize, const dung::Byte_t* diffBlock, size_t diffSize, dung::Byte_t*& newBlock, size_t& newSize )
{
	size_t reservedSize = newSize;

	newBlock = SCARAB_NEW dung::Byte_t[ reservedSize ];
	uint32_t newSize32 = 0;

	m_errorCode = xd3_decode_memory( diffBlock, (uint32_t)diffSize, oldBlock, (uint32_t)oldSize, newBlock, &newSize32, (uint32_t)reservedSize, 0 );

	newSize = newSize32;
	SCARAB_ASSERT( reservedSize == newSize );

	return m_errorCode == 0;
}

void xdelta::XdeltaDecoder::GetErrorMessage( _tstring& errorMessage ) const
{
	if( m_errorCode == 0 )
		return;

	errorMessage = _T("Can't decode memory with xdelta. Error code: ");
	errorMessage += m_errorCode;
}


xdelta::XdeltaDecoderExternal::XdeltaDecoderExternal()
	: m_errorCode()
{
}

xdelta::XdeltaDecoderExternal::~XdeltaDecoderExternal()
{
}

bool xdelta::XdeltaDecoderExternal::DecodeDiffFile(const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName)
{
	FILE* InFile = _wfopen(diffFileName, L"rb");
	FILE* SrcFile = _wfopen(oldFileName, L"rb");
	FILE* OutFile = _wfopen(newFileName, L"wb");

	m_errorCode = code(false, InFile, SrcFile, OutFile, 0x1000);

	fclose(OutFile);
	fclose(SrcFile);
	fclose(InFile);

	return m_errorCode == 0;
}

void xdelta::XdeltaDecoderExternal::GetErrorMessage(_tstring& errorMessage) const
{
	if (m_errorCode == 0)
		return;

	errorMessage = _T("Can't decode memory with xdelta. Error code: ");
	errorMessage += m_errorCode;
}

#endif // SCARAB_XDELTA
