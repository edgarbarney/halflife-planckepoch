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
#include "hud_imgui_settings_video.h"

#define FRANUTILS_MODDIR 1 // For usage of mod directory utilites
#include "FranUtils.hpp"
#include "FranUtils.Filesystem.hpp"

constexpr int defaultRefreshRate = 60;

constexpr float brightnessMin	= 0.0f;
constexpr float brightnessMax	= 2.0f;
constexpr float gammaMin		= 1.8f;
constexpr float gammaMax		= 3.0f;

void CClientImguiVideoSettings::Init()
{
	isOpen_restartDialog = false;

	windowSatutsNameMap = 
	{ 
	{WindowStatus_e::BorderlessFullscreen,	"Fullscreen"},
	{WindowStatus_e::Windowed,				"Windowed"},
	{WindowStatus_e::Fullscreen,			"Fullscreen (Legacy)"}
	};

	// Valve used registry keys to store resolution data for some reason.
	// So we must mangle with these. But thanks to the WinReg, its not that complex.

	HalfLifeRegKey.Open(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Half-Life\\Settings");

	if (HalfLifeRegKey.GetStringValue(L"EngineDLL") != L"hw.dll")
	{
		MessageBox(nullptr, "Software renderer is not supported in Spirinty.\nOpenGL Mode is initiated.", "Warning", MB_OK);
		HalfLifeRegKey.SetStringValue(L"EngineDLL", L"hw.dll");
	}

	if (auto t = HalfLifeRegKey.TryGetDwordValue(L"ScreenBPP"))
		liveVideoSettings.colourDepth = t.value();

	if (auto t = HalfLifeRegKey.TryGetDwordValue(L"hdmodels"))
		liveVideoSettings.hdModels = t.value();

	if (auto t = HalfLifeRegKey.TryGetDwordValue(L"ScreenHeight"))
		liveVideoSettings.screenHeight = t.value();

	if (auto t = HalfLifeRegKey.TryGetDwordValue(L"ScreenWidth"))
		liveVideoSettings.screenWidth = t.value();

	if (auto t = HalfLifeRegKey.TryGetDwordValue(L"RefreshRate"))
		liveVideoSettings.refreshRate = t.value();
	else
		liveVideoSettings.refreshRate = defaultRefreshRate;

	HalfLifeRegKey.SetDwordValue(L"vid_level", 0); //Disable "low quality" mode

	liveVideoSettings.windowType = WindowStatus_e::BorderlessFullscreen;

	std::set<GameResolution_s> tempSet; // For an ordered list with no duplication

	SDL_DisplayMode slddm;
	for (int i = 0; i < SDL_GetNumDisplayModes(0); i++)
	{
		SDL_GetDisplayMode(0, i, &slddm);
		tempSet.insert(GameResolution_s(i, slddm.w, slddm.h, slddm.refresh_rate));
	}

	resolutionVector.clear();
	resolutionVector.assign(tempSet.begin(), tempSet.end());

	resComboLiveIndex = 0;

	for (size_t i = 0; i < resolutionVector.size(); i++)
	{
		if (liveVideoSettings.screenHeight == resolutionVector[i].height && liveVideoSettings.screenWidth == resolutionVector[i].width && liveVideoSettings.refreshRate == resolutionVector[i].refreshRate)
		{
			resComboLiveIndex = i;
			resComboCurrentIndex = i;
			break;
		}
	}

	liveVideoSettings.brightness		= gEngfuncs.pfnGetCvarFloat("brightness");
	liveVideoSettings.gamma				= gEngfuncs.pfnGetCvarFloat("gamma");
	liveVideoSettings.vsync				= (int)gEngfuncs.pfnGetCvarFloat("gl_vsync");

	// Spirinity Settings
	liveVideoSettings.grayScale			= (int)gEngfuncs.pfnGetCvarFloat("te_grayscale");
	liveVideoSettings.postProcessing	= (int)gEngfuncs.pfnGetCvarFloat("te_posteffects");
	liveVideoSettings.overlapDecals		= (int)gEngfuncs.pfnGetCvarFloat("te_overlapdecals");
	liveVideoSettings.shadows			= (int)gEngfuncs.pfnGetCvarFloat("te_shadows");
	liveVideoSettings.shadowsFilter		= (int)gEngfuncs.pfnGetCvarFloat("te_shadows_filter");
	liveVideoSettings.dynLights			= (int)gEngfuncs.pfnGetCvarFloat("te_dynlights");


	currentVideoSettings = liveVideoSettings;
}

void CClientImguiVideoSettings::DrawVideoSettingsTab()
{
	static int s_frame = 0;
	if (ImGui::BeginTabItem("Video"))
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::BeginChildFrame(id_frame_bindingcolumns, ImVec2(frameX, frameY));

		ImGui::Text("Display Settings");
		ImGui::Spacing();

		ImGui::Text("Resolution: ");
		std::string comboDisplayText = resolutionVector[resComboLiveIndex].displayName;
		if (ImGui::BeginCombo(" ", comboDisplayText.c_str(), ImGuiComboFlags_None))
        {
            for (size_t n = 0; n < resolutionVector.size(); n++)
            {
                const bool is_selected = (resComboLiveIndex == n);
                if (ImGui::Selectable(resolutionVector[n].displayName.c_str(), is_selected))
					resComboLiveIndex = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
		liveVideoSettings.SetResolution(resolutionVector[resComboLiveIndex]);

	
		ImGui::Text("Window Type: ");
		std::string windComboDisplayText = windowSatutsNameMap[(WindowStatus_e)winStatComboLiveIndex];
		if (ImGui::BeginCombo("  ", windComboDisplayText.c_str(), ImGuiComboFlags_None))
		{
			for (size_t n = 0; n < windowSatutsNameMap.size(); n++)
			{
				const bool is_selected = (winStatComboLiveIndex == n);
				if (ImGui::Selectable(windowSatutsNameMap[(WindowStatus_e)n].c_str(), is_selected))
					winStatComboLiveIndex = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
		liveVideoSettings.windowTypeDword = winStatComboLiveIndex;

		if (s_frame < 30)
		{
			// Refreshing a bit late fixes some issues
			liveVideoSettings.brightness		= gEngfuncs.pfnGetCvarFloat("brightness");
			liveVideoSettings.gamma				= gEngfuncs.pfnGetCvarFloat("gamma");
			liveVideoSettings.vsync				= (int)gEngfuncs.pfnGetCvarFloat("gl_vsync");

			// Spirinity Settings
			liveVideoSettings.grayScale			= (int)gEngfuncs.pfnGetCvarFloat("te_grayscale");
			liveVideoSettings.postProcessing	= (int)gEngfuncs.pfnGetCvarFloat("te_posteffects");
			liveVideoSettings.overlapDecals		= (int)gEngfuncs.pfnGetCvarFloat("te_overlapdecals");
			liveVideoSettings.shadows			= (int)gEngfuncs.pfnGetCvarFloat("te_shadows");
			liveVideoSettings.shadowsFilter		= (int)gEngfuncs.pfnGetCvarFloat("te_shadows_filter");
			liveVideoSettings.dynLights			= (int)gEngfuncs.pfnGetCvarFloat("te_dynlights");

			currentVideoSettings = liveVideoSettings;

			s_frame++;
		}

		ImGui::Checkbox("VSync", &liveVideoSettings.vsync);

		ImGui::Text("Brightness: ");
		ImGui::SliderFloat(gHUD.m_clImgui.GetUniqueSliderName().c_str(), &liveVideoSettings.brightness, brightnessMin, brightnessMax, "%.2f");

		ImGui::Text("Gamma: ");
		ImGui::SliderFloat(gHUD.m_clImgui.GetUniqueSliderName().c_str(), &liveVideoSettings.gamma, gammaMin, gammaMax, "%.2f");

		ImGui::Separator();

		ImGui::Text("Graphics Settings");
		ImGui::Spacing();
		ImGui::Checkbox("HD Models", &liveVideoSettings.hdModels);
		ImGui::Checkbox("Overlap Decals", &liveVideoSettings.overlapDecals);
		ImGui::Checkbox("Shadows", &liveVideoSettings.shadows);
		ImGui::Checkbox("Shadow Filtering", &liveVideoSettings.shadowsFilter);
		ImGui::Checkbox("Dynamic Lights", &liveVideoSettings.dynLights);

		ImGui::Separator();

		ImGui::Text("Post-Processing Settings");
		ImGui::Spacing();
		ImGui::Checkbox("Post-Processing Master", &liveVideoSettings.postProcessing);
		ImGui::Checkbox("Grayscale", &liveVideoSettings.grayScale);

		ImGui::EndChildFrame();

		if (ImGui::Button("OK", ImVec2(buttonX, buttonY)))
		{
			gHUD.m_clImgui.isOpen_optionsDialog = false;
			isOpen_restartDialog = true;
		}

		ImGui::SameLine(((frameX / 1.33) + sweetSpot) - (buttonX / 1.33));
		if (ImGui::Button("Cancel", ImVec2(buttonX, buttonY)))
		{
			gHUD.m_clImgui.isOpen_optionsDialog = false;
			CancelVideoSettings();
		}

		ImGui::SameLine((frameX + sweetSpot) - (buttonX));
		if (ImGui::Button("Apply", ImVec2(buttonX, buttonY)))
		{
			isOpen_restartDialog = true;
		}

		ImGui::EndTabItem();
	}
}

void CClientImguiVideoSettings::ApplyVideoSettings()
{
	const auto& itemt = resolutionVector[resComboLiveIndex];
	gEngfuncs.Con_DPrintf("W: %d, H: %d, HZ: %d\n", itemt.width, itemt.height, itemt.refreshRate);

	HalfLifeRegKey.SetDwordValue(L"ScreenBPP", liveVideoSettings.colourDepth);
	HalfLifeRegKey.SetDwordValue(L"hdmodels", liveVideoSettings.hdModels);
	HalfLifeRegKey.SetDwordValue(L"ScreenHeight", liveVideoSettings.screenHeight);
	HalfLifeRegKey.SetDwordValue(L"ScreenWidth", liveVideoSettings.screenWidth);
	HalfLifeRegKey.SetDwordValue(L"RefreshRate", liveVideoSettings.refreshRate);

	HalfLifeRegKey.SetDwordValue(L"vid_level", 0); //Disable "low quality" mode

	if (liveVideoSettings.windowType == WindowStatus_e::Fullscreen)
		HalfLifeRegKey.SetDwordValue(L"ScreenWindowed", 0); // Legacy Fullscreen
	else
		HalfLifeRegKey.SetDwordValue(L"ScreenWindowed", 1);

	std::string vsync_cmd = "gl_vsync " + std::to_string(liveVideoSettings.vsync);
	gEngfuncs.pfnClientCmd(vsync_cmd.c_str());

	std::string fscreen_cmd = "r_fullscreen_type " + std::to_string(liveVideoSettings.windowTypeDword);
	gEngfuncs.pfnClientCmd(fscreen_cmd.c_str());

	std::string brightns_cmd = "brightness " + std::to_string(liveVideoSettings.brightness);
	gEngfuncs.pfnClientCmd(brightns_cmd.c_str());

	std::string gmma_cmd = "gamma " + std::to_string(liveVideoSettings.gamma);
	gEngfuncs.pfnClientCmd(gmma_cmd.c_str());


	std::string graysc_cmd = "te_grayscale " + std::to_string(liveVideoSettings.grayScale);
	gEngfuncs.pfnClientCmd(graysc_cmd.c_str());

	std::string ppeff_cmd = "te_posteffects " + std::to_string(liveVideoSettings.postProcessing);
	gEngfuncs.pfnClientCmd(ppeff_cmd.c_str());

	std::string decalov_cmd = "te_overlapdecals " + std::to_string(liveVideoSettings.overlapDecals);
	gEngfuncs.pfnClientCmd(decalov_cmd.c_str());

	std::string shdw_cmd = "te_shadows " + std::to_string(liveVideoSettings.shadows);
	gEngfuncs.pfnClientCmd(shdw_cmd.c_str());

	std::string sdhwfltr_cmd = "te_shadows_filter " + std::to_string(liveVideoSettings.shadowsFilter);
	gEngfuncs.pfnClientCmd(sdhwfltr_cmd.c_str());

	std::string dynlghts_cmd = "te_dynlights " + std::to_string(liveVideoSettings.dynLights);
	gEngfuncs.pfnClientCmd(dynlghts_cmd.c_str());


	resComboCurrentIndex = resComboLiveIndex;
	currentVideoSettings = liveVideoSettings;
}

void CClientImguiVideoSettings::CancelVideoSettings()
{
	resComboLiveIndex = resComboCurrentIndex;
	liveVideoSettings = currentVideoSettings;
}

void CClientImguiVideoSettings::CheckForBorderless()
{
	if ((WindowStatus_e)CVAR_GET_FLOAT("r_fullscreen_type") == WindowStatus_e::BorderlessFullscreen)
	{
		auto brd_window = gHUD.BRD_GetWindow();
		if (brd_window)
		{
			if (!(SDL_GetWindowFlags(brd_window) & SDL_WINDOW_FULLSCREEN))
				//gHUD.BRD_SetBorderless(gHUD.BRD_GetWindow());
				gHUD.BRD_SetBorderless(brd_window);
		}
	}
}

void CClientImguiVideoSettings::ShutDown()
{
	HalfLifeRegKey.Close();
}

