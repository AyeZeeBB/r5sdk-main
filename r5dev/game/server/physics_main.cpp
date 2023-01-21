//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: Physics simulation for non-havok/ipion objects
//
// $NoKeywords: $
//=============================================================================//
#include "core/stdafx.h"
#include "tier1/cvar.h"
#include "player.h"
#include "physics_main.h"
#include "engine/client/client.h"
#include "game/shared/util_shared.h"

//-----------------------------------------------------------------------------
// Purpose: Runs the command simulation for fake players
//-----------------------------------------------------------------------------
void Physics_RunBotSimulation(bool bSimulating)
{
	if (!sv_simulateBots->GetBool())
		return;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CClient* pClient = g_pClient->GetClient(i);
		if (!pClient)
			continue;

		if (pClient->IsActive() && pClient->IsFakeClient())
		{
			CPlayer* pPlayer = UTIL_PlayerByIndex(i + 1);
			if (pPlayer)
				pPlayer->RunNullCommand();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Runs the main physics simulation loop against all entities ( except players )
//-----------------------------------------------------------------------------
void* Physics_RunThinkFunctions(bool bSimulating)
{
	Physics_RunBotSimulation(bSimulating);
	return v_Physics_RunThinkFunctions(bSimulating);
}

///////////////////////////////////////////////////////////////////////////////
void Physics_Main_Attach()
{
	DetourAttach(&v_Physics_RunThinkFunctions, &Physics_RunThinkFunctions);
}

void Physics_Main_Detach()
{
	DetourDetach(&v_Physics_RunThinkFunctions, &Physics_RunThinkFunctions);
}