#ifndef HUD_IMGUI_DIALOGBOX_H
#define HUD_IMGUI_DIALOGBOX_H

#include <string>

using DialogButtons_ = unsigned int;

enum DialogButtons : DialogButtons_
{
	None		= 0 << 0,
	OK			= 1 << 0,
	Cancel		= 1 << 1,
	Yes			= 1 << 2,
	No			= 1 << 3,
	Retry		= 1 << 4,
	Restart		= 1 << 5,
	Abort		= 1 << 6,
	Ignore		= 1 << 7,
	Continue	= 1 << 8,
	Quit		= 1 << 9,
};

class CClientImguiDialogBox
{
public:
	// Convert to a signle function in the main client imgui class??
	static DialogButtons DrawDialogBox(std::string title, std::string message, DialogButtons_ dialogButtons, ImVec2 size = ImVec2(0,0), bool forceSize = false);
};

#endif