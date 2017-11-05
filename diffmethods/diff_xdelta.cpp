#include "diff_xdelta.h"

#if SCARAB_XDELTA

#include <dung/dung.h>

#pragma comment(linker, "/defaultlib:xdelta.lib")
#pragma message("Automatically linking with xdelta.lib")

extern "C"
{
#include <external/xdelta/xdelta3.h>
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
	const size_t reservedSize = max( newSize, 1024 );

	diffBlock = SCARAB_NEW dung::Byte_t[ reservedSize ];
	uint32_t diffSize32 = 0;

	int flags = MakeFlags( m_config );
	m_errorCode = xd3_encode_memory( newBlock, newSize, oldBlock, oldSize, diffBlock, &diffSize32, reservedSize, flags );

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

	m_errorCode = xd3_decode_memory( diffBlock, diffSize, oldBlock, oldSize, newBlock, &newSize32, reservedSize, 0 );

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

#endif // SCARAB_XDELTA
