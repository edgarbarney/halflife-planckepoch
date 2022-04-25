// FranticDreamer 2022

#include <algorithm>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>

#include <Windows.h>
#include <Psapi.h>

#include "hud.h"
#include "cl_dll.h"
#include "cdll_int.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"
#include "SDL2/SDL.h"
#include <gl/GL.h>

#include "kbutton.h"

#include "vgui_TeamFortressViewport.h"

//#include "imgui.h"
#include "backends/imgui_impl_opengl2.h"
#include "backends/imgui_impl_sdl.h"

#include "fontawesome_brands.h"

#include "hud_imgui.h"
#include "hud_imgui_settings_mouse.h"

#define FRANUTILS_MODDIR 1 // For usage of mod directory utilites
#include "FranUtils.hpp"
#include "FranUtils.Filesystem.hpp"

constexpr int id_frame_bindingcolumns = 32;

constexpr int frameX = 510;
constexpr int frameY = 270;

constexpr int buttonX = 100;
constexpr int buttonY = 30;

constexpr int sweetSpot = 8; // 8 Pixel is the sweet spot for the right side padding

constexpr int sensitivityMin = 0.2f;
constexpr int sensitivityMax = 20.0f;

void CClientImguiMouseSettings::Init()
{
	liveMouseSettings.joystick		= (int)gEngfuncs.pfnGetCvarFloat("joystick");
	liveMouseSettings.mousefilter	= (int)gEngfuncs.pfnGetCvarFloat("m_filter");
	liveMouseSettings.rawinput		= (int)gEngfuncs.pfnGetCvarFloat("m_rawinput");
	liveMouseSettings.autoaim		= (int)gEngfuncs.pfnGetCvarFloat("sv_aim");
	liveMouseSettings.invertpitch	= gEngfuncs.pfnGetCvarFloat("m_pitch") < 0 ? true : false;
	liveMouseSettings.sensitivity = gEngfuncs.pfnGetCvarFloat("sensitivity");

	currentMouseSettings = liveMouseSettings;
}

void CClientImguiMouseSettings::DrawMouseSettingsTab()
{
	static int s_frame = 0;
	if (ImGui::BeginTabItem("Mouse"))
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::BeginChildFrame(id_frame_bindingcolumns, ImVec2(frameX, frameY));

		if (s_frame < 30)
		{
			liveMouseSettings.joystick = (int)gEngfuncs.pfnGetCvarFloat("joystick");
			liveMouseSettings.mousefilter = (int)gEngfuncs.pfnGetCvarFloat("m_filter");
			liveMouseSettings.rawinput = (int)gEngfuncs.pfnGetCvarFloat("m_rawinput");
			liveMouseSettings.autoaim = (int)gEngfuncs.pfnGetCvarFloat("sv_aim");
			liveMouseSettings.invertpitch = gEngfuncs.pfnGetCvarFloat("m_pitch") < 0 ? true : false;
			liveMouseSettings.sensitivity = gEngfuncs.pfnGetCvarFloat("sensitivity");

			currentMouseSettings = liveMouseSettings;

			s_frame++;
		}

		ImGui::Checkbox("Joystick Look", &liveMouseSettings.joystick);
		ImGui::Checkbox("Mouse Filter", &liveMouseSettings.mousefilter);
		ImGui::Checkbox("Raw Input", &liveMouseSettings.rawinput);
		ImGui::Checkbox("Auto-Aim", &liveMouseSettings.autoaim);
		ImGui::Checkbox("Revert Mouse", &liveMouseSettings.invertpitch);

		ImGui::Text("Mouse Sensitivity: ");
		ImGui::SliderFloat(gHUD.m_clImgui.GetUniqueSliderName().c_str(), &liveMouseSettings.sensitivity, sensitivityMin, sensitivityMax, "%.2f");

		ImGui::EndChildFrame();

		if (ImGui::Button("OK", ImVec2(buttonX, buttonY)))
		{
			gHUD.m_clImgui.isOpen_optionsDialog = false;
			ApplyMouseSettings();
		}

		ImGui::SameLine(((frameX / 1.33) + sweetSpot) - (buttonX / 1.33));
		if (ImGui::Button("Cancel", ImVec2(buttonX, buttonY)))
		{
			gHUD.m_clImgui.isOpen_optionsDialog = false;
			CancelMouseSettings();
		}

		ImGui::SameLine((frameX + sweetSpot) - (buttonX));
		if (ImGui::Button("Apply", ImVec2(buttonX, buttonY)))
		{
			ApplyMouseSettings();
		}

		ImGui::EndTabItem();
	}
}

void CClientImguiMouseSettings::ApplyMouseSettings()
{
	float curpitch = gEngfuncs.pfnGetCvarFloat("m_pitch");

	gEngfuncs.pfnClientCmd(("joystick " + std::to_string(liveMouseSettings.joystick)).c_str());
	gEngfuncs.pfnClientCmd(("m_filter " + std::to_string(liveMouseSettings.mousefilter)).c_str());
	gEngfuncs.pfnClientCmd(("m_rawinput " + std::to_string(liveMouseSettings.rawinput)).c_str());
	gEngfuncs.pfnClientCmd(("sv_aim " + std::to_string(liveMouseSettings.autoaim)).c_str());
	gEngfuncs.pfnClientCmd(("sensitivity " + std::to_string(liveMouseSettings.sensitivity)).c_str());

	// Pitch is alterable so we're gonna just change it's sign if we need to
	if (curpitch > 0 && liveMouseSettings.invertpitch)
		gEngfuncs.pfnClientCmd(("m_pitch " + std::to_string(-curpitch)).c_str());
	else if (curpitch < 0 && !liveMouseSettings.invertpitch)
		gEngfuncs.pfnClientCmd(("m_pitch " + std::to_string(-curpitch)).c_str());

	if (liveMouseSettings.joystick)
	{
		gEngfuncs.pfnClientCmd("-mlook");
		gEngfuncs.pfnClientCmd("+jlook");
	}
	else
	{
		gEngfuncs.pfnClientCmd("+mlook");
		gEngfuncs.pfnClientCmd("-jlook");
	}

	currentMouseSettings = liveMouseSettings;
}

void CClientImguiMouseSettings::CancelMouseSettings()
{
	liveMouseSettings = currentMouseSettings;
}

