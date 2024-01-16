//=============================================================================//
//
// Purpose: plugin sdk that makes plugins run!
// 
//-----------------------------------------------------------------------------
//
//=============================================================================//

#include "core/stdafx.h"
#include "ifactory.h"
#include "iservercontroller.h"
#include "engine/server/servercontroller.h"
#include "pluginsystem/ipluginsystem.h"
#include "engine/host_state.h"
#include "pluginsdk.h"
#include "protc/servercontroller.pb.h"
#include "tier0/commandline.h"
#include "communication/messagehandler.h"
#include "pluginsdk.h"

inline STARTUPINFO startupInfo = { 0 };
inline IServerController* g_pServerController = nullptr;
inline CMessage* g_pMessageLink = nullptr;

//---------------------------------------------------------------------------------
// Purpose: constructor
// Input  : pszSelfModule - 
//---------------------------------------------------------------------------------
CPluginSDK::CPluginSDK(const char* pszSelfModule) : m_FactoryInstance(nullptr), m_PluginSystem(nullptr)
{
	m_SelfModule.InitFromName(pszSelfModule);
	m_GameModule.InitFromBase(CModule::GetProcessEnvironmentBlock()->ImageBaseAddress);
}

//---------------------------------------------------------------------------------
// Purpose: destructor
//---------------------------------------------------------------------------------
CPluginSDK::~CPluginSDK()
{
}

void CPluginSDK::PipeThreadReader(CPluginSDK* thisp)
{
	char buff[4096];
	DWORD bytesRead = 0;

	for (;;)
	{
		BOOL bSuccess = ReadFile(startupInfo.hStdInput, buff, sizeof(buff) - 1, &bytesRead, NULL);
		if (!bSuccess || !bytesRead)
			break;

		buff[bytesRead] = '\0';
		thisp->m_ServerController->SendCommand(buff);
	}
}

void OnHostStateChanged(HostStates_t newState, HostStates_t oldState)
{
	servercontroller::ServerUpdate update;
	update.set_type(servercontroller::ServerUpdate_MESSAGE_TYPE_HOST_STATE_CHANGE);
	uint16_t buffSize = static_cast<uint16_t>(update.ByteSize());

	char* buff = new char[buffSize];

	if (update.SerializeToArray(buff, buffSize))
	{
		g_pMessageLink->Tx(buff, buffSize);
	}

	delete[] buff;
}

void OnFatalScriptError(const char* contextName)
{
	servercontroller::ServerUpdate update;
	update.set_type(servercontroller::ServerUpdate_MESSAGE_TYPE_FATAL_SCRIPT_ERROR);

	uint16_t buffSize = static_cast<uint16_t>(update.ByteSize());

	char* buff = new char[buffSize];

	if (update.SerializeToArray(buff, buffSize))
	{
		g_pMessageLink->Tx(buff, buffSize);
	}

	delete[] buff;
}

void OnMainMsgRecieve(void* const pData, size_t nBytes, void* pUsr)
{
	servercontroller::ServerControlMessage command;

	if (command.ParseFromArray(pData, static_cast<int>(nBytes)))
	{
		switch (command.type())
		{
		case servercontroller::ServerControlMessage_MESSAGE_TYPE_RELOAD:
		{
			g_pServerController->Reload();
			break;
		}
		default:
			break;
		}
	}

	free(pData);
}

//---------------------------------------------------------------------------------
// Purpose: properly initialize the plugin sdk
//---------------------------------------------------------------------------------
bool CPluginSDK::InitSDK()
{
	InstantiateInterfaceFn factorySystem = m_SDKModule.GetExportedSymbol("GetFactorySystem").RCast<InstantiateInterfaceFn>();
	if (!factorySystem)
	{
		Assert(factorySystem, "factorySystem == NULL; symbol renamed???");
		return false;
	}

	m_FactoryInstance = (IFactorySystem*)factorySystem();

	// Let's make sure the factory version matches, else we unload.
	bool isFactoryVersionOk = strcmp(m_FactoryInstance->GetVersion(), FACTORY_INTERFACE_VERSION) == 0;
	if (!isFactoryVersionOk)
	{
		Assert(isFactoryVersionOk, "Version mismatch!");
		return false;
	}

	// Unload if 
	m_PluginSystem = (IPluginSystem*)m_FactoryInstance->GetFactory(INTERFACEVERSION_PLUGINSYSTEM);
	if (!m_PluginSystem)
	{
		Assert(m_PluginSystem, "CPluginSDK::m_PluginSystem == NULL");
		return false;
	}

	m_ServerController = (IServerController*)m_FactoryInstance->GetFactory(SERVER_CONTROLLER_INTERFACE_VERSION);
	if (!m_ServerController)
	{
		Assert(m_PluginSystem, "CPluginSDK::m_ServerController == NULL");
		return false;
	}

	m_CommandLine = (ICommandLine*)m_FactoryInstance->GetFactory("VCommmandLine");
	if (!m_CommandLine)
	{
		Assert(m_PluginSystem, "CPluginSDK::pCommandLine == NULL");
		return false;
	}

	RegisterCallbacks();

	startupInfo.cb = sizeof(startupInfo);
	GetStartupInfoA(&startupInfo);

	const char* commData = m_CommandLine->ParmValue("-commsData", nullptr);
	if (!commData)
		exit(EXIT_FAILURE);

	HANDLE comm = reinterpret_cast<HANDLE>(strtoull(commData, nullptr, 16));

	g_pMessageLink = new CMessage();
	g_pMessageLink->SetReadCallback(OnMainMsgRecieve, nullptr);
	g_pMessageLink->ConnectClient(comm);

	std::thread(PipeThreadReader, this).detach();

	return true;
}

void CPluginSDK::RegisterCallbacks() const 
{
	PluginHelpWithAnything_t callBackReg;
	callBackReg.m_nHelpID = PluginHelpWithAnything_t::ePluginHelp::PLUGIN_REGISTER_CALLBACK;
	callBackReg.m_nCallbackID = PluginHelpWithAnything_t::ePluginCallback::HostStateChanged;
	callBackReg.m_pszName = "HostStateChanged";
	callBackReg.m_pFunction = OnHostStateChanged;
	m_PluginSystem->HelpWithAnything(&callBackReg);

	PluginHelpWithAnything_t fatalScriptError;
	fatalScriptError.m_nHelpID = PluginHelpWithAnything_t::ePluginHelp::PLUGIN_REGISTER_CALLBACK;
	fatalScriptError.m_nCallbackID = PluginHelpWithAnything_t::ePluginCallback::FatalScriptErrorOccured;
	fatalScriptError.m_pszName = "OnFatalScriptError";
	fatalScriptError.m_pFunction = OnFatalScriptError;
	m_PluginSystem->HelpWithAnything(&fatalScriptError);
}

CPluginSDK* g_pPluginSDK = nullptr;