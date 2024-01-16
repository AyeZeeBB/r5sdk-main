#include "core/stdafx.h"
#include "tier1/interface.h"
#include "iservercontroller.h"
#include "engine/server/servercontroller.h"
#include "engine/host_state.h"
#include "engine/server/server.h"
#include "engine/cmd.h"


void CServerController::SendCommand(const char* pText) const
{
	if (VALID_CHARSTAR(pText))
		Cbuf_AddText(Cbuf_GetCurrentPlayer(), pText, cmd_source_t::kCommandSrcCode);
};

void CServerController::Reload() const
{
	if (g_pServer->IsActive())
		v_HostState_ChangeLevelMP(g_pServer->GetMapName(), nullptr);
}

inline CServerController* g_ServerController = new CServerController();