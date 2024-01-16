#pragma once

class IFactorySystem;
class IPluginSystem;
class IServerController;
class ICommandLine;
//-----------------------------------------------------------------------------//

class CPluginSDK
{
public:
	CPluginSDK(const char* pszSelfModule);
	~CPluginSDK();

	bool InitSDK();
	void RegisterCallbacks() const;

	static void PipeThreadReader(CPluginSDK* thisp);
	inline void SetSDKModule(const CModule& sdkModule) { m_SDKModule = sdkModule; };
private:

	IFactorySystem* m_FactoryInstance;
	IPluginSystem* m_PluginSystem;
	IServerController* m_ServerController;
	ICommandLine* m_CommandLine;
	CModule m_SelfModule;
	CModule m_GameModule;
	CModule m_SDKModule;
};

extern CPluginSDK* g_pPluginSDK;