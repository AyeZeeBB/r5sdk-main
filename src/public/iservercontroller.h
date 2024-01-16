#pragma once

class IServerController
{
public:
	virtual void SendCommand(const char* pText) const = 0;
	virtual void Reload() const = 0;
};
