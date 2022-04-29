// FranticDreamer 2022

#ifndef HUD_IMGUI_CONSOLE_H
#define HUD_IMGUI_CONSOLE_H

enum class ConsoleLogType
{
	Message,
	Warning,
	Error,
};

struct ConsoleLog_s
{
	std::string text;
	ConsoleLogType logType;
	Vector colour;
	Vector& color = colour; // For Bloody Muricans
};

class CClientImguiConsole
{

};

#endif