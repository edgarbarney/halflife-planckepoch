// FranticDreamer 2022

#ifndef HUD_IMGUI_SETTINGS_MOUSE_H
#define HUD_IMGUI_SETTINGS_MOUSE_H

struct MouseSettings_s
{
	bool joystick;
	bool mousefilter;
	bool invertpitch;
	bool rawinput;
	bool autoaim;

	float sensitivity;
};

class CClientImguiMouseSettings
{
private:
	// Mouse settings that are being controlled live
	MouseSettings_s liveMouseSettings;
	// Mouse settings that are applied OR loaded from registry.
	MouseSettings_s currentMouseSettings;

public:
	void Init();

	void DrawMouseSettingsTab();

	void ApplyMouseSettings();
	void CancelMouseSettings();

};

#endif