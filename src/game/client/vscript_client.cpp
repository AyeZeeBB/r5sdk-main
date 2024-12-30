//=============================================================================//
//
// Purpose: Expose native code to VScript API
//
//-----------------------------------------------------------------------------
//
// See 'game/shared/vscript_shared.cpp' for more details.
//
//=============================================================================//

#include "core/stdafx.h"
#include "tier1/keyvalues.h"
#include "engine/cmodel_bsp.h"
#include "engine/host_state.h"
#include "engine/client/cl_main.h"
#include "networksystem/pylon.h"
#include "networksystem/listmanager.h"
#include "networksystem/hostmanager.h"
#include "game/shared/vscript_shared.h"

#include "vscript/vscript.h"
#include "vscript/languages/squirrel_re/include/sqvm.h"

#include "vscript_client.h"
#include <zip/src/ZipFile.h>
#include "tier0/frametask.h"

/*
=====================
SQVM_ClientScript_f

  Executes input on the
  VM in CLIENT context.
=====================
*/
static void SQVM_ClientScript_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
	{
		Script_Execute(args.ArgS(), SQCONTEXT::CLIENT);
	}
}

/*
=====================
SQVM_UIScript_f

  Executes input on the
  VM in UI context.
=====================
*/
static void SQVM_UIScript_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
	{
		Script_Execute(args.ArgS(), SQCONTEXT::UI);
	}
}

static ConCommand script_client("script_client", SQVM_ClientScript_f, "Run input code as CLIENT script on the VM", FCVAR_DEVELOPMENTONLY | FCVAR_CLIENTDLL | FCVAR_CHEAT);
static ConCommand script_ui("script_ui", SQVM_UIScript_f, "Run input code as UI script on the VM", FCVAR_DEVELOPMENTONLY | FCVAR_CLIENTDLL | FCVAR_CHEAT);

//-----------------------------------------------------------------------------
// Purpose: checks if the server index is valid, raises an error if not
//-----------------------------------------------------------------------------
static SQBool Script_CheckServerIndexAndFailure(HSQUIRRELVM v, SQInteger iServer)
{
	SQInteger iCount = static_cast<SQInteger>(g_ServerListManager.m_vServerList.size());

	if (iServer >= iCount)
	{
		v_SQVM_RaiseError(v, "Index must be less than %i.\n", iCount);
		return false;
	}
	else if (iServer == -1) // If its still -1, then 'sq_getinteger' failed
	{
		v_SQVM_RaiseError(v, "Invalid argument type provided.\n");
		return false;
	}

	return true;
}

namespace VScriptCode
{
	namespace Client
	{
		//-----------------------------------------------------------------------------
		// Purpose: checks whether this SDK build is a client dll
		//-----------------------------------------------------------------------------
		SQRESULT IsClientDLL(HSQUIRRELVM v)
		{
			sq_pushbool(v, ::IsClientDLL());
			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}
	}
	namespace Ui
	{
		//-----------------------------------------------------------------------------
		// Purpose: refreshes the server list
		//-----------------------------------------------------------------------------
		SQRESULT RefreshServerList(HSQUIRRELVM v)
		{
			string serverMessage; // Refresh list.
			size_t iCount;

			// TODO: return error string on failure?
			g_ServerListManager.RefreshServerList(serverMessage, iCount);
			sq_pushinteger(v, static_cast<SQInteger>(iCount));

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get current server count from pylon
		//-----------------------------------------------------------------------------
		SQRESULT GetServerCount(HSQUIRRELVM v)
		{
			size_t iCount = g_ServerListManager.m_vServerList.size();
			sq_pushinteger(v, static_cast<SQInteger>(iCount));

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get response from private server request
		//-----------------------------------------------------------------------------
		SQRESULT GetHiddenServerName(HSQUIRRELVM v)
		{
			const SQChar* privateToken = nullptr;

			if (SQ_FAILED(sq_getstring(v, 2, &privateToken)) || VALID_CHARSTAR(privateToken))
			{
				v_SQVM_ScriptError("Empty or null private token");
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);
			}

			string hiddenServerRequestMessage;
			NetGameServer_t serverListing;

			bool result = g_MasterServer.GetServerByToken(serverListing, hiddenServerRequestMessage, privateToken); // Send token connect request.
			if (!result)
			{
				if (hiddenServerRequestMessage.empty())
					sq_pushstring(v, "Request failed", -1);
				else
				{
					hiddenServerRequestMessage = Format("Request failed: %s", hiddenServerRequestMessage.c_str());
					sq_pushstring(v, hiddenServerRequestMessage.c_str(), -1);
				}

				SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
			}

			if (serverListing.name.empty())
			{
				if (hiddenServerRequestMessage.empty())
					hiddenServerRequestMessage = Format("Server listing empty");
				else
					hiddenServerRequestMessage = Format("Server listing empty: %s", hiddenServerRequestMessage.c_str());

				sq_pushstring(v, hiddenServerRequestMessage.c_str(), -1);
			}
			else
			{
				hiddenServerRequestMessage = Format("Found server: %s", serverListing.name.c_str());
				sq_pushstring(v, hiddenServerRequestMessage.c_str(), -1);
			}

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get server's current name from serverlist index
		//-----------------------------------------------------------------------------
		SQRESULT GetServerName(HSQUIRRELVM v)
		{
			AUTO_LOCK(g_ServerListManager.m_Mutex);

			SQInteger iServer = -1;
			sq_getinteger(v, 2, &iServer);

			if (!Script_CheckServerIndexAndFailure(v, iServer))
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);

			const string& serverName = g_ServerListManager.m_vServerList[iServer].name;
			sq_pushstring(v, serverName.c_str(), -1);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get server's current description from serverlist index
		//-----------------------------------------------------------------------------
		SQRESULT GetServerDescription(HSQUIRRELVM v)
		{
			AUTO_LOCK(g_ServerListManager.m_Mutex);

			SQInteger iServer = -1;
			sq_getinteger(v, 2, &iServer);

			if (!Script_CheckServerIndexAndFailure(v, iServer))
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);

			const string& serverDescription = g_ServerListManager.m_vServerList[iServer].description;
			sq_pushstring(v, serverDescription.c_str(), -1);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get server's current map via serverlist index
		//-----------------------------------------------------------------------------
		SQRESULT GetServerMap(HSQUIRRELVM v)
		{
			AUTO_LOCK(g_ServerListManager.m_Mutex);

			SQInteger iServer = -1;
			sq_getinteger(v, 2, &iServer);

			if (!Script_CheckServerIndexAndFailure(v, iServer))
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);

			const string& svServerMapName = g_ServerListManager.m_vServerList[iServer].map;
			sq_pushstring(v, svServerMapName.c_str(), -1);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get server's current playlist via serverlist index
		//-----------------------------------------------------------------------------
		SQRESULT GetServerPlaylist(HSQUIRRELVM v)
		{
			AUTO_LOCK(g_ServerListManager.m_Mutex);

			SQInteger iServer = -1;
			sq_getinteger(v, 2, &iServer);

			if (!Script_CheckServerIndexAndFailure(v, iServer))
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);

			const string& serverPlaylist = g_ServerListManager.m_vServerList[iServer].playlist;
			sq_pushstring(v, serverPlaylist.c_str(), -1);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get server's current player count via serverlist index
		//-----------------------------------------------------------------------------
		SQRESULT GetServerCurrentPlayers(HSQUIRRELVM v)
		{
			AUTO_LOCK(g_ServerListManager.m_Mutex);

			SQInteger iServer = -1;
			sq_getinteger(v, 2, &iServer);

			if (!Script_CheckServerIndexAndFailure(v, iServer))
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);

			const SQInteger playerCount = g_ServerListManager.m_vServerList[iServer].numPlayers;
			sq_pushinteger(v, playerCount);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get server's current player count via serverlist index
		//-----------------------------------------------------------------------------
		SQRESULT GetServerMaxPlayers(HSQUIRRELVM v)
		{
			AUTO_LOCK(g_ServerListManager.m_Mutex);

			SQInteger iServer = -1;
			sq_getinteger(v, 2, &iServer);

			if (!Script_CheckServerIndexAndFailure(v, iServer))
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);

			const SQInteger maxPlayers = g_ServerListManager.m_vServerList[iServer].maxPlayers;
			sq_pushinteger(v, maxPlayers);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: get promo data for serverbrowser panels
		//-----------------------------------------------------------------------------
		SQRESULT GetPromoData(HSQUIRRELVM v)
		{
			enum class R5RPromoData : SQInteger
			{
				PromoLargeTitle,
				PromoLargeDesc,
				PromoLeftTitle,
				PromoLeftDesc,
				PromoRightTitle,
				PromoRightDesc
			};

			SQInteger idx = 0;
			sq_getinteger(v, 2, &idx);

			R5RPromoData ePromoIndex = static_cast<R5RPromoData>(idx);
			const char* pszPromoKey;

			switch (ePromoIndex)
			{
			case R5RPromoData::PromoLargeTitle:
			{
				pszPromoKey = "#PROMO_LARGE_TITLE";
				break;
			}
			case R5RPromoData::PromoLargeDesc:
			{
				pszPromoKey = "#PROMO_LARGE_DESCRIPTION";
				break;
			}
			case R5RPromoData::PromoLeftTitle:
			{
				pszPromoKey = "#PROMO_LEFT_TITLE";
				break;
			}
			case R5RPromoData::PromoLeftDesc:
			{
				pszPromoKey = "#PROMO_LEFT_DESCRIPTION";
				break;
			}
			case R5RPromoData::PromoRightTitle:
			{
				pszPromoKey = "#PROMO_RIGHT_TITLE";
				break;
			}
			case R5RPromoData::PromoRightDesc:
			{
				pszPromoKey = "#PROMO_RIGHT_DESCRIPTION";
				break;
			}
			default:
			{
				pszPromoKey = "#PROMO_SDK_ERROR";
				break;
			}
			}

			sq_pushstring(v, pszPromoKey, -1);
			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		SQRESULT GetEULAContents(HSQUIRRELVM v)
		{
			MSEulaData_t eulaData;
			string eulaRequestMessage;

			if (g_MasterServer.GetEULA(eulaData, eulaRequestMessage))
			{
				// set EULA version cvar to the newly fetched EULA version
				eula_version->SetValue(eulaData.version);

				sq_pushstring(v, eulaData.contents.c_str(), -1);
			}
			else
			{
				string error = Format("Failed to load EULA Data: %s", eulaRequestMessage.c_str());

				Warning(eDLL_T::UI, "%s\n", error.c_str());
				sq_pushstring(v, error.c_str(), -1);
			}

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: connect to server from native server browser entries
		//-----------------------------------------------------------------------------
		SQRESULT ConnectToServer(HSQUIRRELVM v)
		{
			const SQChar* ipAddress = nullptr;
			if (SQ_FAILED(sq_getstring(v, 2, &ipAddress)))
			{
				v_SQVM_ScriptError("Missing ip address");
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);
			}

			const SQChar* cryptoKey = nullptr;
			if (SQ_FAILED(sq_getstring(v, 3, &cryptoKey)))
			{
				v_SQVM_ScriptError("Missing encryption key");
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);
			}

			Msg(eDLL_T::UI, "Connecting to server with ip address '%s' and encryption key '%s'\n", ipAddress, cryptoKey);
			g_ServerListManager.ConnectToServer(ipAddress, cryptoKey);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: set netchannel encryption key and connect to server
		//-----------------------------------------------------------------------------
		SQRESULT ConnectToListedServer(HSQUIRRELVM v)
		{
			AUTO_LOCK(g_ServerListManager.m_Mutex);

			SQInteger iServer = -1;
			sq_getinteger(v, 2, &iServer);

			if (!Script_CheckServerIndexAndFailure(v, iServer))
			{
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);
			}

			const NetGameServer_t& gameServer = g_ServerListManager.m_vServerList[iServer];

			g_ServerListManager.ConnectToServer(gameServer.address, gameServer.port,
				gameServer.netKey);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: request token from pylon and join server with result.
		//-----------------------------------------------------------------------------
		SQRESULT ConnectToHiddenServer(HSQUIRRELVM v)
		{
			const SQChar* privateToken = nullptr;
			const SQRESULT strRet = sq_getstring(v, 2, &privateToken);

			if (SQ_FAILED(strRet) || VALID_CHARSTAR(privateToken))
			{
				v_SQVM_ScriptError("Empty or null private token");
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);
			}

			string hiddenServerRequestMessage;
			NetGameServer_t netListing;

			const bool result = g_MasterServer.GetServerByToken(netListing, hiddenServerRequestMessage, privateToken); // Send token connect request.
			if (result)
			{
				g_ServerListManager.ConnectToServer(netListing.address, netListing.port, netListing.netKey);
			}
			else
			{
				Warning(eDLL_T::UI, "Failed to connect to private server: %s\n", hiddenServerRequestMessage.c_str());
			}

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: create server via native serverbrowser entries
		// TODO: return a boolean on failure instead of raising an error, so we could
		// determine from scripts whether or not to spin a local server, or connect
		// to a dedicated server (for disconnecting and loading the lobby, for example)
		//-----------------------------------------------------------------------------
		SQRESULT CreateServer(HSQUIRRELVM v)
		{
			const SQChar* serverName = nullptr;
			const SQChar* serverDescription = nullptr;
			const SQChar* serverMapName = nullptr;
			const SQChar* serverPlaylist = nullptr;

			sq_getstring(v, 2, &serverName);
			sq_getstring(v, 3, &serverDescription);
			sq_getstring(v, 4, &serverMapName);
			sq_getstring(v, 5, &serverPlaylist);

			SQInteger serverVisibility = 0;
			sq_getinteger(v, 6, &serverVisibility);

			if (!VALID_CHARSTAR(serverName) ||
				!VALID_CHARSTAR(serverMapName) ||
				!VALID_CHARSTAR(serverPlaylist))
			{
				v_SQVM_ScriptError("Empty or null server criteria");
				SCRIPT_CHECK_AND_RETURN(v, SQ_ERROR);
			}

			hostname->SetValue(serverName);
			hostdesc.SetValue(serverDescription);

			// Launch server.
			g_ServerHostManager.SetVisibility(ServerVisibility_e(serverVisibility));
			g_ServerHostManager.LaunchServer(serverMapName, serverPlaylist);

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		//-----------------------------------------------------------------------------
		// Purpose: shuts the server down and disconnects all clients
		//-----------------------------------------------------------------------------
		SQRESULT DestroyServer(HSQUIRRELVM v)
		{
			if (g_pHostState->m_bActiveGame)
				g_pHostState->m_iNextState = HostStates_t::HS_GAME_SHUTDOWN;

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		static size_t WriteToFile(void* buffer, size_t size, size_t nmemb, FILE* outputStream) {
			return fwrite(buffer, size, nmemb, outputStream);
		}

		static size_t WriteToString(void* buffer, size_t size, size_t nmemb, std::string* outputString) {
			outputString->append(static_cast<char*>(buffer), size * nmemb);
			return size * nmemb;
		}

		static bool RemoveDirectory(const std::string& directoryPath) {
			try {
				if (fs::exists(directoryPath)) {
					fs::remove_all(directoryPath);
				}
				return true;
			}
			catch (const std::exception& e) {
				Warning(eDLL_T::UI, "Error deleting directory: %s\n", e.what());
				return false;
			}
		}

		static void ReportScriptProgress(HSCRIPT progressCallback, const std::string& message, int percentage) {
			g_TaskQueue.Dispatch([progressCallback, message, percentage] {
				Assert(ThreadInMainThread());

				ScriptVariant_t args[2] = { percentage, message.c_str() };
				g_pUIScript->ExecuteFunction(progressCallback, args, SDK_ARRAYSIZE(args), nullptr, 0);
				}, 0);
		}

		static void ReportUpdateFailed(HSCRIPT failedCallback, const std::string& message) {
			g_TaskQueue.Dispatch([failedCallback, message] {
				Assert(ThreadInMainThread());

				ScriptVariant_t args[1] = { message.c_str() };
				g_pUIScript->ExecuteFunction(failedCallback, args, SDK_ARRAYSIZE(args), nullptr, 0);
				}, 0);
		}

		static bool ExtractZipFile(const std::string& zipFilePath, const std::string& outputDirectory, HSCRIPT progressCallback) {
			try {
				ZipArchive::Ptr archive = ZipFile::Open(zipFilePath);
				if (!archive) {
					Warning(eDLL_T::UI, "[Script Update] Failed to open ZIP file: %s\n", zipFilePath.c_str());
					return false;
				}

				size_t totalEntries = archive->GetEntriesCount();

				for (size_t i = 0; i < totalEntries; ++i) {
					int progress = static_cast<int>((i + 1) * 100 / totalEntries);
					ReportScriptProgress(progressCallback, Format("Extracting %zu of %zu", i + 1, totalEntries), progress);

					ZipArchiveEntry::Ptr entry = archive->GetEntry(static_cast<int>(i));
					if (!entry) {
						Warning(eDLL_T::UI, "[Script Update] Error reading entry from ZIP archive\n");
						continue;
					}

					std::string fullPath = outputDirectory + "/" + entry->GetFullName();
					if (entry->IsDirectory()) {
						fs::create_directories(fullPath);
					}
					else {
						std::istream* decompressionStream = entry->GetDecompressionStream();
						if (!decompressionStream) {
							Warning(eDLL_T::UI, "[Script Update] Failed to open decompression stream for: %s\n", entry->GetFullName().c_str());
							return false;
						}

						std::ofstream outputFile(fullPath, std::ios::binary);
						if (!outputFile) {
							Warning(eDLL_T::UI, "[Script Update] Failed to create file: %s\n", fullPath.c_str());
							return false;
						}

						char buffer[4096];
						while (!decompressionStream->eof()) {
							decompressionStream->read(buffer, sizeof(buffer));
							outputFile.write(buffer, decompressionStream->gcount());
						}
					}
				}
			}
			catch (const std::exception& e) {
				Warning(eDLL_T::UI, "[Script Update] Error extracting ZIP file: %s\n", e.what());
				return false;
			}
			return true;
		}

		static bool FetchScriptVersion(CUtlString& versionString) {
			CURL* curl = curl_easy_init();
			if (!curl) {
				Warning(eDLL_T::UI, "[Script Update] Failed to initialize CURL\n");
				versionString = "";
				return false;
			}

			curl_easy_setopt(curl, CURLOPT_URL, "https://s3.r5reloaded.com/scripts_live/version.txt");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &versionString);

			CURLcode result = curl_easy_perform(curl);
			if (result != CURLE_OK) {
				Warning(eDLL_T::UI, "[Script Update] Error fetching script version: %s\n", curl_easy_strerror(result));
				curl_easy_cleanup(curl);
				versionString = "";
				return false;
			}

			curl_easy_cleanup(curl);
			return true;
		}

		static int TransferProgressCallback(void* callbackPtr, curl_off_t totalDownload, curl_off_t downloaded, curl_off_t, curl_off_t) {
			if (totalDownload > 0) {
				int progress = static_cast<int>((downloaded / static_cast<double>(totalDownload)) * 100);
				ReportScriptProgress(static_cast<HSCRIPT>(callbackPtr), Format("Downloaded: %lld / %lld bytes", downloaded, totalDownload), progress);
			}
			return 0;
		}

		static void PerformScriptUpdate() {
			CUtlString scriptVersion;
			const HSCRIPT onUpdateProgress = g_pUIScript->FindFunction("onUpdateProgress", "void functionref(int, string)", nullptr);
			const HSCRIPT onUpdateComplete = g_pUIScript->FindFunction("onUpdateComplete", "void functionref(bool)", nullptr);
			const HSCRIPT onUpdateFailed = g_pUIScript->FindFunction("onUpdateFailed", "void functionref(string)", nullptr);

			if (onUpdateProgress == nullptr) {
				Warning(eDLL_T::UI, "[Script Update] Failed to find UI function (onUpdateProgress)\n");
				return;
			}

			if (onUpdateComplete == nullptr) {
				Warning(eDLL_T::UI, "[Script Update] Failed to find UI function (onUpdateComplete)\n");
				return;
			}

			if (onUpdateFailed == nullptr) {
				Warning(eDLL_T::UI, "[Script Update] Failed to find UI function (onUpdateFailed)\n");
				return;
			}

			if (!FetchScriptVersion(scriptVersion))
				return;

			std::string scriptUrl = Format("https://s3.r5reloaded.com/scripts_live/scripts_%s.zip", scriptVersion.String());

			CURL* curl = curl_easy_init();
			if (!curl) {
				Warning(eDLL_T::UI, "[Script Update] Failed to initialize CURL\n");
				ReportUpdateFailed(onUpdateFailed, "Failed to initialize CURL");
				return;
			}

			FILE* file = fopen("scripts.zip", "wb");
			if (!file) {
				Warning(eDLL_T::UI, "[Script Update] Error opening file for writing\n");
				ReportUpdateFailed(onUpdateFailed, "Error opening file for writing");
				curl_easy_cleanup(curl);
				return;
			}

			curl_easy_setopt(curl, CURLOPT_URL, scriptUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToFile);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, TransferProgressCallback);
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, onUpdateProgress);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

			CURLcode result = curl_easy_perform(curl);
			fclose(file);
			curl_easy_cleanup(curl);

			if (result != CURLE_OK) {
				Warning(eDLL_T::UI, "[Script Update] Error downloading script package: %s\n", curl_easy_strerror(result));
				ReportUpdateFailed(onUpdateFailed, "Error downloading script package");
				return;
			}

			ReportScriptProgress(onUpdateProgress, "", 0);

			if (!RemoveDirectory("platform/scripts")) {
				Warning(eDLL_T::UI, "[Script Update] Failed to remove target directory\n");
				ReportUpdateFailed(onUpdateFailed, "Failed to remove target directory");
				return;
			}

			if (!ExtractZipFile("scripts.zip", "platform/", onUpdateProgress)) {
				Warning(eDLL_T::UI, "[Script Update] Failed to extract script package\n");
				ReportUpdateFailed(onUpdateFailed, "Failed to extract script package");
				return;
			}

			fs::remove("scripts.zip");

			g_TaskQueue.Dispatch([onUpdateComplete] {
				Assert(ThreadInMainThread());

				ScriptVariant_t args[1] = { true };
				g_pUIScript->ExecuteFunction(onUpdateComplete, args, SDK_ARRAYSIZE(args), nullptr, 0);
				}, 0);
		}

		static void NotifyScriptVersion(HSCRIPT callbackFunction, CUtlString& version, bool success) {
			g_TaskQueue.Dispatch([callbackFunction, version, success] {
				Assert(ThreadInMainThread());

				ScriptVariant_t args[2] = { version.String(), success };
				g_pUIScript->ExecuteFunction(callbackFunction, args, SDK_ARRAYSIZE(args), nullptr, 0);
				}, 0);
		}

		static void CheckForScriptUpdates() {
			CUtlString scriptVersion;
			const HSCRIPT onUpdateCheck = g_pUIScript->FindFunction("onUpdateCheck", "void functionref(string, bool)", nullptr);

			if (onUpdateCheck == nullptr) {
				Warning(eDLL_T::UI, "[Script Update] Failed to find script update check function (onUpdateCheck)\n");
				return;
			}

			bool success = FetchScriptVersion(scriptVersion);
			NotifyScriptVersion(onUpdateCheck, scriptVersion, success);
		}

		SQRESULT StartScriptUpdate(HSQUIRRELVM v) {
			std::thread(PerformScriptUpdate).detach();
			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}

		SQRESULT StartScriptCheck(HSQUIRRELVM v) {
			std::thread(CheckForScriptUpdates).detach();

			SCRIPT_CHECK_AND_RETURN(v, SQ_OK);
		}
	}
}

//---------------------------------------------------------------------------------
// Purpose: registers script functions in CLIENT context
// Input  : *s -
//---------------------------------------------------------------------------------
void Script_RegisterClientFunctions(CSquirrelVM* s)
{
	Script_RegisterCommonAbstractions(s);
	Script_RegisterCoreClientFunctions(s);
}

//---------------------------------------------------------------------------------
// Purpose: core client script functions
// Input  : *s -
//---------------------------------------------------------------------------------
void Script_RegisterCoreClientFunctions(CSquirrelVM* s)
{
	DEFINE_CLIENT_SCRIPTFUNC_NAMED(s, IsClientDLL, "Returns whether this build is client only", "bool", "");
}

//---------------------------------------------------------------------------------
// Purpose: registers script functions in UI context
// Input  : *s -
//---------------------------------------------------------------------------------
void Script_RegisterUIFunctions(CSquirrelVM* s)
{
	Script_RegisterCommonAbstractions(s);
	Script_RegisterCoreClientFunctions(s);

	DEFINE_UI_SCRIPTFUNC_NAMED(s, StartScriptUpdate, "", "bool", "");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, StartScriptCheck, "", "string", "");

	DEFINE_UI_SCRIPTFUNC_NAMED(s, RefreshServerList, "Refreshes the public server list and returns the count", "int", "");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetServerCount, "Gets the number of public servers", "int", "");

	// Functions for retrieving server browser data
	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetHiddenServerName, "Gets hidden server name by token", "string", "string");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetServerName, "Gets the name of the server at the specified index of the server list", "string", "int");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetServerDescription, "Gets the description of the server at the specified index of the server list", "string", "int");

	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetServerMap, "Gets the map of the server at the specified index of the server list", "string", "int");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetServerPlaylist, "Gets the playlist of the server at the specified index of the server list", "string", "int");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetServerCurrentPlayers, "Gets the current player count of the server at the specified index of the server list", "int", "int");

	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetServerMaxPlayers, "Gets the max player count of the server at the specified index of the server list", "int", "int");

	// Misc main menu functions
	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetPromoData, "Gets promo data for specified slot type", "string", "int");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, GetEULAContents, "Gets EULA contents from masterserver", "string", "");

	// Functions for connecting to servers
	DEFINE_UI_SCRIPTFUNC_NAMED(s, ConnectToServer, "Joins server by ip address and encryption key", "void", "string, string");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, ConnectToListedServer, "Joins listed server by index", "void", "int");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, ConnectToHiddenServer, "Joins hidden server by token", "void", "string");
}

void Script_RegisterUIServerFunctions(CSquirrelVM* s)
{
	DEFINE_UI_SCRIPTFUNC_NAMED(s, CreateServer, "Starts server with the specified settings", "void", "string, string, string, string, int");
	DEFINE_UI_SCRIPTFUNC_NAMED(s, DestroyServer, "Shuts the local server down", "void", "");
}

//---------------------------------------------------------------------------------
// Purpose: console variables for scripts, these should not be used in engine/sdk code !!!
//---------------------------------------------------------------------------------
static ConVar settings_reflex("settings_reflex", "1", FCVAR_RELEASE, "Selected NVIDIA Reflex mode.", "0 = Off. 1 = On. 2 = On + Boost.");
static ConVar settings_antilag("settings_antilag", "1", FCVAR_RELEASE, "Selected AMD Anti-Lag mode.", "0 = Off. 1 = On.");

static ConVar serverbrowser_hideEmptyServers("serverbrowser_hideEmptyServers", "0", FCVAR_RELEASE, "Hide empty servers in the server browser.");
static ConVar serverbrowser_mapFilter("serverbrowser_mapFilter", "0", FCVAR_RELEASE, "Filter servers by map in the server browser.");
static ConVar serverbrowser_gamemodeFilter("serverbrowser_gamemodeFilter", "0", FCVAR_RELEASE, "Filter servers by gamemode in the server browser.");

// NOTE: if we want to make a certain promo only show once, add the playerprofile flag to the cvar below. Current behavior = always show after game restart.
static ConVar promo_version_accepted("promo_version_accepted", "0", FCVAR_RELEASE, "The accepted promo version.");

static ConVar player_setting_damage_closes_deathbox_menu("player_setting_damage_closes_deathbox_menu", "1", FCVAR_ARCHIVE | FCVAR_RELEASE, "Controls whether death box automatically closes when taking damage (used for menus).");
static ConVar show_motd_on_server_first_join("show_motd_on_server_first_join", "0", FCVAR_ARCHIVE | FCVAR_RELEASE, "Controls whether or not the server message of the day shows on first join for that server.");

//---------------------------------------------------------------------------------
// Purpose: script code class function registration
//---------------------------------------------------------------------------------
static void Script_RegisterClientEntityClassFuncs()
{
	v_Script_RegisterClientEntityClassFuncs();
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
}
//---------------------------------------------------------------------------------
static void Script_RegisterClientPlayerClassFuncs()
{
	v_Script_RegisterClientPlayerClassFuncs();
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
}
//---------------------------------------------------------------------------------
static void Script_RegisterClientAIClassFuncs()
{
	v_Script_RegisterClientAIClassFuncs();
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
}
//---------------------------------------------------------------------------------
static void Script_RegisterClientWeaponClassFuncs()
{
	v_Script_RegisterClientWeaponClassFuncs();
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
}
//---------------------------------------------------------------------------------
static void Script_RegisterClientProjectileClassFuncs()
{
	v_Script_RegisterClientProjectileClassFuncs();
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
}
//---------------------------------------------------------------------------------
static void Script_RegisterClientTitanSoulClassFuncs()
{
	v_Script_RegisterClientTitanSoulClassFuncs();
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
}
//---------------------------------------------------------------------------------
static void Script_RegisterClientPlayerDecoyClassFuncs()
{
	v_Script_RegisterClientPlayerDecoyClassFuncs();
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
}
//---------------------------------------------------------------------------------
static void Script_RegisterClientFirstPersonProxyClassFuncs()
{
	v_Script_RegisterClientFirstPersonProxyClassFuncs();
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
}

void VScriptClient::Detour(const bool bAttach) const
{
	DetourSetup(&v_Script_RegisterClientEntityClassFuncs, &Script_RegisterClientEntityClassFuncs, bAttach);
	DetourSetup(&v_Script_RegisterClientPlayerClassFuncs, &Script_RegisterClientPlayerClassFuncs, bAttach);
	DetourSetup(&v_Script_RegisterClientAIClassFuncs, &Script_RegisterClientAIClassFuncs, bAttach);
	DetourSetup(&v_Script_RegisterClientWeaponClassFuncs, &Script_RegisterClientWeaponClassFuncs, bAttach);
	DetourSetup(&v_Script_RegisterClientProjectileClassFuncs, &Script_RegisterClientProjectileClassFuncs, bAttach);
	DetourSetup(&v_Script_RegisterClientTitanSoulClassFuncs, &Script_RegisterClientTitanSoulClassFuncs, bAttach);
	DetourSetup(&v_Script_RegisterClientPlayerDecoyClassFuncs, &Script_RegisterClientPlayerDecoyClassFuncs, bAttach);
	DetourSetup(&v_Script_RegisterClientFirstPersonProxyClassFuncs, &Script_RegisterClientFirstPersonProxyClassFuncs, bAttach);
}