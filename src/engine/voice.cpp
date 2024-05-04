#include "voice.h"
#include <Mmreg.h>
#include "voice_record_dsound.h"
#include "shared/voice_codecs/opus/VoiceEncoder_Opus.h"
#include "voice_gain.h"
#include "voice_mixer_controls.h"
#include <vgui/vgui_baseui_interface.h>

#define VOICE_CODEC_QUALITY 5
#define VOICE_NUM_CHANNELS 2


WAVEFORMATEX g_VoiceSampleFormat{
	1,
	1,
	VOICE_OUTPUT_SAMPLE_RATE,
	VOICE_OUTPUT_SAMPLE_RATE * 2,
	2,
	16,
	sizeof(WAVEFORMATEX)
};

class CVoiceChannel
{
public:
	void SetCodec(IVoiceCodec* pCodec) { m_pVoiceCodec = pCodec; }

private:
	int m_iEntity;
	char m_Buffer[44100];
	char m_gapAC48[56];
	IVoiceCodec* m_pVoiceCodec;
	CAutoGain m_AutoGain;
	char m_gap001[220540];
};

static_assert(sizeof(CVoiceChannel) == 0x40A30);

int Voice_SamplesPerSec()
{
	return g_VoiceSampleFormat.nSamplesPerSec;
}

int Voice_AvgBytesPerSec()
{
	return (g_VoiceSampleFormat.nSamplesPerSec * g_VoiceSampleFormat.wBitsPerSample) >> 3;
}

int Voice_GetChannelCount()
{
	return g_VoiceSampleFormat.nChannels;
}

void Voice_Init()
{
	if (!voice_enabled->GetBool())
		return;

	Voice_Deinit();

	g_bVoiceAtLeastPartiallyInitted = true;
	g_pEngineVGui->UpdateProgressBar(LevelLoadingProgress_e::PROGRESS_DEFAULT);

	//Setup the global frame encoder
	*g_ppRecord = CreateVoiceRecord_DSound(Voice_SamplesPerSec());
	
	VoiceCodec_Frame* pCodec = CreateOpusVoiceCodec();
	*g_ppVoiceCodecFrame = pCodec;

	if (!pCodec->Init(VOICE_CODEC_QUALITY, Voice_SamplesPerSec()))
	{
		Voice_Deinit();
		return;
	}

	//Create and set the encoders for our voice channels
	for (size_t i = 0; i < VOICE_NUM_CHANNELS; i++)
	{
		VoiceCodec_Frame* pChannelCodec = CreateOpusVoiceCodec();
		g_pVoiceChannels[i].SetCodec(pChannelCodec);
		
		if (!pChannelCodec->Init(VOICE_CODEC_QUALITY, Voice_SamplesPerSec()))
		{
			Voice_Deinit();
			return;
		}
	}
	
	v_InitMixerControls();

	if (voice_forcemicrecord->GetBool())
	{
		if (g_pMixerControls)
			g_pMixerControls->SelectMicrophoneForWaveInput();
	}
}