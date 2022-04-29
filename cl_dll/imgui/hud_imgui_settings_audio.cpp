// FranticDreamer 2022

#include <algorithm>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>

#include "PlatformHeaders.h"
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
#include "hud_imgui_settings_audio.h"

#define FRANUTILS_MODDIR 1 // For usage of mod directory utilites
#include "FranUtils.hpp"
#include "FranUtils.Filesystem.hpp"

void CClientImguiAudioSettings::Init()
{
	liveSoundSettings.masterVolume	= 0.0f;
	liveSoundSettings.mp3Volume		= 0.0f;
	liveSoundSettings.hevVolume		= 0.0f;
	liveSoundSettings.hiRezSound	= true;

	ParseConfigFile("config.cfg");
}

void CClientImguiAudioSettings::ParseConfigFile(std::string filedir)
{
	std::ifstream fstream;
	//fstream.open(filedir);
	FranUtils::Filesystem::OpenInputFile(filedir, fstream);

	int lineIteration = 0;
	std::string line;
	while (std::getline(fstream, line))
	{
		lineIteration++;
		FranUtils::LowerCase_Ref(line);
		gEngfuncs.Con_DPrintf("\n config.cfg - parsing %d", lineIteration);
		if (line.empty()) // Ignore empty lines
		{
			continue;
		}
		else if (line[0] == '/') // Ignore comments
		{
			continue;
		}
		else if (line.substr(0, 7) == "volume ") // Master Volume
		{
			line = line.erase(0, 7);

			// Remove first and last characters of the string, which are the quote marks
			line.pop_back(); line.erase(line.begin());

			if (!line.empty())
			{
				liveSoundSettings.masterVolume = std::stof(line);
			}
			else
			{
				gEngfuncs.Con_DPrintf("\nERR: config.cfg - Can't parse the bind line %d. Are you sure the syntax is correct?\n", lineIteration);
			}

			continue;
		}
		else if (line.substr(0, 11) == "suitvolume ") // Suit Volume
		{
			line = line.erase(0, 11);

			// Remove first and last characters of the string, which are the quote marks
			line.pop_back(); line.erase(line.begin());

			if (!line.empty())
			{
				liveSoundSettings.hevVolume = std::stof(line);
			}
			else
			{
				gEngfuncs.Con_DPrintf("\nERR: config.cfg - Can't parse the bind line %d. Are you sure the syntax is correct?\n", lineIteration);
			}

			continue;
		}
		else if (line.substr(0, 10) == "mp3volume ") // MP3 Volume
		{
			line = line.erase(0, 10);

			// Remove first and last characters of the string, which are the quote marks
			line.pop_back(); line.erase(line.begin());

			if (!line.empty())
			{
				liveSoundSettings.mp3Volume = std::stof(line);
			}
			else
			{
				gEngfuncs.Con_DPrintf("\nERR: config.cfg - Can't parse the bind line %d. Are you sure the syntax is correct?\n", lineIteration);
			}

			continue;
		}
		else if (line.substr(0, 8) == "hisound ") // MP3 Volume
		{
			line = line.erase(0, 8);

			// Remove first and last characters of the string, which are the quote marks
			line.pop_back(); line.erase(line.begin());

			if (!line.empty())
			{
				liveSoundSettings.hiRezSound = std::stoi(line);
			}
			else
			{
				gEngfuncs.Con_DPrintf("\nERR: config.cfg - Can't parse the bind line %d. Are you sure the syntax is correct?\n", lineIteration);
			}

			continue;
		}
	}

	currentSoundSettings	= liveSoundSettings;
}


void CClientImguiAudioSettings::DrawAudioSettingsTab()
{
	if (ImGui::BeginTabItem("Audio"))
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::BeginChildFrame(id_frame_bindingcolumns, ImVec2(frameX, frameY));

		//ImGui::Text("Master Volume: %.2f", liveSoundSettings.masterVolume);
		ImGui::Text("Master Volume: ");
		ImGui::SliderFloat(gHUD.m_clImgui.GetUniqueSliderName().c_str(), &liveSoundSettings.masterVolume, 0.0f, 1.0f, "%.2f");

		ImGui::Text("MP3 Volume: ");
		ImGui::SliderFloat(gHUD.m_clImgui.GetUniqueSliderName().c_str(), &liveSoundSettings.mp3Volume, 0.0f, 1.0f, "%.2f");

		ImGui::Text("HEV Volume: ");
		ImGui::SliderFloat(gHUD.m_clImgui.GetUniqueSliderName().c_str(), &liveSoundSettings.hevVolume, 0.0f, 1.0f, "%.2f");
		
		ImGui::Checkbox("Hi-Res Sound", &liveSoundSettings.hiRezSound);

		ImGui::SetWindowFontScale(0.75f);
		ImGui::Text("MPEG Layer-3 playback supplied with the Miles Sound System from\nRAD Game Tools, Inc. MPEG Layer-3 audio compression technology licensed by\nFraunhofer IIS and THOMSON multimedia.");
		ImGui::SetWindowFontScale(1.0f);

		ImGui::EndChildFrame();

		if (ImGui::Button("OK", ImVec2(buttonX, buttonY)))
		{
			gHUD.m_clImgui.isOpen_optionsDialog = false;
			ApplySoundSettings();
		}

		ImGui::SameLine(((frameX / 1.33) + sweetSpot) - (buttonX / 1.33));
		if (ImGui::Button("Cancel", ImVec2(buttonX, buttonY)))
		{
			gHUD.m_clImgui.isOpen_optionsDialog = false;
			CancelSoundSettings();
		}

		ImGui::SameLine((frameX + sweetSpot) - (buttonX));
		if (ImGui::Button("Apply", ImVec2(buttonX, buttonY)))
		{
			ApplySoundSettings();
		}

		ImGui::EndTabItem();
	}
}

void CClientImguiAudioSettings::ApplySoundSettings()
{
	std::string mastervolcommand = "volume " + std::to_string(liveSoundSettings.masterVolume);
	std::string hevvolcommand = "mp3volume " + std::to_string(liveSoundSettings.mp3Volume);
	std::string mp3volcommand = "suitvolume " + std::to_string(liveSoundSettings.hevVolume);
	std::string hirezcommand = "hisound " + std::to_string((int)liveSoundSettings.hiRezSound);

	gEngfuncs.pfnClientCmd(mastervolcommand.c_str());
	gEngfuncs.pfnClientCmd(hevvolcommand.c_str());
	gEngfuncs.pfnClientCmd(mp3volcommand.c_str());
	gEngfuncs.pfnClientCmd(hirezcommand.c_str());

	currentSoundSettings = liveSoundSettings;
}

void CClientImguiAudioSettings::CancelSoundSettings()
{
	liveSoundSettings = currentSoundSettings;
}

SoundSettings_s CClientImguiAudioSettings::GetSoundSettings()
{
	return currentSoundSettings;
}
