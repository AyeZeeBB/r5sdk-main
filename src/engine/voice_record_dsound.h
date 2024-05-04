#ifndef VOICE_RECORD_DSOUND_H
#define VOICE_RECORD_DSOUND_H
#include "ivoicerecord.h"

class VoiceRecord_DSound;

inline void(*v_VoiceRecord_DSound__Term)(VoiceRecord_DSound* thisp);
inline void* g_pVoiceRecord_DSound_vtbl;

class VoiceRecord_DSound : public IVoiceRecord
{
protected:
	virtual				~VoiceRecord_DSound() { v_VoiceRecord_DSound__Term(this); }
public:
	VoiceRecord_DSound() 
	{ 
		void** pVFTable = reinterpret_cast<void**>(this);
		*pVFTable = g_pVoiceRecord_DSound_vtbl;

		Clear(); 
	}

	virtual void		Release() { delete this; }

	virtual bool		RecordStart() 
	{
		const static int index = 2;
		return CallVFunc<bool>(index, this);
	}

	virtual void		RecordStop()
	{
		const static int index = 3;
		CallVFunc<void>(index, this);
	}

	virtual int			Idle()
	{
		const static int index = 4;
		return CallVFunc<int>(index, this);
	}

	virtual void		Unk()
	{
		const static int index = 5;
		CallVFunc<void>(index, this);
	}

	virtual int			GetRecordedData(short* pOut, int nSamplesWanted, bool bUnk)
	{
		const static int index = 6;
		return CallVFunc<int>(index, this);
	}
	
	virtual bool		Init(int nSampleRate)
	{
		const static int index = 7;
		return CallVFunc<bool>(index, this, nSampleRate);
	}

	void Clear();
public:
	HINSTANCE m_hInstDS;
	void* m_pCapture;
	void* m_pCaptureBuffer;
	HANDLE m_hWrapEvent;
	DWORD m_nCaptureBufferBytes;
	char m_gap02C[4];
	size_t m_WrapOffset;
	size_t m_LastReadPos;
	char m_gap034[7];
	bool m_unkBool;
	char m_gap048[4];
	JobID_t m_VoiceJobID;
};

static_assert(sizeof(VoiceRecord_DSound) == 0x50);

VoiceRecord_DSound* CreateVoiceRecord_DSound(int nSampleRate);

class VVoiceRecord_DSound : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("VoiceRecord_DSound::Term", v_VoiceRecord_DSound__Term);
		LogConAdr("VoiceRecord_DSound::`vftable'", g_pVoiceRecord_DSound_vtbl);
	}
	virtual void GetFun(void) const
	{
		g_GameDll.FindPatternSIMD("48 89 5C 24 ? 57 48 83 EC 20 33 D2 48 8B D9 8B 49 4C 4C 8D 42 FF").GetPtr(v_VoiceRecord_DSound__Term);
	}

	virtual void GetVar(void) const { }
	virtual void GetCon(void) const 
	{ 
		g_pVoiceRecord_DSound_vtbl = g_GameDll.GetVirtualMethodTable(".?AVVoiceRecord_DSound@@");
	}
	virtual void Detour(const bool bAttach) const { }
};

#endif // !VOICE_RECORD_DSOUND_H
