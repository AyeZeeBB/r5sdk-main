//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef VOICE_CODEC_FRAME_H
#define VOICE_CODEC_FRAME_H

#include "public/ivoicecodec.h"
#include "public/iframeencoder.h"

#define MAX_FRAMEBUFFER_SAMPLES 1024

class VoiceCodec_Frame : public IVoiceCodec
{
protected:
	virtual			~VoiceCodec_Frame();
public:
	VoiceCodec_Frame(IFrameEncoder* pEncoder);

	virtual bool	Init(uint32_t nQuality, uint32_t nSampleRate);
	virtual void	Release();

	virtual int		Compress(const char* pUncompressed, int nSamples, char* pCompressed, int maxCompressedBytes);
	virtual int		Decompress(const char* pCompressed, int compressedBytes, char* pUncompressed, int maxUncompressedBytes);

	virtual bool	ResetState();

	virtual int		GetNumEncodedBytes() { return m_nEncodedBytes; }

	__int16 m_EncodeBuffer[MAX_FRAMEBUFFER_SAMPLES];
	int m_nEncodeBufferSamples;
	IFrameEncoder* m_pFrameEncoder;
	int m_nRawBytes;
	int m_nRawSamples;
	int m_nEncodedBytes;
};

VoiceCodec_Frame* CreateVoiceCodec_Frame(IFrameEncoder* pEncoder);

#endif // !VOICE_CODEC_FRAME_H
