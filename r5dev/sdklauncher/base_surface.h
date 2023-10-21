#pragma once
#include "advanced_surface.h"

class CBaseSurface : public Forms::Form
{
public:
	CBaseSurface();
	virtual ~CBaseSurface()
	{};

	void ToggleUpdateView(bool bValue);

protected:
	static void OnInstallClick(Forms::Control* Sender);
	static void OnUpdateClick(Forms::Control* Sender);
	static void OnLaunchClick(Forms::Control* Sender);

	static void OnAdvancedClick(Forms::Control* Sender);


	static void OnSupportClick(Forms::Control* Sender);
	static void OnJoinClick(Forms::Control* Sender);
	static void OnYouTubeClick(Forms::Control* Sender);

private:
	void Frame();

	enum class eMode
	{
		NONE = -1,
		HOST,
		SERVER,
		CLIENT,
	};
	enum class eVisibility
	{
		PUBLIC,
		HIDDEN,
	};

	// Game.
	UIX::UIXGroupBox* m_BaseGroup;

	UIX::UIXButton* m_ManageButton;
	UIX::UIXButton* m_RepairButton;

	UIX::UIXButton* m_DonateButton;
	UIX::UIXButton* m_JoinButton;
	UIX::UIXButton* m_AdvancedButton;

	UIX::UIXCheckBox* m_ExperimentalBuildsCheckbox;

	// When this is false, the installer will
	// download the HD textures as well (STARPAK's).
	bool m_bPartialInstall;

	bool m_bUpdateViewToggled;
	bool m_bIsInstalled;
};
