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
#include "hud_imgui_settings_keyboard.h"

class CClientImgui : public CBaseClientExtension
{
public:
	void InitExtension() override;

	void DrawImgui(); // Doesn't override base Draw() because... well... obvious reasons. (spooky function pointer hax)
	void DrawMainMenu();

	void SetTheme();

	void FinishExtension() override;

	CClientImguiConsole consoleManager;
	CClientImguiKeyboardSettings keyboardManager;

	ImFont* brandsFont;

	bool isOptionsClosed;
};



#endif // !HUD_IMGUI_H