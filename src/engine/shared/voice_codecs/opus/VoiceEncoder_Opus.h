#ifndef VOICEENCODER_OPUS_H
#define VOICEENCODER_OPUS_H

#include "iframeencoder.h"
#include "thirdparty/opus/include/opus.h"
#include "speex/include/speex_resampler.h"

#define OPUS_VOICE_ENCODED_SAMPLERATE 24000
#define OPUS_VOICE_FRAME_DURATION 10.f

class VoiceEncoder_Opus : public IFrameEncoder
{
public:
	virtual ~VoiceEncoder_Opus() { Release(); };

	virtual bool	Init(int quality, int& rawFrameSize, int& encodedFrameSize);

	virtual void	Release();

	// pCompressed is the size of encodedFrameSize.
	virtual bool    EncodeFrame(const char* pUncompressed, char* pCompressed);

	// pCompressed is the size of encodedFrameSize.
	virtual void	DecodeFrame(const char* pCompressed, char* pDecompressed);

	// Some codecs maintain state between Compress and Decompress calls. This should clear that state.
	virtual bool	ResetState();

private:
	int m_EncodedFrameSize = 0;
	int m_RawFrameSizeSamples = 0;

	OpusDecoder* m_pDecoder = nullptr;
	OpusEncoder* m_pEncoder = nullptr;


	SpeexResamplerState* m_pEncodeResampler = nullptr;
	SpeexResamplerState* m_pDecodeResampler = nullptr;

};

class VoiceCodec_Frame* CreateOpusVoiceCodec();

#endif
