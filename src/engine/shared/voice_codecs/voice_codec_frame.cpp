//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "voice_codec_frame.h"


VoiceCodec_Frame::VoiceCodec_Frame(IFrameEncoder* pEncoder)
{
	m_nEncodeBufferSamples = 0;
	m_pFrameEncoder = pEncoder;	
	m_nRawBytes = 0;
	m_nRawSamples = 0;
	m_nEncodedBytes = 0;
}

VoiceCodec_Frame::~VoiceCodec_Frame()
{
	if (m_pFrameEncoder)
		m_pFrameEncoder->Release();
}

bool VoiceCodec_Frame::Init(uint32_t nQuality, uint32_t nSampleRate)
{
	if (m_pFrameEncoder && m_pFrameEncoder->Init(nQuality, m_nRawBytes, m_nEncodedBytes))
	{
		m_nRawSamples = m_nRawBytes >> 1;
		return true;
	}
	else
	{
		if (m_pFrameEncoder)
			m_pFrameEncoder->Release();

		m_pFrameEncoder = nullptr;
		return false;
	}
}

void VoiceCodec_Frame::Release()
{
	delete this;
}

#define BOUNDS_CHECK(memory, access_offset, index) \
    Assert(&memory[index] + access_offset < &memory[sizeof(memory) / sizeof(*memory)]);

int VoiceCodec_Frame::Compress(const char* pUncompressedBytes, int nSamples, char* pCompressed, int maxCompressedBytes)
{
	int nCompressedBytes = 0;
	uint16_t samples[MAX_FRAMEBUFFER_SAMPLES];

	const uint16_t* pUncompressed = reinterpret_cast<const uint16_t*>(pUncompressedBytes);

	if (!m_pFrameEncoder)
		return 0;

	if (m_nEncodeBufferSamples + nSamples >= m_nRawSamples)
	{
		do
		{
			if ((maxCompressedBytes - nCompressedBytes) < m_nEncodedBytes)
				break;

			BOUNDS_CHECK(samples, 0, m_nEncodeBufferSamples * BYTES_PER_SAMPLE);
			memcpy(samples, m_EncodeBuffer, m_nEncodeBufferSamples * BYTES_PER_SAMPLE);

			BOUNDS_CHECK(samples, m_nEncodeBufferSamples, (m_nRawSamples - m_nEncodeBufferSamples) * BYTES_PER_SAMPLE);
			memcpy(&samples[m_nEncodeBufferSamples], pUncompressed, (m_nRawSamples - m_nEncodeBufferSamples) * BYTES_PER_SAMPLE);

			nSamples += m_nEncodeBufferSamples - m_nRawSamples;
			pUncompressed += m_nRawSamples - m_nEncodeBufferSamples;

			m_nEncodeBufferSamples = 0;

			if (m_pFrameEncoder->EncodeFrame(reinterpret_cast<const char*>(samples), &pCompressed[nCompressedBytes]))
				nCompressedBytes += m_nEncodedBytes;

		} while (m_nEncodeBufferSamples + nSamples >= m_nRawSamples);
	}

	int nNewSamples = min(nSamples, min(m_nRawSamples - m_nEncodeBufferSamples, m_nRawSamples));

	if (!nNewSamples)
		return nCompressedBytes;

	BOUNDS_CHECK(m_EncodeBuffer, m_nEncodeBufferSamples, nNewSamples * BYTES_PER_SAMPLE);
	memcpy(&m_EncodeBuffer[m_nEncodeBufferSamples], &pUncompressed[nSamples - nNewSamples], nNewSamples * BYTES_PER_SAMPLE);
	m_nEncodeBufferSamples += nNewSamples;
	return nCompressedBytes;
}


int VoiceCodec_Frame::Decompress(const char* pCompressed, int nCompressedBytes, char* pDecompressed, int nMaxDecompressedBytes)
{
	Assert((nCompressedBytes % m_nEncodedBytes) == 0);
	
	int nDecompressedBytes = 0;
	int curCompressedByte = 0;

	if (!m_pFrameEncoder || nCompressedBytes < m_nEncodedBytes)
		return 0;

	do
	{
		if (nMaxDecompressedBytes - nDecompressedBytes < m_nRawBytes)
			break;

		m_pFrameEncoder->DecodeFrame(&pCompressed[curCompressedByte], &pDecompressed[nDecompressedBytes]);
		curCompressedByte += m_nEncodedBytes;
		nDecompressedBytes += m_nRawBytes;

	} while (nCompressedBytes - curCompressedByte >= m_nEncodedBytes);

	return nDecompressedBytes / 2;
}

#undef BOUNDS_CHECK


bool VoiceCodec_Frame::ResetState()
{
	if (m_pFrameEncoder)
		return m_pFrameEncoder->ResetState();
	else
		return false;
}

VoiceCodec_Frame* CreateVoiceCodec_Frame(IFrameEncoder* pEncoder)
{
	return new VoiceCodec_Frame(pEncoder);
}