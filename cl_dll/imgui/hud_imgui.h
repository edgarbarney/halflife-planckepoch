//FranticDreamer 2022

#ifndef HUD_IMGUI_H
#define HUD_IMGUI_H

#include <vector>
#include <array>
#include <map>

//Ugh.
#include "imgui.h"

constexpr int id_frame_bindingcolumns = 32;

constexpr int frameX = 510;
constexpr int frameY = 270;

constexpr int buttonX = 100;
constexpr int buttonY = 30;

constexpr int sweetSpot = 8; // 8 Pixel is the sweet spot for the right side padding

//Thanks to admer for base implementation of imgui
// Just used for imgui for now. Mainly a placeholder for future ADM "inspirations" :gabe:
class CBaseClientExtension : public CHudBase
{
public:
	bool Init();
	virtual void InitExtension() {};
	virtual bool VidInit() { return 1; };

	virtual bool Draw(float flTime) { return 1; };
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