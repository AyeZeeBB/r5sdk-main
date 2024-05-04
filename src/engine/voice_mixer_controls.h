//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef MIXER_CONTROLS_H
#define MIXER_CONTROLS_H
#include "mmeapi.h"

class CMixerControls;

inline void* g_pCMixerControls__vtbl;
inline CMixerControls* g_pMixerControls;

inline void(*v_InitMixerControls)();
inline bool(*v_CMixerControls__Init)(CMixerControls* thisp);

class IMixerControls
{
protected:
	virtual			~IMixerControls() {}
public:

	enum Control
	{
		// Microphone boost is a boolean switch that sound cards support which boosts the input signal by about +20dB.
		// If this isn't on, the mic is usually way too quiet.
		MicBoost = 0,

		// Volume values are 0-1.
		MicVolume,

		// Mic playback muting. You usually want this set to false, otherwise the sound card echoes whatever you say into the mic.
		MicMute,

		NumControls
	};

	virtual bool	GetValue_Float(Control iControl, float& value) = 0;
	virtual bool	SetValue_Float(Control iControl, float value) = 0;

	// Apps like RealJukebox will switch the waveIn input to use CD audio 
	// rather than the microphone. This should be called at startup to set it back.
	virtual bool	SelectMicrophoneForWaveInput() = 0;
};

class CMixerControls : public IMixerControls
{
public:
	CMixerControls()
	{
		void** pVFTable = reinterpret_cast<void**>(this);
		*pVFTable = g_pCMixerControls__vtbl;

		m_dwMicSelectControlID = 0xFFFFFFFF;
		Init();
	}

	virtual			~CMixerControls() 
	{
		const static int index = 0;
		CallVFunc<void>(index, this);
	}

	void			Release()
	{
		delete this;
	}

	virtual bool	GetValue_Float(Control iControl, float& value)
	{
		const static int index = 1;
		return CallVFunc<bool>(index, this, iControl, value);
	}

	virtual bool	SetValue_Float(Control iControl, float value)
	{
		const static int index = 2;
		return CallVFunc<bool>(index, this, iControl, value);
	}

	virtual bool SelectMicrophoneForWaveInput()
	{
		const static int index = 3;
		return CallVFunc<bool>(index, this);
	}

private:
	bool			Init()
	{
		v_CMixerControls__Init(this);
	}

	void			Clear()
	{
		m_hMixer = 0;
		memset(m_ControlInfos, 0, sizeof(m_ControlInfos));
	}

	HMIXEROBJ m_hMixer;
	DWORD m_dwMicSelectControlID;
	DWORD m_dwMicSelectMultipleItems;
	DWORD m_dwMicSelectControlType;
	DWORD m_dwMicSelectIndex;
	Control m_ControlInfos[NumControls];
};


class VMixerControls : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("InitMixerControls", v_InitMixerControls);
		LogFunAdr("CMixerControls::Init", v_CMixerControls__Init);
		LogConAdr("CMixerControls::`vftable'", g_pCMixerControls__vtbl);
	}
	virtual void GetFun(void) const
	{
		g_GameDll.FindPatternSIMD("E8 ? ? ? ? 48 8B 05 ? ? ? ? 39 70 6C 74 12").FollowNearCallSelf().GetPtr(v_InitMixerControls);
		g_GameDll.FindPatternSIMD("40 55 53 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ?").GetPtr(v_CMixerControls__Init);
	}
	virtual void GetVar(void) const 
	{
		CMemory(v_InitMixerControls).OffsetSelf(0x68).FindPatternSelf("48 89 1D E5").ResolveRelativeAddressSelf(0x3, 0x7);
	}
	virtual void GetCon(void) const 
	{
		g_pCMixerControls__vtbl = g_GameDll.GetVirtualMethodTable(".?AVCMixerControls@@");
	}
	virtual void Detour(const bool bAttach) const { }
};

#endif // MIXER_CONTROLS_H
