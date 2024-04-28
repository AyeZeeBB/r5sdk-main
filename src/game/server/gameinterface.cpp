//=============================================================================//
//
// Purpose: Implement things from GameInterface.cpp. Mostly the engine interfaces.
//
// $NoKeywords: $
//=============================================================================//

#include "core/stdafx.h"
#include "tier1/cvar.h"
#include "public/server_class.h"
#include "public/eiface.h"
#include "public/const.h"
#include "common/protocol.h"
#include "engine/server/sv_main.h"
#include "gameinterface.h"
#include "entitylist.h"
#include "baseanimating.h"
#include "engine/server/server.h"
#include "game/shared/usercmd.h"
#include "game/server/util_server.h"
#include "pluginsystem/pluginsystem.h"
#include "game/server/recipientfilter.h"

//-----------------------------------------------------------------------------
// This is called when a new game is started. (restart, map)
//-----------------------------------------------------------------------------
bool CServerGameDLL::GameInit(void)
{
	const static int index = 1;
	return CallVFunc<bool>(index, this);
}

//-----------------------------------------------------------------------------
// This is called when scripts are getting recompiled. (restart, map, changelevel)
//-----------------------------------------------------------------------------
void CServerGameDLL::PrecompileScriptsJob(void)
{
	const static int index = 2;
	CallVFunc<void>(index, this);
}

//-----------------------------------------------------------------------------
// Called when a level is shutdown (including changing levels)
//-----------------------------------------------------------------------------
void CServerGameDLL::LevelShutdown(void)
{
	const static int index = 8;
	CallVFunc<void>(index, this);
}

//-----------------------------------------------------------------------------
// This is called when a game ends (server disconnect, death, restart, load)
// NOT on level transitions within a game
//-----------------------------------------------------------------------------
void CServerGameDLL::GameShutdown(void)
{
	// Game just calls a nullsub for GameShutdown lol.
	const static int index = 9;
	CallVFunc<void>(index, this);
}

//-----------------------------------------------------------------------------
// Purpose: Gets the simulation tick interval
// Output : float
//-----------------------------------------------------------------------------
float CServerGameDLL::GetTickInterval(void)
{
	const static int index = 11;
	return CallVFunc<float>(index, this);
}

//-----------------------------------------------------------------------------
// Purpose: get all server classes
// Output : ServerClass*
//-----------------------------------------------------------------------------
ServerClass* CServerGameDLL::GetAllServerClasses(void)
{
	const static int index = 12;
	return CallVFunc<ServerClass*>(index, this);
}

static ConVar chat_debug("chat_debug", "0", FCVAR_RELEASE, "Enables chat-related debug printing.");

static ConVar sv_override_team_chat_restriction("sv_override_team_chat_restriction", "0", FCVAR_RELEASE, 
	"When enabled this allows sv_forceChatToTeamOnly to take control of the team chat restriction.", 
	"0: Default, 1: Forces the value from sv_forceChatToTeamOnly."
);

void __fastcall CServerGameDLL::OnReceivedSayTextMessage(void* thisptr, int senderId, const char* text, bool isTeamChat)
{
	const CGlobalVars* globals = *g_pGlobals;
	const bool bTeamChatOnly = sv_override_team_chat_restriction.GetBool() ? sv_forceChatToTeamOnly->GetBool() : isTeamChat;

	if (senderId > globals->m_nMaxPlayers || senderId < 1)
		return;

	CPlayer* pSenderPlayer = reinterpret_cast<CPlayer*>(globals->m_pEdicts[senderId + 30728]);

	if (!pSenderPlayer || !pSenderPlayer->IsConnected())
		return;

	pSenderPlayer->UpdateLastActiveTime(globals->m_flCurTime);

	for (auto& cb : !g_PluginSystem.GetChatMessageCallbacks())
	{
		if (!cb.Function()(pSenderPlayer, text, bTeamChatOnly))
		{
			if (chat_debug.GetBool())
			{
				char moduleName[MAX_PATH] = {};

				V_UnicodeToUTF8(V_UnqualifiedFileName(cb.ModuleName()), moduleName, MAX_PATH);

				Msg(eDLL_T::SERVER, "[%s] Plugin blocked chat message from '%s' (%llu): \"%s\"\n", moduleName, pSenderPlayer->GetNetName(), pSenderPlayer->GetPlatformUserId(), text);
			}

			return;
		}
	}

	bool bSenderDeadAndCanOnlyTalkToDead = hudchat_dead_can_only_talk_to_other_dead->GetBool() && pSenderPlayer->GetLifeState();

	for (int nRecipientID = 1; nRecipientID <= globals->m_nMaxPlayers; nRecipientID++)
	{
		const CPlayer* pRecipientPlayer = reinterpret_cast<const CPlayer*>(globals->m_pEdicts[nRecipientID + 30728]);

		if (!pRecipientPlayer)
			continue;

		if (!pRecipientPlayer->IsConnected())
			continue;

		//If we are only allowed to talk to the dead make sure the recipient is dead
		if (bSenderDeadAndCanOnlyTalkToDead && !pRecipientPlayer->GetLifeState())
			continue;

		if (pRecipientPlayer != pSenderPlayer &&
			//If the chat is limited to one team we must check the sender and recipient are on the same team
			bTeamChatOnly && pSenderPlayer->GetTeamNum() != pRecipientPlayer->GetTeamNum()
		)
			continue;

		CSingleUserRecipientFilter filter(pRecipientPlayer);
		filter.MakeReliable();

		v_UserMessageBegin(&filter, "SayText", 2);

		MessageWriteByte(pSenderPlayer->GetEdict());
		MessageWriteString(text);
		MessageWriteBool(bTeamChatOnly);

		MessageEnd();
	}
	
}

void DrawServerHitbox(int iEntity)
{
	IHandleEntity* pEntity = LookupEntityByIndex(iEntity);
	CBaseAnimating* pAnimating = dynamic_cast<CBaseAnimating*>(pEntity);

	if (pAnimating)
	{
		pAnimating->DrawServerHitboxes();
	}
}

void DrawServerHitboxes(bool bRunOverlays)
{
	int nVal = sv_showhitboxes->GetInt();
	Assert(nVal < NUM_ENT_ENTRIES);

	if (nVal == -1)
		return;

	if (nVal == 0)
	{
		for (int i = 0; i < NUM_ENT_ENTRIES; i++)
		{
			DrawServerHitbox(i);
		}
	}
	else // Lookup entity manually by index from 'sv_showhitboxes'.
	{
		DrawServerHitbox(nVal);
	}
}

void CServerGameClients::ProcessUserCmds(CServerGameClients* thisp, edict_t edict,
	bf_read* buf, int numCmds, int totalCmds, int droppedPackets, bool ignore, bool paused)
{
	int i;
	CUserCmd* from, * to;

	// We track last three command in case we drop some
	// packets but get them back.
	CUserCmd cmds[MAX_BACKUP_COMMANDS_PROCESS];
	CUserCmd cmdNull;  // For delta compression

	Assert(numCmds >= 0);
	Assert((totalCmds - numCmds) >= 0);

	CPlayer* pPlayer = UTIL_PlayerByIndex(edict);

	// Too many commands?
	if (totalCmds < 0 || totalCmds >= (MAX_BACKUP_COMMANDS_PROCESS - 1) ||
		numCmds < 0 || numCmds > totalCmds)
	{
		CClient* pClient = g_pServer->GetClient(edict-1);

		Warning(eDLL_T::SERVER, "%s: Player '%s' sent too many cmds (%i)\n", __FUNCTION__, pClient->GetServerName(), totalCmds);
		buf->SetOverflowFlag();

		return;
	}

	from = &cmdNull;
	for (i = totalCmds - 1; i >= 0; i--)
	{
		to = &cmds[i];
		ReadUserCmd(buf, to, from);
		from = to;
	}

	// Client not fully connected or server has gone inactive or is paused, just ignore
	if (ignore || !pPlayer)
	{
		return;
	}

	pPlayer->ProcessUserCmds(cmds, numCmds, totalCmds, droppedPackets, paused);
}

void RunFrameServer(double flFrameTime, bool bRunOverlays, bool bUniformUpdate)
{
	DrawServerHitboxes(bRunOverlays);
	v_RunFrameServer(flFrameTime, bRunOverlays, bUniformUpdate);
}

void MessageEnd( void )
{
	Assert(*g_ppMessageBuffer);

	g_pEngineServer->MessageEnd();

	(*g_ppMessageBuffer) = nullptr;
}

void MessageWriteByte(int iValue)
{
	if (!*g_ppMessageBuffer)
		Error(eDLL_T::ENGINE, EXIT_FAILURE, "WRITE_BYTE called with no active message\n");
	
	(*g_ppMessageBuffer)->WriteByte(iValue);
}

void MessageWriteString(const char* pszString)
{
	if (!*g_ppMessageBuffer)
		Error(eDLL_T::ENGINE, EXIT_FAILURE, "WriteString called with no active message\n");
	
	(*g_ppMessageBuffer)->WriteString(pszString);
}

void MessageWriteBool(bool bValue)
{
	if (!*g_ppMessageBuffer)
		Error(eDLL_T::ENGINE, EXIT_FAILURE, "WriteBool called with no active message\n");

	(*g_ppMessageBuffer)->WriteOneBit(bValue ? 1 : 0);
}

void VServerGameDLL::Detour(const bool bAttach) const
{
	DetourSetup(&CServerGameDLL__OnReceivedSayTextMessage, &CServerGameDLL::OnReceivedSayTextMessage, bAttach);
	DetourSetup(&CServerGameClients__ProcessUserCmds, CServerGameClients::ProcessUserCmds, bAttach);
	DetourSetup(&v_RunFrameServer, &RunFrameServer, bAttach);
}

CServerGameDLL* g_pServerGameDLL = nullptr;
CServerGameClients* g_pServerGameClients = nullptr;
CServerGameEnts* g_pServerGameEntities = nullptr;

// Holds global variables shared between engine and game.
CGlobalVars** g_pGlobals = nullptr;
