#include "VoiceEncoder_Opus.h"
#include "engine/voice.h"
#include "engine/shared/voice_codecs/voice_codec_frame.h"

#define OPUS_ERROR_CHECK(ptr, fmtStr, errCode)										\
	if(errCode != OPUS_OK || !ptr)													\
	{																				\
		Assert(0);																	\
		if(errCode != OPUS_OK)														\
			Error(eDLL_T::AUDIO, errCode, fmtStr, opus_strerror(errCode));			\
		else																		\
		    Error(eDLL_T::AUDIO, EXIT_FAILURE, fmtStr, "NOMEM");					\
		return false;																\
	}

#define SPEEX_RESAMPLER_ERROR_CHECK(ptr, fmtStr, errCode)							\
	if(errCode != RESAMPLER_ERR_SUCCESS || !ptr)									\
	{																				\
		Assert(0);																	\
		if(errCode != RESAMPLER_ERR_SUCCESS)										\
			Error(eDLL_T::AUDIO, errCode, fmtStr, speex_resampler_strerror(error));	\
		else																		\
		    Error(eDLL_T::AUDIO, EXIT_FAILURE, fmtStr, "NOMEM");					\
		return false;																\
	}

bool VoiceEncoder_Opus::Init(int quality, int& rawFrameSize, int& encodedFrameSize)
{
	Assert(m_pEncodeResampler == nullptr);
	Assert(m_pEncoder == nullptr);
	Assert(m_pDecoder == nullptr);

	int error = 0;

	SpeexResamplerState* const pEncodeResampler = speex_resampler_init(1, Voice_SamplesPerSec(), OPUS_VOICE_ENCODED_SAMPLERATE, quality, &error);
	SPEEX_RESAMPLER_ERROR_CHECK(pEncodeResampler, "Failed to initialize speex encode resampler: %s", error);
	m_pEncodeResampler = pEncodeResampler;

	SpeexResamplerState* const pDecodeResampler = speex_resampler_init(1, OPUS_VOICE_ENCODED_SAMPLERATE, Voice_SamplesPerSec(), quality, &error);
	SPEEX_RESAMPLER_ERROR_CHECK(pDecodeResampler, "Failed to initialize speex decode resampler: %s", error);
	m_pDecodeResampler = pDecodeResampler;


	OpusEncoder* const pEncoder = opus_encoder_create(OPUS_VOICE_ENCODED_SAMPLERATE, Voice_GetChannelCount(), OPUS_APPLICATION_VOIP, &error);
	OPUS_ERROR_CHECK(pEncoder, "Failed to create opus encoder: %s", error);
	m_pEncoder = pEncoder;
	
	OpusDecoder* const pDecoder = opus_decoder_create(OPUS_VOICE_ENCODED_SAMPLERATE, Voice_GetChannelCount(), &error);
	OPUS_ERROR_CHECK(pDecoder, "Failed to create opus decoder: %s", error);
	m_pDecoder = pDecoder;

	opus_int32 bitrate = 0;

	//engine uses fixed encoded frame sizes which is ass
	opus_encoder_ctl(m_pEncoder, OPUS_SET_VBR(0));
	opus_encoder_ctl(m_pEncoder, OPUS_GET_BITRATE(&bitrate));


	//These are probably wrong but idk why the default is 160 for raw frames
	m_RawFrameSizeSamples = 441;
	rawFrameSize = m_RawFrameSizeSamples * BYTES_PER_SAMPLE;

	m_EncodedFrameSize = 480;
	encodedFrameSize = m_EncodedFrameSize * BYTES_PER_SAMPLE;

	return true;
}

#undef OPUS_ERROR_CHECK

void VoiceEncoder_Opus::Release()
{
	Assert(m_pEncodeResampler != nullptr);
	speex_resampler_destroy(m_pEncodeResampler);
	m_pEncodeResampler = nullptr;

	Assert(m_pDecodeResampler != nullptr);
	speex_resampler_destroy(m_pDecodeResampler);
	m_pDecodeResampler = nullptr;

	Assert(m_pEncoder != nullptr);
	opus_encoder_destroy(m_pEncoder);
	m_pEncoder = nullptr;

	Assert(m_pDecoder != nullptr);
	opus_decoder_destroy(m_pDecoder);
	m_pDecoder = nullptr;
}

bool VoiceEncoder_Opus::EncodeFrame(const char* pUncompressed, char* pCompressed)
{
	Assert(m_pEncoder);
	const opus_int16* pInputData = reinterpret_cast<const opus_int16*>(pUncompressed);
	opus_int16* pOutputBuffer = reinterpret_cast<opus_int16*>(pCompressed);

	spx_uint32_t RawSampleCount = m_RawFrameSizeSamples;
	spx_uint32_t ResampleBufferSize = m_EncodedFrameSize;
		
	speex_resampler_process_int(m_pEncodeResampler, 0, pInputData, &RawSampleCount, pOutputBuffer, &ResampleBufferSize);
	//opus_encode(m_pEncoder, resampleBuffer, ResampleBufferSize, pOutputBuffer, m_EncodedFrameSize);

	//memcpy(pCompressed, pUncompressed, m_RawFrameSizeSamples * BYTES_PER_SAMPLE);

	return true;
}

void VoiceEncoder_Opus::DecodeFrame(const char* pCompressed, char* pDecompressed)
{
	Assert(m_pDecoder);
	const opus_int16* pInputData = reinterpret_cast<const opus_int16*>(pCompressed);
	opus_int16* const pOutput = reinterpret_cast<opus_int16* const>(pDecompressed);

	uint32_t OutBufferSampleCount = m_RawFrameSizeSamples;
	uint32_t numDecodedSamples = m_EncodedFrameSize; 
	//opus_decode(m_pDecoder, pInputData, m_EncodedFrameSize, resampleBuffer, sizeof(resampleBuffer), 0);

	//memcpy(pDecompressed, pCompressed, m_RawFrameSizeSamples * BYTES_PER_SAMPLE);

	speex_resampler_process_int(m_pDecodeResampler, 0, pInputData, &numDecodedSamples, pOutput, &OutBufferSampleCount);

	//memcpy(pDecompressed, pCompressed, m_ )

	return;
}

bool VoiceEncoder_Opus::ResetState()
{
	Assert(m_pEncodeResampler != nullptr);
	Assert(m_pEncoder != nullptr);
	Assert(m_pDecoder != nullptr);

	int error = 0;
	
	error = speex_resampler_reset_mem(m_pEncodeResampler);
	
	if (error != RESAMPLER_ERR_SUCCESS)
	{
		Error(eDLL_T::AUDIO, error, "Failed to reset speex resampler: %s", speex_resampler_strerror(error));
		return false;
	}

	error = opus_encoder_ctl(m_pEncoder, OPUS_RESET_STATE);

	if (error != OPUS_OK)
	{
		Error(eDLL_T::AUDIO, error, "Failed to reset opus encoder: %s", opus_strerror(error));
		return false;
	}

	error = opus_decoder_ctl(m_pDecoder, OPUS_RESET_STATE);
	
	if (error != OPUS_OK)
	{
		Error(eDLL_T::AUDIO, error, "Failed to reset opus decoder: %s", opus_strerror(error));
		return false;
	}

	return true;
}

VoiceCodec_Frame* CreateOpusVoiceCodec()
{
	IFrameEncoder* pEncoder = new VoiceEncoder_Opus;
	return CreateVoiceCodec_Frame(pEncoder);
}