//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef VOICE_H
#define VOICE_H
#include "engine/shared/voice_codecs/voice_codec_frame.h"

#define VOICE_OUTPUT_SAMPLE_RATE 22050

inline bool g_bVoiceAtLeastPartiallyInitted = false;
inline class VoiceRecord_DSound** g_ppRecord;
inline VoiceCodec_Frame** g_ppVoiceCodecFrame;

inline class CVoiceChannel* g_pVoiceChannels;

inline void(*v_Voice_Init)();

int Voice_SamplesPerSec();
int Voice_AvgBytesPerSec();
int Voice_GetChannelCount();

void Voice_Init();
inline void Voice_Deinit()
{
};

class VVoice : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("Voice_Init", v_Voice_Init);
		LogVarAdr("g_pRecord", g_ppRecord);
		LogVarAdr("g_pVoiceCodecFrame", g_ppVoiceCodecFrame);
		LogVarAdr("g_VoiceChannels", g_pVoiceChannels);
	}
	virtual void GetFun(void) const 
	{
		g_GameDll.FindPatternSIMD("48 83 EC 38 48 8B 05 ? ? ? ? 83 78 6C 00 75 07 32 C0 48 83 C4 38 C3").GetPtr(v_Voice_Init);
	}
	virtual void GetVar(void) const 
	{
		CMemory(v_Voice_Init).OffsetSelf(0xD8).FindPatternSelf("48 89 1D").ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_ppRecord);
		CMemory(v_Voice_Init).OffsetSelf(0x1A0).FindPatternSelf("48 89").ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_ppVoiceCodecFrame);

		g_GameDll.FindPatternSIMD("48 89 5C 24 ? 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 8B 7A 24").OffsetSelf(0xEA)
			.FindPatternSelf("48 8D 0D").ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_pVoiceChannels);
	}
	virtual void GetCon(void) const { }
	virtual void Detour(const bool bAttach) const 
	{
		DetourSetup(&v_Voice_Init, &Voice_Init, bAttach);
	}
};

#endif // !VOICE_H

