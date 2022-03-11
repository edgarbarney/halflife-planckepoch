// FranticDreamer 2022

#ifndef HUD_IMGUI_SETTINGS_KEYBOARD_H
#define HUD_IMGUI_SETTINGS_KEYBOARD_H

struct KeybindKey_s
{
	std::string command;
	std::string displayName;
	ImGuiKey primary;
	ImGuiKey alternate;

	KeybindKey_s(const std::string& commandStr, const std::string& displayNameStr, const ImGuiKey primaryKey = 0, const ImGuiKey alternateKey = 0)
	{
		command = commandStr;
		displayName = displayNameStr;
		primary = primaryKey;
		alternate = alternateKey;
	}
};

struct KeybindCategory_s
{
	std::string categoryName;
	std::vector<KeybindKey_s> categoryCommands;

	KeybindCategory_s()
	{
		categoryName = "Action Category";
	}

	KeybindCategory_s(std::string name)
	{
		categoryName = name;
	}
};

struct ClientImguiKey_s
{
	const ImGuiKey keyType;					// Its also in the map but for future references
	const std::string gameKeyCode;			// Keycode recognised by the engine
	const std::string keyVisibleName;		// For us puny hoomans

	ClientImguiKey_s() : keyType(0), gameKeyCode(""), keyVisibleName(ImGui::GetKeyName(0))
	{
		gEngfuncs.Con_DPrintf("Wrong constructor called for ClientImguiKey_s!!\n");
	};
	ClientImguiKey_s(const ImGuiKey keyID, const std::string& gameKey) : keyType(keyID), gameKeyCode(gameKey), keyVisibleName(ImGui::GetKeyName(keyID)) {};
};

enum class KeyCaptureMode
{
	NoCapture,
	Primary,
	Alternate,
};

class CClientImguiKeyboardSettings
{
public:
	void Init();

	void DrawKeyboardSettingsTab();

	void ParseKeybindFormatData();
	void ParseKeybindData();
	void ParseDefaultConfigFile(); // config.cfg (Engine default)
	void ParseBindingConfigFile(); // bindingconfig.cfg (Custom Fiie)

	void SetKeybind(const std::string& command, ImGuiKey keyCode, const bool alternate = false);
	void SetKeybind(const std::string& command, ImGuiKey_ keyCode, const bool alternate = false) { SetKeybind(command, (ImGuiKey)keyCode, alternate); };


	/*inline*/ std::string GetMappedKeyName(ImGuiKey_ keyCode);
	/*inline*/ std::string GetMappedKeyName(ImGuiKey keyCode) { return GetMappedKeyName((ImGuiKey_)keyCode); };

	/*inline*/ std::string GetGameKeyCode(ImGuiKey_ keyCode);
	/*inline*/ std::string GetGameKeyCode(ImGuiKey keyCode) { return GetGameKeyCode((ImGuiKey_)keyCode); };

	KeyCaptureMode keyCaptureMode; // Are we tring to get key?
	std::string currentKeyCapturingBind; // Current action that we're trying to capture a key for

	std::map<ImGuiKey_, ClientImguiKey_s> imguiKeyToGameKeyMap;
	std::vector<KeybindCategory_s> vecKeybindsData;

};

#endif