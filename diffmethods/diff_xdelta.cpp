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
	diffSize = 0;

	int flags = MakeFlags( m_config );
	m_errorCode = xd3_encode_memory( newBlock, newSize, oldBlock, oldSize, diffBlock, &diffSize, reservedSize, flags );

	return m_errorCode == 0;
}

void xdelta::XdeltaEncoder::GetErrorMessage( char* errorMessage, size_t bufferSize ) const
{
	if( m_errorCode != 0 )
		sprintf( errorMessage, "Can't encode memory with xdelta. Error code: %d", m_errorCode );
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

	m_errorCode = xd3_decode_memory( diffBlock, diffSize, oldBlock, oldSize, newBlock, &newSize, reservedSize, 0 );

	SCARAB_ASSERT( reservedSize == newSize );

	return m_errorCode == 0;
}

void xdelta::XdeltaDecoder::GetErrorMessage( char* errorMessage, size_t bufferSize ) const
{
	if( m_errorCode != 0 )
		sprintf( errorMessage, "Can't decode memory with xdelta. Error code: %d", m_errorCode );
}

#endif // SCARAB_XDELTA
