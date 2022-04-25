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
#include "hud_imgui_settings_keyboard.h"

#define FRANUTILS_MODDIR 1 // For usage of mod directory utilites
#include "FranUtils.hpp"
#include "FranUtils.Filesystem.hpp"

constexpr int id_frame_bindingcolumns = 32;

constexpr int frameX = 510;
constexpr int frameY = 270;

constexpr int buttonX = 100;
constexpr int buttonY = 30;

constexpr int sweetSpot = 8; // 8 Pixel is the sweet spot for the right side padding

void CClientImguiKeyboardSettings::Init()
{
	ParseKeybindFormatData();
	ParseKeybindData();
	
	/*
	if (std::filesystem::exists(FranUtils::GetModDirectory() + "bindingconfig.cfg"))
		ParseBindingConfigFile(); // Custom Config.cfg
	else if (std::filesystem::exists(FranUtils::GetModDirectory() + "config.cfg"))
		ParseDefaultConfigFile(FranUtils::GetModDirectory() + "config.cfg"); // Mod Config.cfg
	else
		ParseDefaultConfigFile(std::filesystem::current_path().string() + "//" + FranUtils::Globals::GetFallbackDir() + "//" + "config.cfg"); // Fallback dir config.cfg
	*/

	ParseDefaultConfigFile("config.cfg");

	// Copy data from vecKeybindsData into the backup vector
	vecKeybindsOriginalData = vecKeybindsData;
}

void CClientImguiKeyboardSettings::DrawKeyboardSettingsTab()
{
	if (ImGui::BeginTabItem("Keyboard"))
	{
		ImGuiIO& io = ImGui::GetIO();

		// Half-Life Mouse Wheel Event for "Key" imitation
		switch ((int)io.MouseWheel)
		{
		case -1:
			io.AddKeyEvent(ImGuiKey_MouseWheelDown, true);
			io.AddKeyEvent(ImGuiKey_MouseWheelUp, false);
			break;
		case 1:
			io.AddKeyEvent(ImGuiKey_MouseWheelDown, false);
			io.AddKeyEvent(ImGuiKey_MouseWheelUp, true);
			break;
		case 0:
		default:
			io.AddKeyEvent(ImGuiKey_MouseWheelDown, false);
			io.AddKeyEvent(ImGuiKey_MouseWheelUp, false);
			break;
		}

		if (keyCaptureMode == KeyCaptureMode::NoCapture)
			ImGui::BeginChildFrame(id_frame_bindingcolumns, ImVec2(frameX, frameY));
		else
			ImGui::BeginChildFrame(id_frame_bindingcolumns, ImVec2(frameX, frameY), ImGuiWindowFlags_NoScrollWithMouse);

		ImGui::Columns(3, "bindingcolumns");
		for (/*const*/ auto& elem : vecKeybindsData)
		{
			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.62f, 0.57f, 0.20f, 1.00f));
			ImGui::Text(elem.categoryName.c_str()); ImGui::NextColumn();
			ImGui::Text("Key/Button"); ImGui::NextColumn();
			ImGui::Text("Alternate"); ImGui::NextColumn();
			ImGui::PopStyleColor();
			ImGui::Separator();
			for (/*const*/ auto& elem2 : elem.categoryCommands)
			{
				ImGui::Text(std::string(elem2.displayName).c_str()); ImGui::NextColumn();
				if (keyCaptureMode == KeyCaptureMode::Primary && currentKeyCapturingBind == elem2.displayName)
				{
					// Primary Button
					ImGui::PushID(gHUD.m_clImgui.GetUniqueButtonID());
					ImGui::Button("*INPUT WAITING*", ImVec2(ImGui::GetColumnWidth(-1) - 15, 0.0f));
					for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
					{
						if (ImGui::IsKeyPressed(key))
						{
							currentKeyCapturingBind.clear();
							keyCaptureMode = KeyCaptureMode::NoCapture;
							AddToKeybindQueue(SmallKeybind_s(elem2.command, key, false));
						}
					}
					ImGui::NextColumn();
					ImGui::PopID();

					// Alternate Button
					// No need for ID management for unresponsive buttons
					ImGui::Button(GetMappedKeyName(elem2.alternate).c_str(), ImVec2(ImGui::GetColumnWidth(-1) - 15, 0.0f)); ImGui::NextColumn();
				}
				else if (keyCaptureMode == KeyCaptureMode::Alternate && currentKeyCapturingBind == elem2.displayName)
				{
					// Primary Button
					// No need for ID management for unresponsive buttons
					ImGui::Button(GetMappedKeyName(elem2.primary).c_str(), ImVec2(ImGui::GetColumnWidth(-1) - 15, 0.0f)); ImGui::NextColumn();

					// Alternate Button
					ImGui::PushID(gHUD.m_clImgui.GetUniqueButtonID());
					ImGui::Button("*INPUT WAITING*", ImVec2(ImGui::GetColumnWidth(-1) - 15, 0.0f));
					for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
					{
						if (ImGui::IsKeyPressed(key))
						{
							currentKeyCapturingBind.clear();
							keyCaptureMode = KeyCaptureMode::NoCapture;
							AddToKeybindQueue(SmallKeybind_s(elem2.command, key, true));
						}
					}
					ImGui::PopID();
					ImGui::NextColumn();
				}
				else
				{
					// Primary Button
					ImGui::PushID(gHUD.m_clImgui.GetUniqueButtonID());
					if (ImGui::Button(GetMappedKeyName(elem2.primary).c_str(), ImVec2(ImGui::GetColumnWidth(-1) - 15, 0.0f)))
					{
						keyCaptureMode = KeyCaptureMode::Primary;
						currentKeyCapturingBind = elem2.displayName;
					}
					ImGui::PopID();
					ImGui::NextColumn();

					// Alternate Button
					ImGui::PushID(gHUD.m_clImgui.GetUniqueButtonID());
					if (ImGui::Button(GetMappedKeyName(elem2.alternate).c_str(), ImVec2(ImGui::GetColumnWidth(-1) - 15, 0.0f)))
					{
						keyCaptureMode = KeyCaptureMode::Alternate;
						currentKeyCapturingBind = elem2.displayName;
					}
					ImGui::PopID();
					ImGui::NextColumn();
				}
			}
		}
		ImGui::EndChildFrame();

		if (ImGui::Button("OK", ImVec2(buttonX, buttonY)))
		{
			gHUD.m_clImgui.isOpen_optionsDialog = false;
			ApplyKeybinds();
		}

		ImGui::SameLine(((frameX / 1.33) + sweetSpot) - (buttonX / 1.33));
		if (ImGui::Button("Cancel", ImVec2(buttonX, buttonY)))
		{
			gHUD.m_clImgui.isOpen_optionsDialog = false;
			CancelKeybinds();
		}

		ImGui::SameLine((frameX + sweetSpot) - (buttonX));
		if (ImGui::Button("Apply", ImVec2(buttonX, buttonY)))
		{
			ApplyKeybinds();
		}

		ImGui::EndTabItem();
	}
}

void CClientImguiKeyboardSettings::ParseKeybindFormatData()
{
	std::ifstream fstream;
	//fstream.open(FranUtils::GetModDirectory() + "resource\\ControlsList.txt");
	FranUtils::Filesystem::OpenInputFile("resource\\ControlsList.txt", fstream);

	std::string lastType;

	bool inSection = false;
	int stepIteration = 0;
	int lineIteration = 0;
	std::string line;
	while (std::getline(fstream, line))
	{
		lineIteration++;
		//line.erase(remove_if(line.begin(), line.end(), isspace), line.end()); // Remove whitespace

		if (line.empty()) // Ignore empty lines
		{
			continue;
		}
		else if (line[0] == '/') // Ignore comments
		{
			continue;
		}
		else if (line[0] == '{') // Opening braces will start the operation
		{
			inSection = true;
			continue;
		}
		else if (line[0] == '}') // Closing braces will terminate the operation
		{
			inSection = false;
			lastType = "";
			stepIteration++;
			continue;
		}
		else if (line[0] == '"')
		{

			if (!inSection)
			{
				lastType = line;
				lastType.pop_back(); lastType.erase(lastType.begin()); // Remove first and last characters of the string, which are the quote marks
				vecKeybindsData.push_back(KeybindCategory_s(lastType));
				continue;
			}
			else
			{
				std::istringstream iss(line);
				std::string command, uidesc;

				std::string buffer;
				//bool inStr = false;
				int wordNum = 0;
				while (iss >> buffer)
				{
					/*
					if (!inStr && buffer.front() == '"') // Start of a word
					{
						inStr = true;
					}

					else if (buffer.back() == '"')
					{
						inStr = false;
						wordNum++;
					}
					*/


					if (wordNum == 0)
					{
						command.empty() ? command += buffer : command += " " + buffer;
					}
					else
					{
						uidesc.empty() ? uidesc += buffer : uidesc += " " + buffer;
					}

					if (buffer.back() == '"')
					{
						wordNum++;
					}
				}

				// Remove first and last characters of the string, which are the quote marks
				command.pop_back(); command.erase(command.begin());
				uidesc.pop_back(); uidesc.erase(uidesc.begin());

				vecKeybindsData.back().categoryCommands.push_back(KeybindKey_s(command, uidesc));
				// Also insert into the command map that will make things easier and faster in expense of size.

				continue;
			}
		}
		else
		{
			gEngfuncs.Con_DPrintf("\nERR: ControlsList.txt - Can't parse line %d. Are you sure the syntax is correct?\n\n %s", lineIteration, line.c_str());
		}
	}
}

void CClientImguiKeyboardSettings::ParseKeybindData()
{
#pragma warning( disable : 26812 )

	// This is mostly a manual mapping.
	imguiKeyToGameKeyMap =
	{
		{ImGuiKey_Tab,					ClientImguiKey_s(ImGuiKey_Tab,				"tab")},
		{ImGuiKey_LeftArrow,			ClientImguiKey_s(ImGuiKey_LeftArrow,		"leftarrow")},
		{ImGuiKey_RightArrow,			ClientImguiKey_s(ImGuiKey_RightArrow,		"rightarrow")},
		{ImGuiKey_UpArrow,				ClientImguiKey_s(ImGuiKey_UpArrow,			"uparrow")},
		{ImGuiKey_DownArrow,			ClientImguiKey_s(ImGuiKey_DownArrow,		"downarrow")},
		{ImGuiKey_PageUp,				ClientImguiKey_s(ImGuiKey_PageUp,			"pgup")},
		{ImGuiKey_PageDown,				ClientImguiKey_s(ImGuiKey_PageDown,			"pgdn")},
		{ImGuiKey_Home,					ClientImguiKey_s(ImGuiKey_Home,				"home")},
		{ImGuiKey_End,					ClientImguiKey_s(ImGuiKey_End,				"end")},
		{ImGuiKey_Insert,				ClientImguiKey_s(ImGuiKey_Insert,			"ins")},
		{ImGuiKey_Delete,				ClientImguiKey_s(ImGuiKey_Delete,			"del")},
		{ImGuiKey_Backspace,			ClientImguiKey_s(ImGuiKey_Backspace,		"backspace")},
		{ImGuiKey_Space,				ClientImguiKey_s(ImGuiKey_Space,			"space")},
		{ImGuiKey_Enter,				ClientImguiKey_s(ImGuiKey_Enter,			"enter")},
		{ImGuiKey_Escape,				ClientImguiKey_s(ImGuiKey_Escape,			"escape")},
		{ImGuiKey_LeftCtrl,				ClientImguiKey_s(ImGuiKey_LeftCtrl,			"ctrl")},
		{ImGuiKey_LeftShift,			ClientImguiKey_s(ImGuiKey_LeftShift,		"shift")},
		{ImGuiKey_LeftAlt,				ClientImguiKey_s(ImGuiKey_LeftAlt,			"alt")},
		{ImGuiKey_LeftSuper,			ClientImguiKey_s(ImGuiKey_LeftSuper,		"win")},
		{ImGuiKey_RightCtrl,			ClientImguiKey_s(ImGuiKey_RightCtrl,		"ctrl")}, //The game doesn't differantiate on the left-right function keys
		{ImGuiKey_RightShift,			ClientImguiKey_s(ImGuiKey_RightShift,		"shift")}, //The game doesn't differantiate on the left-right function keys
		{ImGuiKey_RightAlt,				ClientImguiKey_s(ImGuiKey_RightAlt,			"alt")}, //The game doesn't differantiate on the left-right function keys
		{ImGuiKey_RightSuper,			ClientImguiKey_s(ImGuiKey_RightSuper,		"win")}, //The game doesn't differantiate on the left-right function keys
		{ImGuiKey_Menu,					ClientImguiKey_s(ImGuiKey_Menu,				"")}, //Unsupported
		{ImGuiKey_0,					ClientImguiKey_s(ImGuiKey_0,				"0")},
		{ImGuiKey_1,					ClientImguiKey_s(ImGuiKey_1,				"1")},
		{ImGuiKey_2,					ClientImguiKey_s(ImGuiKey_2,				"2")},
		{ImGuiKey_3,					ClientImguiKey_s(ImGuiKey_3,				"3")},
		{ImGuiKey_4,					ClientImguiKey_s(ImGuiKey_4,				"4")},
		{ImGuiKey_5,					ClientImguiKey_s(ImGuiKey_5,				"5")},
		{ImGuiKey_6,					ClientImguiKey_s(ImGuiKey_6,				"6")},
		{ImGuiKey_7,					ClientImguiKey_s(ImGuiKey_7,				"7")},
		{ImGuiKey_8,					ClientImguiKey_s(ImGuiKey_8,				"8")},
		{ImGuiKey_9,					ClientImguiKey_s(ImGuiKey_9,				"9")},
		{ImGuiKey_A,					ClientImguiKey_s(ImGuiKey_A,				"a")},
		{ImGuiKey_B,					ClientImguiKey_s(ImGuiKey_B,				"b")},
		{ImGuiKey_C,					ClientImguiKey_s(ImGuiKey_C,				"c")},
		{ImGuiKey_D,					ClientImguiKey_s(ImGuiKey_D,				"d")},
		{ImGuiKey_E,					ClientImguiKey_s(ImGuiKey_E,				"e")},
		{ImGuiKey_F,					ClientImguiKey_s(ImGuiKey_F,				"f")},
		{ImGuiKey_G,					ClientImguiKey_s(ImGuiKey_G,				"g")},
		{ImGuiKey_H,					ClientImguiKey_s(ImGuiKey_H,				"h")},
		{ImGuiKey_I,					ClientImguiKey_s(ImGuiKey_I,				"i")},
		{ImGuiKey_J,					ClientImguiKey_s(ImGuiKey_J,				"j")},
		{ImGuiKey_K,					ClientImguiKey_s(ImGuiKey_K,				"k")},
		{ImGuiKey_L,					ClientImguiKey_s(ImGuiKey_L,				"l")},
		{ImGuiKey_M,					ClientImguiKey_s(ImGuiKey_M,				"m")},
		{ImGuiKey_N,					ClientImguiKey_s(ImGuiKey_N,				"n")},
		{ImGuiKey_O,					ClientImguiKey_s(ImGuiKey_O,				"o")},
		{ImGuiKey_P,					ClientImguiKey_s(ImGuiKey_P,				"p")},
		{ImGuiKey_Q,					ClientImguiKey_s(ImGuiKey_Q,				"q")},
		{ImGuiKey_R,					ClientImguiKey_s(ImGuiKey_R,				"r")},
		{ImGuiKey_S,					ClientImguiKey_s(ImGuiKey_S,				"s")},
		{ImGuiKey_T,					ClientImguiKey_s(ImGuiKey_T,				"t")},
		{ImGuiKey_U,					ClientImguiKey_s(ImGuiKey_U,				"u")},
		{ImGuiKey_V,					ClientImguiKey_s(ImGuiKey_V,				"v")},
		{ImGuiKey_W,					ClientImguiKey_s(ImGuiKey_W,				"w")},
		{ImGuiKey_X,					ClientImguiKey_s(ImGuiKey_X,				"x")},
		{ImGuiKey_Y,					ClientImguiKey_s(ImGuiKey_Y,				"y")},
		{ImGuiKey_Z,					ClientImguiKey_s(ImGuiKey_Z,				"z")},
		{ImGuiKey_F1,					ClientImguiKey_s(ImGuiKey_F1,				"F1")},
		{ImGuiKey_F2,					ClientImguiKey_s(ImGuiKey_F2,				"F2")},
		{ImGuiKey_F3,					ClientImguiKey_s(ImGuiKey_F3,				"F3")},
		{ImGuiKey_F4,					ClientImguiKey_s(ImGuiKey_F4,				"F4")},
		{ImGuiKey_F5,					ClientImguiKey_s(ImGuiKey_F5,				"F5")},
		{ImGuiKey_F6,					ClientImguiKey_s(ImGuiKey_F6,				"F6")},
		{ImGuiKey_F7,					ClientImguiKey_s(ImGuiKey_F7,				"F7")},
		{ImGuiKey_F8,					ClientImguiKey_s(ImGuiKey_F8,				"F8")},
		{ImGuiKey_F9,					ClientImguiKey_s(ImGuiKey_F9,				"F9")},
		{ImGuiKey_F10,					ClientImguiKey_s(ImGuiKey_F10,				"F10")},
		{ImGuiKey_F11,					ClientImguiKey_s(ImGuiKey_F11,				"F11")},
		{ImGuiKey_F12,					ClientImguiKey_s(ImGuiKey_F12,				"F12")},
		{ImGuiKey_Apostrophe,    		ClientImguiKey_s(ImGuiKey_Apostrophe,    	"'")},
		{ImGuiKey_Comma,         		ClientImguiKey_s(ImGuiKey_Comma,         	",")},
		{ImGuiKey_Minus,         		ClientImguiKey_s(ImGuiKey_Minus,         	"-")},
		{ImGuiKey_Period,        		ClientImguiKey_s(ImGuiKey_Period,        	".")},
		{ImGuiKey_Slash,         		ClientImguiKey_s(ImGuiKey_Slash,         	"/")},
		{ImGuiKey_Semicolon,     		ClientImguiKey_s(ImGuiKey_Semicolon,     	"semicolon", ";")},
		{ImGuiKey_Equal,         		ClientImguiKey_s(ImGuiKey_Equal,         	"=")},
		{ImGuiKey_LeftBracket,   		ClientImguiKey_s(ImGuiKey_LeftBracket,   	"[")},
		{ImGuiKey_Backslash,     		ClientImguiKey_s(ImGuiKey_Backslash,     	"\\")},
		{ImGuiKey_RightBracket,  		ClientImguiKey_s(ImGuiKey_RightBracket,  	"]")},
		{ImGuiKey_GraveAccent,   		ClientImguiKey_s(ImGuiKey_GraveAccent,   	"~", "`")},
		{ImGuiKey_CapsLock,				ClientImguiKey_s(ImGuiKey_CapsLock,			"capslock")},
		{ImGuiKey_ScrollLock,			ClientImguiKey_s(ImGuiKey_ScrollLock,		"")}, //Unsupported
		{ImGuiKey_NumLock,				ClientImguiKey_s(ImGuiKey_NumLock,			"")}, //Unsupported
		{ImGuiKey_PrintScreen,			ClientImguiKey_s(ImGuiKey_PrintScreen,		"")}, //Unsupported
		{ImGuiKey_Pause,				ClientImguiKey_s(ImGuiKey_Pause,			"pause")},
		{ImGuiKey_Keypad0,				ClientImguiKey_s(ImGuiKey_Keypad0,			"kp_ins")},
		{ImGuiKey_Keypad1,				ClientImguiKey_s(ImGuiKey_Keypad1,			"kp_end")},
		{ImGuiKey_Keypad2,				ClientImguiKey_s(ImGuiKey_Keypad2,			"kp_downarrow")},
		{ImGuiKey_Keypad3,				ClientImguiKey_s(ImGuiKey_Keypad3,			"kp_pgdn")},
		{ImGuiKey_Keypad4,				ClientImguiKey_s(ImGuiKey_Keypad4,			"kp_leftarrow")},
		{ImGuiKey_Keypad5,				ClientImguiKey_s(ImGuiKey_Keypad5,			"kp_5")},
		{ImGuiKey_Keypad6,				ClientImguiKey_s(ImGuiKey_Keypad6,			"kp_rightarrow")},
		{ImGuiKey_Keypad7,				ClientImguiKey_s(ImGuiKey_Keypad7,			"kp_home")},
		{ImGuiKey_Keypad8,				ClientImguiKey_s(ImGuiKey_Keypad8,			"kp_uparrow")},
		{ImGuiKey_Keypad9,				ClientImguiKey_s(ImGuiKey_Keypad9,			"kp_pgup")},
		{ImGuiKey_KeypadDecimal,		ClientImguiKey_s(ImGuiKey_KeypadDecimal,	"kp_del")},
		{ImGuiKey_KeypadDivide,			ClientImguiKey_s(ImGuiKey_KeypadDivide,		"kp_slash")},
		{ImGuiKey_KeypadMultiply,		ClientImguiKey_s(ImGuiKey_KeypadMultiply,	"kp_multiply")},
		{ImGuiKey_KeypadSubtract,		ClientImguiKey_s(ImGuiKey_KeypadSubtract,	"kp_minus")},
		{ImGuiKey_KeypadAdd,			ClientImguiKey_s(ImGuiKey_KeypadAdd,		"kp_plus")},
		{ImGuiKey_KeypadEnter,			ClientImguiKey_s(ImGuiKey_KeypadEnter,		"kp_enter")},
		{ImGuiKey_KeypadEqual,			ClientImguiKey_s(ImGuiKey_KeypadEqual,		"")}, //Unsupported

		// TODO: GAMEPAD SUPPORT

		{ ImGuiKey_MouseWheelDown,		ClientImguiKey_s(ImGuiKey_MouseWheelDown,	"mwheeldown") },
		{ ImGuiKey_MouseWheelUp,		ClientImguiKey_s(ImGuiKey_MouseWheelUp,		"mwheelup") },
		{ ImGuiKey_Mouse1,				ClientImguiKey_s(ImGuiKey_Mouse1,			"mouse1") },
		{ ImGuiKey_Mouse2,				ClientImguiKey_s(ImGuiKey_Mouse2,			"mouse2") },
		{ ImGuiKey_Mouse3,				ClientImguiKey_s(ImGuiKey_Mouse3,			"mouse3") },
		{ ImGuiKey_Mouse4,				ClientImguiKey_s(ImGuiKey_Mouse4,			"mouse4") },
		{ ImGuiKey_Mouse5,				ClientImguiKey_s(ImGuiKey_Mouse5,			"mouse5") },
	};

#pragma warning( default : 26812 )
}

void CClientImguiKeyboardSettings::ParseDefaultConfigFile(std::string filedir)
{
	std::ifstream fstream;
	//fstream.open(filedir);
	FranUtils::Filesystem::OpenInputFile(filedir, fstream);

	int lineIteration = 0;
	std::string line;
	while (std::getline(fstream, line))
	{
		lineIteration++;
		//line.erase(remove_if(line.begin(), line.end(), isspace), line.end()); // Remove whitespace
		gEngfuncs.Con_DPrintf("\n config.cfg - parsing %d", lineIteration);
		if (line.empty()) // Ignore empty lines
		{
			continue;
		}
		else if (line[0] == '/') // Ignore comments
		{
			continue;
		}
		else if (line.substr(0, 5) == "bind ") // Find a binding line
		{
			line = line.erase(0, 5);

			// Hardcoded order :/
			auto words = FranUtils::SplitQuotedWords(line);
			std::string& cmd = words[1];
			std::string& keyname = words[0];

			if (!cmd.empty() && !keyname.empty())
			{
				FranUtils::LowerCase_Ref(cmd);
				FranUtils::LowerCase_Ref(keyname);

				for (auto& keybindCategory : vecKeybindsData)
				{
					for (auto& categoryCommand : keybindCategory.categoryCommands)
					{
						// Bind the key
						if (categoryCommand.command == cmd)
						{
							int keycode = 0;

							// Find the keycode of the key
							for (const auto& [kmapKey, kmapVal] : imguiKeyToGameKeyMap)
							{
								if (FranUtils::LowerCase(kmapVal.gameKeyCode) == keyname || FranUtils::LowerCase(kmapVal.otherGameKeyCode) == keyname)
									keycode = kmapKey;
							}
							
							if (keycode == 0)
							{
								gEngfuncs.Con_DPrintf("\nERR: config.cfg - Can't parse the keycode '%s' in line %d. Are you sure the syntax is correct?\n", keyname.c_str(), lineIteration);
							}
							else if (categoryCommand.primary == 0) // If primary is not bound to the key, bind to primary
							{
								categoryCommand.primary = keycode;
							}
							else
							{
								categoryCommand.alternate = keycode; // Otherwise, bind to alternative key
							}
							//const std::string cmdbuffer = "bind " + GetGameKeyCode(keyCode) + " \"" + command + '"';
							//gEngfuncs.pfnClientCmd(cmdbuffer.c_str());
						}
					}
				}
			}
			else
			{
				gEngfuncs.Con_DPrintf("\nERR: config.cfg - Can't parse the bind line %d. Are you sure the syntax is correct?\n", lineIteration);
			}

			continue;
		}
	}
}

void CClientImguiKeyboardSettings::ParseBindingConfigFile()
{
	std::ifstream fstream;
	//fstream.open(FranUtils::GetModDirectory() + "bindingconfig.cfg");
	FranUtils::Filesystem::OpenInputFile("bindingconfig.cfg", fstream);

	int lineIteration = 0;
	std::string line;
	while (std::getline(fstream, line))
	{
		lineIteration++;
		//line.erase(remove_if(line.begin(), line.end(), isspace), line.end()); // Remove whitespace
		gEngfuncs.Con_DPrintf("\n config.cfg - parsing %d", lineIteration);
		if (line.empty()) // Ignore empty lines
		{
			continue;
		}
		else if (line[0] == '/') // Ignore comments
		{
			continue;
		}
		else if (line.substr(0, 5) == "bind ") // Find a binding line
		{
			line = line.erase(0, 5);

			// Hardcoded order :/
			auto words = FranUtils::SplitQuotedWords(line);
			std::string& cmd = words[1];
			std::string& keyname = words[0];

			if (!cmd.empty() && !keyname.empty())
			{
				FranUtils::LowerCase_Ref(cmd);
				FranUtils::LowerCase_Ref(keyname);

				for (auto& keybindCategory : vecKeybindsData)
				{
					for (auto& categoryCommand : keybindCategory.categoryCommands)
					{
						// Bind the key
						if (categoryCommand.command == cmd)
						{
							int keycode = 0;

							// Find the keycode of the key
							for (const auto& [kmapKey, kmapVal] : imguiKeyToGameKeyMap)
							{
								if (FranUtils::LowerCase(kmapVal.gameKeyCode) == keyname || FranUtils::LowerCase(kmapVal.otherGameKeyCode) == keyname)
									keycode = kmapKey;
							}

							if (keycode == 0)
							{
								gEngfuncs.Con_DPrintf("\nERR: config.cfg - Can't parse the keycode '%s' in line %d. Are you sure the syntax is correct?\n", keyname.c_str(), lineIteration);
							}
							else if (categoryCommand.primary == 0) // If primary is not bound to the key, bind to primary
							{
								categoryCommand.primary = keycode;
							}
							else
							{
								categoryCommand.alternate = keycode; // Otherwise, bind to alternative key
							}
							//const std::string cmdbuffer = "bind " + GetGameKeyCode(keyCode) + " \"" + command + '"';
							//gEngfuncs.pfnClientCmd(cmdbuffer.c_str());
						}
					}
				}
			}
			else
			{
				gEngfuncs.Con_DPrintf("\nERR: config.cfg - Can't parse the bind line %d. Are you sure the syntax is correct?\n", lineIteration);
			}

			continue;
		}
	}
}

void CClientImguiKeyboardSettings::ApplyKeybinds()
{
	for (const auto& val : vecKeybindApplyQueue)
	{
		SetKeybind(val);
	}

	vecKeybindsData = vecKeybindsOriginalData;
	vecKeybindApplyQueue.clear();
}

void CClientImguiKeyboardSettings::CancelKeybinds()
{
	vecKeybindsData = vecKeybindsOriginalData;
	vecKeybindApplyQueue.clear();
}

void CClientImguiKeyboardSettings::AddToKeybindQueue(const SmallKeybind_s kbnd)
{
	vecKeybindApplyQueue.push_back(kbnd);

	// Bind and unbind visibly for temporary reasons
	for (auto& keybindCategory : vecKeybindsData)
	{
		// TODO: FIX THIS!! COULDN'T FIND A BETTER WAY TO PREVENT UNBINDING RIGHT AFTER THE BIND
		for (auto& categoryCommand : keybindCategory.categoryCommands)
		{
			// We hafta unbind the key if we already bound it
			if (categoryCommand.primary == kbnd.keyCode)
			{
				categoryCommand.primary = 0;
			}
			else if (categoryCommand.alternate == kbnd.keyCode)
			{
				categoryCommand.alternate = 0;
			}
		}

		for (auto& categoryCommand : keybindCategory.categoryCommands)
		{
			// Now we can bind the key
			if (categoryCommand.command == kbnd.command)
			{
				// Unbind the old key first
				if (kbnd.alternate)
				{
					categoryCommand.alternate = kbnd.keyCode;
				}
				else
				{
					categoryCommand.primary = kbnd.keyCode;
				}
			}
		}
	}
}

void CClientImguiKeyboardSettings::SetKeybind(const SmallKeybind_s kbnd)
{
	// We can overwrite the old config now

	for (auto& keybindCategory : vecKeybindsOriginalData)
	{
		// TODO: FIX THIS!! COULDN'T FIND A BETTER WAY TO PREVENT UNBINDING RIGHT AFTER THE BIND
		for (auto& categoryCommand : keybindCategory.categoryCommands)
		{
			// We hafta unbind the key if we already bound it
			if (categoryCommand.primary == kbnd.keyCode)
			{
				categoryCommand.primary = 0;
				const std::string cmdbuffer = "unbind " + GetGameKeyCode(kbnd.keyCode);
				gEngfuncs.pfnClientCmd(cmdbuffer.c_str());
			}
			else if (categoryCommand.alternate == kbnd.keyCode)
			{
				categoryCommand.alternate = 0;
				const std::string cmdbuffer = "unbind " + GetGameKeyCode(kbnd.keyCode);
				gEngfuncs.pfnClientCmd(cmdbuffer.c_str());
			}
		}

		for (auto& categoryCommand : keybindCategory.categoryCommands)
		{
			// Now we can bind the key
			if (categoryCommand.command == kbnd.command)
			{

				if (kbnd.alternate)
				{
					// Unbind the old key first
					const std::string cmdbuffer = "unbind " + GetGameKeyCode(categoryCommand.alternate);
					gEngfuncs.pfnClientCmd(cmdbuffer.c_str());
					categoryCommand.alternate = kbnd.keyCode;
				}
				else
				{
					// Unbind the old key first
					const std::string cmdbuffer = "unbind " + GetGameKeyCode(categoryCommand.primary);
					gEngfuncs.pfnClientCmd(cmdbuffer.c_str());
					categoryCommand.primary = kbnd.keyCode;
				}
				const std::string cmdbuffer = "bind " + GetGameKeyCode(kbnd.keyCode) + " \"" + kbnd.command + '"';
				gEngfuncs.pfnClientCmd(cmdbuffer.c_str());
			}
		}
	}
}

std::string CClientImguiKeyboardSettings::GetMappedKeyName(ImGuiKey_ keyCode)
{
	return imguiKeyToGameKeyMap[keyCode].keyVisibleName;
}

std::string CClientImguiKeyboardSettings::GetGameKeyCode(ImGuiKey_ keyCode)
{
	return imguiKeyToGameKeyMap[keyCode].gameKeyCode;
}