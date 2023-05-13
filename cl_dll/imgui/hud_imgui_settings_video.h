// FranticDreamer 2022

#ifndef HUD_IMGUI_SETTINGS_VIDEO_H
#define HUD_IMGUI_SETTINGS_VIDEO_H

#include <set>
#if _WIN32
#include "WinReg/WinReg.hpp"
#else
#include <stdint.h>

typedef uint32_t DWORD;
#endif

enum class WindowStatus_e : DWORD
{
	BorderlessFullscreen = 0,
	Windowed = 1,
	Fullscreen = 2, // A bit buggy with trinity. Usage is not advised.
};

struct GameResolution_s
{
	const int resolutionIndex;
	const int width;
	const int height;
	const int refreshRate;

	const std::string displayName;

	GameResolution_s(int index, int w, int h, int hz) : resolutionIndex(index), width(w), height(h), refreshRate(hz), displayName(std::to_string(width) + "x" + std::to_string(height) + " " + std::to_string(refreshRate) + "hz") {};

	inline GameResolution_s operator= (const GameResolution_s& src)
	{
		return GameResolution_s(src);
	}

};

inline bool operator== (const GameResolution_s& src, const GameResolution_s& res)
{
	return (src.width == res.width && src.height == res.height && src.refreshRate == res.refreshRate);
}

inline bool operator> (const GameResolution_s& src, const GameResolution_s& res)
{
	int mult = src.width * src.height;
	int resmult = res.width * res.height;
	if (mult == resmult)
		return src.refreshRate > res.refreshRate;
	else
		return mult > resmult;
}

inline bool operator< (const GameResolution_s& src, const GameResolution_s& res)
{
	int mult = src.width * src.height;
	int resmult = res.width * res.height;
	if (mult == resmult)
		return src.refreshRate < res.refreshRate;
	else
		return mult < resmult;
}

struct VideoSettings_s
{
	bool hdModels;
	bool vsync;

	bool grayScale;
	bool postProcessing;
	bool overlapDecals;
	bool shadows;
	bool shadowsFilter;
	bool dynLights;

	float brightness;
	float gamma;

	DWORD colourDepth;
	DWORD screenHeight;
	DWORD screenWidth;
	DWORD refreshRate;
	union
	{
		DWORD windowTypeDword;
		WindowStatus_e windowType;
	};
	
	//inline void operator= (const GameResolution_s& src)
	void SetResolution(const GameResolution_s& src)
	{
		screenHeight = src.height;
		screenWidth = src.width;
		refreshRate = src.refreshRate;
	}
	
};

class CClientImguiVideoSettings
{
private:
#if _WIN32
	winreg::RegKey HalfLifeRegKey;
#endif

	// Video settings that are being controlled live
	VideoSettings_s liveVideoSettings;
	// Video settings that are applied OR loaded from registry.
	VideoSettings_s currentVideoSettings;

	std::vector<GameResolution_s> resolutionVector; // Available resolution data

	std::map<WindowStatus_e, std::string> windowSatutsNameMap; // Window Status Data

	int resComboLiveIndex; // Resolution Combo Box Selected Index
	int resComboCurrentIndex; // The original Resolution Combo Box Selected Index

	int winStatComboLiveIndex; // Window Status Combo Box Selected Index
	int winStatComboCurrentIndex; // The original Window Status Combo Box Selected Index

public:
	void Init();

	void DrawVideoSettingsTab();

	void ApplyVideoSettings();
	void CancelVideoSettings();

	void CheckForBorderless();

	void ShutDown();

	bool isOpen_restartDialog;

};

#endif
