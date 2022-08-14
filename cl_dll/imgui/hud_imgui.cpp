//FranticDreamer 2022

// imgui main file
// Base is made by Admer

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

#define FRANUTILS_MODDIR 1 // For usage of mod directory utilites
#include "FranUtils.hpp"

SDL_Window* mainWindow;
SDL_GLContext mainContext;

CClientImgui* mainImgui;

extern int g_iVisibleMouse;

void ClientImGui_HookedDraw()
{
	if (FranUtils::Globals::inMainMenu && !FranUtils::Globals::called3DMainMenu)
	{
		//gEngfuncs.pfnClientCmd("map test_3dbackground");
		FranUtils::Globals::called3DMainMenu = true;
	}
	else if (!FranUtils::Globals::inMainMenu)
	{
		FranUtils::Globals::called3DMainMenu = false;
	}

	mainImgui->DrawImgui();
	SDL_GL_SwapWindow(mainWindow);
}

int ClientImGui_EventWatch(void* data, SDL_Event* event)
{
	return ImGui_ImplSDL2_ProcessEvent(event);
}

void ClientImGui_HWHook()
{
	// Thanks to half payne's developer for the idea
	// Changed some types for constant size things

	#pragma warning( disable : 6387 )

	unsigned int origin = 0;

	MODULEINFO moduleInfo;
	if (GetModuleInformation(GetCurrentProcess(), GetModuleHandle("hw.dll"), &moduleInfo, sizeof(moduleInfo))) 
	{
		origin = (unsigned int)moduleInfo.lpBaseOfDll;

		int8_t* slice = new int8_t[FranUtils::Megabyte];
		ReadProcessMemory(GetCurrentProcess(), (const void*)origin, slice, FranUtils::Megabyte, nullptr);

		// Predefined magic stuff
		uint8_t magic[] = {0x8B, 0x4D, 0x08, 0x83, 0xC4, 0x08, 0x89, 0x01, 0x5D, 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0xA1};

		for (unsigned int i = 0; i < FranUtils::Megabyte - 16; i++) 
		{
			bool sequenceIsMatching = memcmp(slice + i, magic, 16) == 0;
			if (sequenceIsMatching) 
			{
				origin += i + 27;
				break;
			}
		}

		delete[] slice;

		int8_t opCode[1];
		ReadProcessMemory(GetCurrentProcess(), (const void*)origin, opCode, 1, nullptr);
		if (opCode[0] != 0xFFFFFFE8) 
		{
			gEngfuncs.Con_DPrintf("Failed to embed ImGUI. couldn't find opCode.\n");
			return;
		}
	}
	else 
	{
		gEngfuncs.Con_DPrintf("Failed to embed ImGUI: failed to get hw.dll memory base address.\n");
		return;
	}

	ImGui_ImplOpenGL2_Init();
	ImGui_ImplSDL2_InitForOpenGL(mainWindow, ImGui::GetCurrentContext());

	// To make a detour, an offset to dedicated function must be calculated and then correctly replaced
	unsigned int detourFunctionAddress = (unsigned int)&ClientImGui_HookedDraw;
	unsigned int offset = (detourFunctionAddress)-origin - 5;

	// Little endian offset
	uint8_t offsetBytes[4];
	for (int i = 0; i < 4; i++) 
	{
		offsetBytes[i] = (offset >> (i * 8));
	}

	// This is WinAPI call, blatantly overwriting the memory with raw pointer would crash the program
	// Notice the 1 byte offset from the origin
	WriteProcessMemory(GetCurrentProcess(), (void*)(origin + 1), offsetBytes, 4, nullptr);

	SDL_AddEventWatch(ClientImGui_EventWatch, nullptr);

	#pragma warning( default : 6387 )
}

DECLARE_COMMAND(m_clImgui, ImguiQuit);
DECLARE_COMMAND(m_clImgui, ImguiOptions);

void CClientImgui::InitExtension()
{
	isOpen_quitDialog = false;
	isOpen_optionsDialog = false;

	uniqueButtonIDBase = 4096;
	uniqueSliderNameBase = " ";

	HOOK_COMMAND("imgui_quit", ImguiQuit);
	HOOK_COMMAND("imgui_options", ImguiOptions);

	mainWindow = SDL_GetWindowFromID(1);
	//mainContext = SDL_GL_CreateContext(mainWindow);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Set
	mainImgui = this;

	// Extensions
	keyboardManager.Init();
	mouseManager.Init();
	videoManager.Init();
	audioManager.Init();

	// For Overdraw
	ClientImGui_HWHook();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	//ImGui_ImplOpenGL2_Init();
	//ImGui_ImplSDL2_InitForOpenGL(mainWindow, ImGui::GetCurrentContext());
	io.DisplaySize.x = ScreenWidth;
	io.DisplaySize.y = ScreenHeight;

	//Set the real style
	SetTheme();
}

void CClientImgui::DrawImgui()
{
	uniqueButtonIDBase = 4096; // Reset the button IDs
	uniqueSliderNameBase = "  "; // Reset the slider names

	//g_iVisibleMouse = 1;
	//App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::scu_arrow));
	//g_iVisibleMouse = 1;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(mainWindow);
	ImGui::NewFrame();

	if (FranUtils::Globals::isPaused || FranUtils::Globals::in3DMainMenu)
	{
		DrawMainMenu();
	}

	//glViewport( 0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y );

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void CClientImgui::DrawMainMenu()
{
	ImGui::PushFont(brandsFont);

	if (isOpen_quitDialog)
	{
		auto qDialog = CClientImguiDialogBox::DrawDialogBox("Quit", "Do you wish to stop playing now?", DialogButtons::Quit | DialogButtons::Cancel);
		if (qDialog == DialogButtons::Quit)
		{
			FranUtils::QuitGame();
		}
		else if (qDialog == DialogButtons::Cancel) // We need another if cus result can be "None"
		{
			isOpen_quitDialog = false;
		}
	}

	if (videoManager.isOpen_restartDialog)
	{
		auto qDialog = CClientImguiDialogBox::DrawDialogBox("Warning", "Applying thsese settings will restart the game.\nAre you sure", DialogButtons::Restart | DialogButtons::Cancel);
		if (qDialog == DialogButtons::Restart)
		{
			videoManager.ApplyVideoSettings();
			FranUtils::RestartGame();
		}
		else if (qDialog == DialogButtons::Cancel) // We need another if cus result can be "None"
		{
			videoManager.isOpen_restartDialog = false;
		}
	}
	
	if (isOpen_optionsDialog)
	{
		ImGui::SetNextWindowSize(ImVec2(525, 400));

		if (ImGui::Begin((std::string(ICON_FA_STEAM_SYMBOL) + " " + "Options").c_str(), &isOpen_optionsDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			if (ImGui::BeginTabBar("OptionsTabBar"))
			{
				keyboardManager.DrawKeyboardSettingsTab();
				mouseManager.DrawMouseSettingsTab();
				audioManager.DrawAudioSettingsTab();
				videoManager.DrawVideoSettingsTab();
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}
	ImGui::PopFont();
}

void CClientImgui::SetTheme()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//brandsFont = io.Fonts->AddFontFromMemoryCompressedTTF(fa_brands_400_compressed_data, fa_brands_400_compressed_size, 50.0f);
	std::string fontdir = (FranUtils::GetModDirectory() + "resource\\fa-brands-400.ttf");
	brandsFont = io.Fonts->AddFontFromFileTTF(fontdir.c_str(), 20.0f, nullptr, GetGlyphRangesFontAwesome());
	io.Fonts->Build();

	ImVec4* colours = ImGui::GetStyle().Colors;
	colours[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colours[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colours[ImGuiCol_WindowBg] = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
	colours[ImGuiCol_ChildBg] = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
	colours[ImGuiCol_PopupBg] = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
	colours[ImGuiCol_Border] = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
	colours[ImGuiCol_BorderShadow] = ImVec4(0.14f, 0.16f, 0.11f, 0.52f);
	colours[ImGuiCol_FrameBg] = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
	colours[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.30f, 0.23f, 1.00f);
	colours[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.34f, 0.26f, 1.00f);
	colours[ImGuiCol_TitleBg] = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
	colours[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
	colours[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colours[ImGuiCol_MenuBarBg] = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
	colours[ImGuiCol_ScrollbarBg] = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
	colours[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.32f, 0.24f, 1.00f);
	colours[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.30f, 0.22f, 1.00f);
	colours[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.23f, 0.27f, 0.21f, 1.00f);
	colours[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
	colours[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
	colours[ImGuiCol_SliderGrabActive] = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
	colours[ImGuiCol_Button] = ImVec4(0.29f, 0.34f, 0.26f, 0.40f);
	colours[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
	colours[ImGuiCol_ButtonActive] = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
	colours[ImGuiCol_Header] = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
	colours[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.42f, 0.31f, 0.6f);
	colours[ImGuiCol_HeaderActive] = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
	colours[ImGuiCol_Separator] = ImVec4(0.14f, 0.16f, 0.11f, 1.00f);
	colours[ImGuiCol_SeparatorHovered] = ImVec4(0.54f, 0.57f, 0.51f, 1.00f);
	colours[ImGuiCol_SeparatorActive] = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
	colours[ImGuiCol_ResizeGrip] = ImVec4(0.19f, 0.23f, 0.18f, 0.00f); // grip invis
	colours[ImGuiCol_ResizeGripHovered] = ImVec4(0.54f, 0.57f, 0.51f, 1.00f);
	colours[ImGuiCol_ResizeGripActive] = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
	colours[ImGuiCol_Tab] = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
	colours[ImGuiCol_TabHovered] = ImVec4(0.54f, 0.57f, 0.51f, 0.78f);
	colours[ImGuiCol_TabActive] = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
	colours[ImGuiCol_TabUnfocused] = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
	colours[ImGuiCol_TabUnfocusedActive] = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
	//colours[ImGuiCol_DockingPreview] = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
	//colours[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colours[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colours[ImGuiCol_PlotLinesHovered] = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
	colours[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.78f, 0.28f, 1.00f);
	colours[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colours[ImGuiCol_TextSelectedBg] = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
	colours[ImGuiCol_DragDropTarget] = ImVec4(0.73f, 0.67f, 0.24f, 1.00f);
	colours[ImGuiCol_NavHighlight] = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
	colours[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colours[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colours[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameBorderSize = 1.0f;
	style.WindowRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
}

void CClientImgui::FinishExtension()
{
	videoManager.ShutDown();
	SDL_DelEventWatch(ClientImGui_EventWatch, nullptr);
}

unsigned int CClientImgui::GetUniqueButtonID()
{
	return uniqueButtonIDBase++;
}

std::string CClientImgui::GetUniqueSliderName()
{
	return uniqueSliderNameBase += " ";
}

void CClientImgui::UserCmd_ImguiQuit()
{
	FranUtils::PauseMenu();
	isOpen_quitDialog = true;
}

void _cdecl CClientImgui::UserCmd_ImguiOptions()
{
	FranUtils::PauseMenu();
	isOpen_optionsDialog = true;
}


