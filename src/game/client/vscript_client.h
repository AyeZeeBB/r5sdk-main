#ifndef VSCRIPT_CLIENT_H
#define VSCRIPT_CLIENT_H

namespace VScriptCode
{
	namespace Client
	{
		SQRESULT IsClientDLL(HSQUIRRELVM v);
	}

	namespace Ui
	{
		SQRESULT RefreshServerList(HSQUIRRELVM v);
		SQRESULT GetServerCount(HSQUIRRELVM v);

		SQRESULT GetHiddenServerName(HSQUIRRELVM v);
		SQRESULT GetServerName(HSQUIRRELVM v);
		SQRESULT GetServerDescription(HSQUIRRELVM v);

		SQRESULT GetServerMap(HSQUIRRELVM v);
		SQRESULT GetServerPlaylist(HSQUIRRELVM v);

		SQRESULT GetServerCurrentPlayers(HSQUIRRELVM v);
		SQRESULT GetServerMaxPlayers(HSQUIRRELVM v);

		SQRESULT GetPromoData(HSQUIRRELVM v);

		SQRESULT ConnectToListedServer(HSQUIRRELVM v);
		SQRESULT ConnectToHiddenServer(HSQUIRRELVM v);
		SQRESULT ConnectToServer(HSQUIRRELVM v);
	}
}

void Script_RegisterClientFunctions(CSquirrelVM* s);
void Script_RegisterUIFunctions(CSquirrelVM* s);
void Script_RegisterUIServerFunctions(CSquirrelVM* s);
void Script_RegisterCoreClientFunctions(CSquirrelVM* s);

#define DEFINE_CLIENT_SCRIPTFUNC_NAMED(s, functionName, helpString,     \
	returnType, parameters)                                             \
	s->RegisterFunction(#functionName, MKSTRING(Script_##functionName), \
	helpString, returnType, parameters, VScriptCode::Client::##functionName);   \

#define DEFINE_UI_SCRIPTFUNC_NAMED(s, functionName, helpString,     \
	returnType, parameters)                                             \
	s->RegisterFunction(#functionName, MKSTRING(Script_##functionName), \
	helpString, returnType, parameters, VScriptCode::Ui::##functionName);   \

inline void (*v_Script_RegisterClientEntityClassFuncs)();
inline void (*v_Script_RegisterClientPlayerClassFuncs)();
inline void (*v_Script_RegisterClientAIClassFuncs)();
inline void (*v_Script_RegisterClientWeaponClassFuncs)();
inline void (*v_Script_RegisterClientProjectileClassFuncs)();
inline void (*v_Script_RegisterClientTitanSoulClassFuncs)();
inline void (*v_Script_RegisterClientPlayerDecoyClassFuncs)();
inline void (*v_Script_RegisterClientFirstPersonProxyClassFuncs)();

inline ScriptClassDescriptor_t* g_clientScriptEntityStruct;
inline ScriptClassDescriptor_t* g_clientScriptPlayerStruct;
inline ScriptClassDescriptor_t* g_clientScriptAIStruct;
inline ScriptClassDescriptor_t* g_clientScriptWeaponStruct;
inline ScriptClassDescriptor_t* g_clientScriptProjectileStruct;
inline ScriptClassDescriptor_t* g_clientScriptTitanSoulStruct;
inline ScriptClassDescriptor_t* g_clientScriptPlayerDecoyStruct;
inline ScriptClassDescriptor_t* g_clientScriptFirstPersonProxyStruct;

///////////////////////////////////////////////////////////////////////////////
class VScriptClient : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("Script_RegisterClientEntityClassFuncs", v_Script_RegisterClientEntityClassFuncs);
		LogFunAdr("Script_RegisterClientPlayerClassFuncs", v_Script_RegisterClientPlayerClassFuncs);
		LogFunAdr("Script_RegisterClientAIClassFuncs", v_Script_RegisterClientAIClassFuncs);
		LogFunAdr("Script_RegisterClientWeaponClassFuncs", v_Script_RegisterClientWeaponClassFuncs);
		LogFunAdr("Script_RegisterClientProjectileClassFuncs", v_Script_RegisterClientProjectileClassFuncs);
		LogFunAdr("Script_RegisterClientTitanSoulClassFuncs", v_Script_RegisterClientTitanSoulClassFuncs);
		LogFunAdr("Script_RegisterClientPlayerDecoyClassFuncs", v_Script_RegisterClientPlayerDecoyClassFuncs);
		LogFunAdr("Script_RegisterClientFirstPersonProxyClassFuncs", v_Script_RegisterClientFirstPersonProxyClassFuncs);

		LogVarAdr("g_clientScriptEntityStruct", g_clientScriptEntityStruct);
		LogVarAdr("g_clientScriptPlayerStruct", g_clientScriptPlayerStruct);
		LogVarAdr("g_clientScriptAIStruct", g_clientScriptAIStruct);
		LogVarAdr("g_clientScriptWeaponStruct", g_clientScriptWeaponStruct);
		LogVarAdr("g_clientScriptProjectileStruct", g_clientScriptProjectileStruct);
		LogVarAdr("g_clientScriptTitanSoulStruct", g_clientScriptTitanSoulStruct);
		LogVarAdr("g_clientScriptPlayerDecoyStruct", g_clientScriptPlayerDecoyStruct);
		LogVarAdr("g_clientScriptFirstPersonProxyStruct", g_clientScriptFirstPersonProxyStruct);
	}
	virtual void GetFun(void) const
	{
		g_GameDll.FindPatternSIMD("40 55 48 8B EC 48 83 EC ?? 80 3D ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 5C 24 ?? 48 89 05 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? C6 05 ?? ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 C7 05")
			.GetPtr(v_Script_RegisterClientEntityClassFuncs);

		g_GameDll.FindPatternSIMD("40 55 48 8B EC 48 83 EC ?? 80 3D ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 5C 24 ?? 48 89 05 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? C6 05 ?? ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 C7 05 ?? ?? ?? ?? ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 63 C8 48 6B C1 ?? 48 8D 0D ?? ?? ?? ?? 48 03 05 ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? 48 C7 40 ?? ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? C6 40 ?? ?? 48 C7 40 ?? ?? ?? ?? ?? 48 C7 40 ?? ?? ?? ?? ?? 48 89 08 48 8D 0D ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 63 C8 48 6B C1 ?? 48 8D 0D ?? ?? ?? ?? 48 03 05 ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? 48 C7 40 ?? ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? C6 40 ?? ?? 48 C7 40 ?? ?? ?? ?? ?? 48 C7 40 ?? ?? ?? ?? ?? 48 89 08 48 8D 0D ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 63 C8 4C 8D 45 ?? 48 6B D9 ?? 48 8D 05")
			.GetPtr(v_Script_RegisterClientPlayerClassFuncs);

		g_GameDll.FindPatternSIMD("48 83 EC ?? 80 3D ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 5C 24 ?? 48 63 1D")
			.GetPtr(v_Script_RegisterClientAIClassFuncs);

		g_GameDll.FindPatternSIMD("40 55 48 8B EC 48 83 EC ?? 80 3D ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 5C 24 ?? 48 89 05 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? C6 05 ?? ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 C7 05 ?? ?? ?? ?? ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 63 C8 48 6B C1 ?? 48 8D 0D ?? ?? ?? ?? 48 03 05 ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? 48 C7 40 ?? ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? C6 40 ?? ?? 48 C7 40 ?? ?? ?? ?? ?? 48 C7 40 ?? ?? ?? ?? ?? 48 89 08 48 8D 0D ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 63 C8 48 6B C1 ?? 48 8D 0D ?? ?? ?? ?? 48 03 05 ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? 48 C7 40 ?? ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? C6 40 ?? ?? 48 C7 40 ?? ?? ?? ?? ?? 48 C7 40 ?? ?? ?? ?? ?? 48 89 08 48 8D 0D ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? C7 40 ?? ?? ?? ?? ?? 48 89 48 ?? 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 63 C8 4C 8D 45 ?? 48 6B D9 ?? C7 45")
			.GetPtr(v_Script_RegisterClientWeaponClassFuncs);

		g_GameDll.FindPatternSIMD("48 8B C4 48 83 EC ?? 80 3D ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 89 58 ?? 48 63 1D ?? ?? ?? ?? 48 89 68 ?? 33 ED 48 89 70 ?? 48 89 78 ?? 4C 89 60 ?? 4C 89 68 ?? 4C 89 70 ?? 44 8D 75 ?? 4C 89 78 ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 8D 43 ?? 4C 63 C8 49 8B C1 C6 05 ?? ?? ?? ?? ?? 48 2B C1 48 89 2D ?? ?? ?? ?? 48 85 C0 0F 8E ?? ?? ?? ?? 4C 8B 05 ?? ?? ?? ?? 4D 85 C0 0F 88 ?? ?? ?? ?? 74 ?? 49 8D 49 ?? 48 8B C1 48 99 49 F7 F8 4C 2B C2 4C 03 C1 EB ?? 48 85 C9 4C 8B C1 4D 0F 44 C6 4D 3B C1 7D ?? 4D 03 C0 4D 3B C1 7C ?? 4D 3B C1 7D ?? 4D 85 C0 75 ?? 4D 85 C9 79 ?? 49 C7 C0 ?? ?? ?? ?? EB ?? 4B 8D 04 08 48 99 48 2B C2 48 D1 F8 4C 8B C0 49 3B C1 7C ?? 48 8B 15 ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 4C 89 05 ?? ?? ?? ?? 48 85 D2 74 ?? 48 85 C0 75 ?? E8 ?? ?? ?? ?? 4C 8B 05 ?? ?? ?? ?? 48 8B 15 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 4C 8B 08 48 8B C8 4D 6B C0 ?? 41 FF 51 ?? 48 8B D0 48 89 05 ?? ?? ?? ?? EB ?? 49 6B F8 ?? 48 85 C0 75 ?? E8 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 4C 8B 00 48 8B D7 48 8B C8 41 FF 50 ?? 48 8B D0 48 89 05 ?? ?? ?? ?? EB ?? 48 8B 15 ?? ?? ?? ?? FF 05 ?? ?? ?? ?? 4C 8D 2D")
			.GetPtr(v_Script_RegisterClientProjectileClassFuncs);

		g_GameDll.FindPatternSIMD("48 83 EC ?? 80 3D ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 5C 24 ?? 48 89 6C 24 ?? 33 ED 48 89 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 89 74 24 ?? 48 89 7C 24 ?? 48 63 3D ?? ?? ?? ?? 4C 89 74 24 ?? 44 8D 75 ?? C6 05 ?? ?? ?? ?? ?? 48 89 2D ?? ?? ?? ?? 8D 47 ?? 4C 63 C0 49 8B C0 48 2B C1 48 85 C0 0F 8E ?? ?? ?? ?? 4C 8B 0D ?? ?? ?? ?? 4D 85 C9 0F 88 ?? ?? ?? ?? 74 ?? 49 8D 48 ?? 48 8B C1 48 99 49 F7 F9 48 2B CA 49 03 C9 EB ?? 48 85 C9 49 0F 44 CE 49 3B C8 7D ?? 48 03 C9 49 3B C8 7C ?? 49 3B C8 7D ?? 48 85 C9 75 ?? 4D 85 C0 79 ?? 48 C7 C1 ?? ?? ?? ?? EB ?? 0F 1F 40 ?? 0F 1F 84 00 ?? ?? ?? ?? 4A 8D 04 01 48 99 48 2B C2 48 D1 F8 48 8B C8 49 3B C0 7C ?? 48 8B 15 ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 89 0D ?? ?? ?? ?? 48 85 D2 74 ?? 48 85 C0 75 ?? E8 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8B 15 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 4C 8B 08 4C 6B C1 ?? 48 8B C8 41 FF 51 ?? 48 8B D0 48 89 05 ?? ?? ?? ?? EB ?? 48 6B D9 ?? 48 85 C0 75 ?? E8 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 4C 8B 00 48 8B D3 48 8B C8 41 FF 50 ?? 48 8B D0 48 89 05 ?? ?? ?? ?? EB ?? 48 8B 15 ?? ?? ?? ?? FF 05 ?? ?? ?? ?? 48 8D 35")
			.GetPtr(v_Script_RegisterClientTitanSoulClassFuncs);

		g_GameDll.FindPatternSIMD("80 3D ?? ?? ?? ?? ?? 75 ?? 48 8D 05 ?? ?? ?? ?? C6 05 ?? ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 C7 05 ?? ?? ?? ?? ?? ?? ?? ?? C3 CC CC CC 48 8D 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 89 5C 24")
			.GetPtr(v_Script_RegisterClientPlayerDecoyClassFuncs);

		g_GameDll.FindPatternSIMD("48 83 EC ?? 80 3D ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 05 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 74 24 ?? 33 F6 48 89 05 ?? ?? ?? ?? 48 89 7C 24 ?? 48 63 3D ?? ?? ?? ?? 4C 89 74 24 ?? 8D 6E ?? 4C 8D 35 ?? ?? ?? ?? C6 05 ?? ?? ?? ?? ?? 4C 89 35 ?? ?? ?? ?? 8D 47 ?? 48 89 35 ?? ?? ?? ?? 4C 63 C0")
			.GetPtr(v_Script_RegisterClientFirstPersonProxyClassFuncs);
	}
	virtual void GetVar(void) const
	{
		CMemory(v_Script_RegisterClientEntityClassFuncs).FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 150, 2).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_clientScriptEntityStruct);
		CMemory(v_Script_RegisterClientPlayerClassFuncs).FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 150, 2).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_clientScriptPlayerStruct);
		CMemory(v_Script_RegisterClientAIClassFuncs).FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 150, 2).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_clientScriptAIStruct);
		CMemory(v_Script_RegisterClientWeaponClassFuncs).FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 150, 2).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_clientScriptWeaponStruct);
		CMemory(v_Script_RegisterClientProjectileClassFuncs).FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 150, 2).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_clientScriptProjectileStruct);
		CMemory(v_Script_RegisterClientTitanSoulClassFuncs).FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 150, 2).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_clientScriptTitanSoulStruct);
		CMemory(v_Script_RegisterClientPlayerDecoyClassFuncs).FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 150, 2).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_clientScriptPlayerDecoyStruct);
		CMemory(v_Script_RegisterClientFirstPersonProxyClassFuncs).FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 150, 2).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(g_clientScriptFirstPersonProxyStruct);
	}
	virtual void GetCon(void) const { }
	virtual void Detour(const bool bAttach) const;
};
///////////////////////////////////////////////////////////////////////////////

#endif // VSCRIPT_CLIENT_H
