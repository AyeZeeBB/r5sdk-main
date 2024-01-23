#pragma once
#include "iservercontroller.h"

#define SERVER_CONTROLLER_INTERFACE_VERSION "ServerController001"

class CServerController : IServerController
{
public:
	virtual void SendCommand(const char* pText) const;
	virtual void Reload() const;
	virtual void StartGame(const char* pGamemode, const char* pMap) const;
};

extern CServerController* g_ServerController;

FORCEINLINE CServerController* ServerController()
{
	return g_ServerController;
}