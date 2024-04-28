//=============================================================================//
//
// Purpose: Interface server dll virtual functions to the SDK.
//
// $NoKeywords: $
//=============================================================================//
#ifndef GAMEINTERFACE_H
#define GAMEINTERFACE_H
#include "public/eiface.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class ServerClass;
class IRecipientFilter;
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
class CServerGameDLL
{
public:
	bool GameInit(void);
	void PrecompileScriptsJob(void);
	void LevelShutdown(void);
	void GameShutdown(void);
	float GetTickInterval(void);
	ServerClass* GetAllServerClasses(void);

	static void __fastcall OnReceivedSayTextMessage(void* thisptr, int senderId, const char* text, bool isTeamChat);
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
class CServerGameClients : public IServerGameClients
{
public:
	static void ProcessUserCmds(CServerGameClients* thisp, edict_t edict, bf_read* buf,
		int numCmds, int totalCmds, int droppedPackets, bool ignore, bool paused);
private:
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
class CServerGameEnts : public IServerGameEnts
{
};

inline void(*CServerGameDLL__OnReceivedSayTextMessage)(void* thisptr, int senderId, const char* text, bool isTeamChat);
inline bool(*CServerGameDLL__GameInit)(void);

inline void(*CServerGameClients__ProcessUserCmds)(CServerGameClients* thisp, edict_t edict, bf_read* buf,
	int numCmds, int totalCmds, int droppedPackets, bool ignore, bool paused);

inline void(*v_RunFrameServer)(double flFrameTime, bool bRunOverlays, bool bUniformUpdate);
inline void(*v_UserMessageBegin)(IRecipientFilter* filter, const char* pszMessageName, int messageIndex);

inline float* g_pflServerFrameTimeBase = nullptr;
inline bf_write** g_ppMessageBuffer = nullptr;

extern CServerGameDLL* g_pServerGameDLL;
extern CServerGameClients* g_pServerGameClients;
extern CServerGameEnts* g_pServerGameEntities;

extern CGlobalVars** g_pGlobals;

inline void MessageEnd( void );

inline void MessageWriteByte( int iValue );
inline void MessageWriteString( const char* pszString );
inline void MessageWriteBool( bool bValue );


///////////////////////////////////////////////////////////////////////////////
class VServerGameDLL : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("CServerGameDLL::OnReceivedSayTextMessage", CServerGameDLL__OnReceivedSayTextMessage);
		LogFunAdr("CServerGameDLL::GameInit", CServerGameDLL__GameInit);
		LogFunAdr("CServerGameClients::ProcessUserCmds", CServerGameClients__ProcessUserCmds);
		LogFunAdr("RunFrameServer", v_RunFrameServer);
		LogFunAdr("UserMessageBegin", v_UserMessageBegin);
		LogVarAdr("g_flServerFrameTimeBase", g_pflServerFrameTimeBase);
		LogVarAdr("g_pServerGameDLL", g_pServerGameDLL);
		LogVarAdr("g_pServerGameClients", g_pServerGameClients);
		LogVarAdr("g_pServerGameEntities", g_pServerGameEntities);
		LogVarAdr("g_pGlobals", g_pGlobals);
		LogVarAdr("g_ppMessageBuffer", g_ppMessageBuffer);
	}
	virtual void GetFun(void) const
	{
		g_GameDll.FindPatternSIMD("85 D2 0F 8E ?? ?? ?? ?? 4C 8B DC").GetPtr(CServerGameDLL__OnReceivedSayTextMessage);
		g_GameDll.FindPatternSIMD("48 83 EC 28 48 8B 0D ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? 48 8B 01 FF 90 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8B 01").GetPtr(CServerGameDLL__GameInit);
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 55 41 55 41 57").GetPtr(CServerGameClients__ProcessUserCmds);
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 57 48 83 EC 30 0F 29 74 24 ?? 48 8D 0D ?? ?? ?? ??").GetPtr(v_RunFrameServer);
		g_GameDll.FindPatternSIMD("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 40 41 8B E8").GetPtr(v_UserMessageBegin);
	}
	virtual void GetVar(void) const
	{
		g_pGlobals = g_GameDll.FindPatternSIMD("4C 8B 0D ?? ?? ?? ?? 48 8B D1").ResolveRelativeAddressSelf(0x3, 0x7).RCast<CGlobalVars**>();
		g_pflServerFrameTimeBase = CMemory(CServerGameDLL__GameInit).FindPatternSelf("F3 0F 11 0D").ResolveRelativeAddressSelf(0x4, 0x8).RCast<float*>();
		CMemory(v_UserMessageBegin).OffsetSelf(0xB1).FindPatternSelf("48 89").ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_ppMessageBuffer);
	}
	virtual void GetCon(void) const { }
	virtual void Detour(const bool bAttach) const;
};
///////////////////////////////////////////////////////////////////////////////

#endif // GAMEINTERFACE_H