#pragma once

class IServerController
{
public:
	virtual void SendCommand(const char* pText) const = 0;
	virtual void Reload() const = 0;
	virtual void StartGame(const char* pGamemode, const char* pMap) const = 0;
};
