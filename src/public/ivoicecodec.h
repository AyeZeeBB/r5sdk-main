//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Define the IVoiceCodec interface.
//
// $NoKeywords: $
//=============================================================================//

#ifndef IVOICECODEC_H
#define IVOICECODEC_H

// This interface is for voice codecs to implement.

// Codecs are guaranteed to be called with the exact output from Compress into Decompress (ie:
// data won't be stuck together and sent to Decompress).

// Decompress is not guaranteed to be called in any specific order relative to Compress, but 
// Codecs maintain state between calls, so it is best to call Compress with consecutive voice data
// and decompress likewise. If you call it out of order, it will sound wierd. 

// In the same vein, calling Decompress twice with the same data is a bad idea since the state will be
// expecting the next block of data, not the same block.

#define BYTES_PER_SAMPLE 2

class IVoiceCodec
{
protected:
	virtual			~IVoiceCodec() {}
	
	virtual bool	Init(uint32_t nQuality, uint32_t nSampleRate) = 0;

	// Use this to delete the object.
	virtual void	Release() = 0;

	virtual int		Compress(const char* pUncompressed, int nSamples, char* pCompressed, int maxCompressedBytes) = 0;

	// Decompress voice data. pUncompressed is 16-bit signed mono.
	virtual int		Decompress(const char* pCompressed, int compressedBytes, char* pDecompressed, int maxDecompressedBytes) = 0;

	// Some codecs maintain state between Compress and Decompress calls. This should clear that state.
	virtual bool	ResetState() = 0;

	virtual int		GetNumEncodedBytes() = 0;
};

#endif