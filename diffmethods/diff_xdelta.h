#pragma once

#include "diffmethods.h"

#ifndef SCARAB_XDELTA
#define SCARAB_XDELTA 0 // disabled by default
#endif

#if SCARAB_XDELTA

#include <dung/diffencoder.h>
#include <dung/diffdecoder.h>

namespace xdelta
{
	struct Config
	{
		int compression;

		bool DJW;
		bool FGK;
		bool LZMA;

		bool nodata;
		bool noinst;
		bool noaddr;

		bool adler32;
		bool adler32_nover;

		bool beGreedy; 
	};

	class XdeltaEncoder : public dung::DiffEncoder_i
	{
	public:
		XdeltaEncoder( Config const& config );
		~XdeltaEncoder();

	private:
		static int MakeFlags( Config const& config );
		virtual bool EncodeDiffMemoryBlock( const dung::Byte_t* newBlock, size_t newSize, const dung::Byte_t* oldBlock, size_t oldSize, dung::Byte_t*& diffBlock, size_t& diffSize );
		virtual void GetErrorMessage( _tstring& errorMessage ) const;

		Config m_config;
		int m_errorCode;
	};

	class XdeltaEncoderExternal : public dung::DiffEncoderExternal_i
	{
	public:
		XdeltaEncoderExternal(Config const& config);
		~XdeltaEncoderExternal();

	private:
		static int MakeFlags(Config const& config);
		virtual bool EncodeDiffFile(const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName);
		virtual void GetErrorMessage(_tstring& errorMessage) const;

		Config m_config;
		int m_errorCode;
	};


	class XdeltaDecoder : public dung::DiffDecoder_i
	{
	public:
		XdeltaDecoder();
		~XdeltaDecoder();

	private:
		virtual bool DecodeDiffMemoryBlock( const dung::Byte_t* oldBlock, size_t oldSize, const dung::Byte_t* diffBlock, size_t diffSize, dung::Byte_t*& newBlock, size_t& newSize );
		virtual void GetErrorMessage( _tstring& errorMessage ) const;

		int m_errorCode;
	};

	class XdeltaDecoderExternal : public dung::DiffDecoderExternal_i
	{
	public:
		XdeltaDecoderExternal();
		~XdeltaDecoderExternal();

	private:
		virtual bool DecodeDiffFile(const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName);
		virtual void GetErrorMessage(_tstring& errorMessage) const;

		int m_errorCode;
	};
}

#endif // SCARAB_XDELTA
