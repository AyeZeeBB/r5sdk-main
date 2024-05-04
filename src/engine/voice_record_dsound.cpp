#include "voice_record_dsound.h"

void VoiceRecord_DSound::Clear()
{
	m_hInstDS = 0;
	m_pCapture = nullptr;
	m_pCaptureBuffer = nullptr;
	m_hWrapEvent = 0;
	m_nCaptureBufferBytes = 0;
	m_WrapOffset = 0;
	m_LastReadPos = UINT64_MAX;
	m_unkBool = false;
	m_VoiceJobID = 12288;

	memset(m_gap02C, 0, sizeof(m_gap02C));
	memset(m_gap034, 0, sizeof(m_gap034));
	memset(m_gap048, 0, sizeof(m_gap048));
}

VoiceRecord_DSound* CreateVoiceRecord_DSound(int nSampleRate)
{
	VoiceRecord_DSound* pRecord = new VoiceRecord_DSound;

	if (pRecord && pRecord->Init(nSampleRate))
	{
		return pRecord;
	}
	else
	{
		if (pRecord)
			pRecord->Release();

		return nullptr;
	}
}