// FranticDreamer 2022

#ifndef HUD_IMGUI_SETTINGS_KEYBOARD_H
#define HUD_IMGUI_SETTINGS_KEYBOARD_H

struct SmallKeybind_s
{
	const std::string command;
	const ImGuiKey keyCode;
	const bool alternate;

	SmallKeybind_s(const std::string& cmd, const ImGuiKey kcode, const bool alt = false) : command(cmd), keyCode(kcode), alternate(alt) {};
	SmallKeybind_s(const std::string& cmd, const ImGuiKey_ kcode, const bool alt = false) : command(cmd), keyCode((ImGuiKey)kcode), alternate(alt) {};
};

struct KeybindKey_s
{
	std::string command;
	std::string displayName;
	ImGuiKey primary;
	ImGuiKey alternate;

	KeybindKey_s(const std::string& commandStr, const std::string& displayNameStr, const ImGuiKey primaryKey = 0, const ImGuiKey alternateKey = 0)
		: command(commandStr), displayName(displayNameStr), primary(primaryKey),alternate(alternateKey) {};
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
	const std::string otherGameKeyCode;		// Another keycode recognised by the engine (used only for parsing config.cfg)
	const std::string keyVisibleName;		// For us puny hoomans

	ClientImguiKey_s() : keyType(0), gameKeyCode(""), keyVisibleName(ImGui::GetKeyName(0))
	{
		gEngfuncs.Con_DPrintf("Wrong constructor called for ClientImguiKey_s!!\n");
	};
	ClientImguiKey_s(const ImGuiKey keyID, const std::string& gameKey) : keyType(keyID), gameKeyCode(gameKey), keyVisibleName(ImGui::GetKeyName(keyID)) {};
	ClientImguiKey_s(const ImGuiKey keyID, const std::string& gameKey, const std::string& gameKey2) : keyType(keyID), gameKeyCode(gameKey), otherGameKeyCode(gameKey2), keyVisibleName(ImGui::GetKeyName(keyID)) {};
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

	// config.cfg (Engine default)
	void ParseDefaultConfigFile(std::string filedir);

	// bindingconfig.cfg (Custom Fiie)
	void ParseBindingConfigFile(); 

	void ApplyKeybinds();
	void CancelKeybinds();
	void AddToKeybindQueue(const SmallKeybind_s kbnd);
	void SetKeybind(const SmallKeybind_s kbnd);

	/*inline*/ std::string GetMappedKeyName(ImGuiKey_ keyCode);
	/*inline*/ std::string GetMappedKeyName(ImGuiKey keyCode) { return GetMappedKeyName((ImGuiKey_)keyCode); };

	/*inline*/ std::string GetGameKeyCode(ImGuiKey_ keyCode);
	/*inline*/ std::string GetGameKeyCode(ImGuiKey keyCode) { return GetGameKeyCode((ImGuiKey_)keyCode); };

	KeyCaptureMode keyCaptureMode = KeyCaptureMode::NoCapture; // Are we tring to get key?
	std::string currentKeyCapturingBind; // Current action that we're trying to capture a key for

	std::map<ImGuiKey_, ClientImguiKey_s> imguiKeyToGameKeyMap;
	std::vector<KeybindCategory_s> vecKeybindsData;
	std::vector<KeybindCategory_s> vecKeybindsOriginalData; // Used For Reverting
	std::vector<SmallKeybind_s> vecKeybindApplyQueue;

};

#endif