//	Implementation of the US Secure Hash Algorithm 1 (SHA1)
//	Taken from the Request for Comments No. 3174
#pragma once

#include "dung.h"

#include <stdint.h>

/*
 *  sha1.h
 *
 *  Description:
 *      This is the header file for code which implements the Secure
 *      Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
 *      April 17, 1995.
 *
 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *
 *      Please read the file sha1.cpp for more information.
 *
 */

///	sha Error Codes
enum SHA_enum
{
    shaSuccess = 0,			///< done OK
    shaNull,				///< Null pointer parameter
    shaInputTooLong,		///< input data too long
    shaStateError			///< called Input after Result
};

/// SHA hash size
static const int SHA1HashSize = 20;

///	This structure will hold context information for the SHA-1 hashing operation
typedef struct SHA1Context
{
	uint32_t Intermediate_Hash[SHA1HashSize/4];	///< Message Digest

	uint32_t Length_Low;						///< Message length in bits
	uint32_t Length_High;           			///< Message length in bits

	int_least16_t Message_Block_Index;			///< Index into message block array
	uint8_t Message_Block[64];					///< 512-bit message blocks

	int Computed;								///< Is the digest computed?
	int Corrupted;								///< Is the message digest corrupted?
}
SHA1Context_t;

///	Description:
///		This function will initialize the SHA1Context in preparation
///		for computing a new SHA1 message digest.
///
///	Parameters:
///		context: [in/out]
///			The context to reset.
///
///	Returns:
///		sha Error Code.
int SHA1Reset( SHA1Context * );

///	Description:
///		This function accepts an array of octets as the next portion of the message.
///
///	Parameters:
///		context: [in/out]
///			The SHA context to update
///		message_array: [in]
///			An array of characters representing the next portion of the message.
///		length: [in]
///			The length of the message in message_array
///
///	Returns:
///		sha Error Code.
int SHA1Input( SHA1Context *, const uint8_t *, unsigned );

///	Description:
///		This function will return the 160-bit message digest into the
///		Message_Digest array  provided by the caller.
///		NOTE: The first octet of hash is stored in the 0th element,
///		      the last octet of hash in the 19th element.
///
///	Parameters:
///		context: [in/out]
///			The context to use to calculate the SHA-1 hash.
///		Message_Digest: [out]
///			Where the digest is returned.
///
///	Returns:
///		sha Error Code.
int SHA1Result( SHA1Context *, uint8_t Message_Digest[SHA1HashSize] );

/// Helpers
static const int SHA1StringSize = SHA1HashSize * 2 + 1;

struct Sha1
{
	Sha1();
	unsigned char digest[SHA1HashSize];
};

int SHA1Compute( const void * pMemoryBlock, size_t size, Sha1& sha1 );
void SHA1ToString( Sha1 const& sha1, _TCHAR output[SHA1StringSize] );
