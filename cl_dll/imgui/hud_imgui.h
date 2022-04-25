//FranticDreamer 2022

#ifndef HUD_IMGUI_H
#define HUD_IMGUI_H

#include <vector>
#include <array>
#include <map>

//Ugh.
#include "imgui.h"

//Thanks to admer for base implementation of imgui
// Just used for imgui for now. Mainly a placeholder for future ADM "inspirations" :gabe:
class CBaseClientExtension : public CHudBase
{
public:
	int Init();
	virtual void InitExtension() {};
	virtual int VidInit() { return 1; };

	virtual int Draw(float flTime) { return 1; };
	virtual void Think() {};
	virtual void Reset() {};

	virtual void FinishExtension() {};
};

#include "hud_imgui_console.h"
#include "hud_imgui_dialogbox.h"
#include "hud_imgui_settings_keyboard.h"
#include "hud_imgui_settings_mouse.h"
#include "hud_imgui_settings_video.h"
#include "hud_imgui_settings_audio.h"

class CClientImgui : public CBaseClientExtension
{
private:
	unsigned int uniqueButtonIDBase;		//Button id. For preventing ID clash when buttons have the same name
											//Start from 4096 cus why not mate
	std::string uniqueSliderNameBase;

public:
	void InitExtension() override;

	void DrawImgui(); // Doesn't override base Draw() because... well... obvious reasons. (spooky function pointer hax)
	void DrawMainMenu();

	void SetTheme();

	void FinishExtension() override;

	// Returns the uniqueButtonIDBase then increments it
	unsigned int GetUniqueButtonID();

	// Returns a set of spaces.
	std::string GetUniqueSliderName();

	void _cdecl UserCmd_ImguiQuit();
	void _cdecl UserCmd_ImguiOptions();

	CClientImguiConsole consoleManager;
	CClientImguiKeyboardSettings keyboardManager;
	CClientImguiMouseSettings mouseManager;
	CClientImguiVideoSettings videoManager;
	CClientImguiAudioSettings audioManager;

	ImFont* brandsFont;

	bool isOpen_quitDialog;
	bool isOpen_optionsDialog;
};



#endif // !HUD_IMGUI_H