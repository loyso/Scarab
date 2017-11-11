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

	class XdeltaEncoder : public dung::DiffEncoderExternal_i
	{
	public:
		XdeltaEncoder(Config const& config);
		~XdeltaEncoder();

	private:
		static int MakeFlags(Config const& config);
		virtual bool EncodeDiffFile(const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName);
		virtual void GetErrorMessage(_tstring& errorMessage) const;

		Config m_config;
		_tstring m_errorMessage;
		int m_errorCode;
	};


	class XdeltaDecoder : public dung::DiffDecoderExternal_i
	{
	public:
		XdeltaDecoder();
		~XdeltaDecoder();

	private:
		virtual bool DecodeDiffFile(const _TCHAR* newFileName, const _TCHAR* oldFileName, const _TCHAR* diffFileName);
		virtual void GetErrorMessage(_tstring& errorMessage) const;

		_tstring m_errorMessage;
		int m_errorCode;
	};
}

#endif // SCARAB_XDELTA
