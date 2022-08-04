//#pragma once

// TODO: ORGANISE THIS FILE!!!

#ifndef FRANUTILS_H
#define FRANUTILS_H

#include <string>

// ==========================================================
// START OF THE HORRIBLE HORRIBLE PREPROCESSOR HACKS SHIT THAT I HATE
// ==========================================================

#ifdef FRANUTILS_MODDIR

#include <vector>
#include <fstream>
#include <algorithm>

#ifdef _FILESYSTEM_

	#ifdef CLIENT_DLL
	extern cl_enginefunc_t gEngfuncs;
	#define GetGameDir    (*gEngfuncs.pfnGetGameDirectory)
	#else
	extern enginefuncs_t g_engfuncs;
	#define GetGameDir    (*g_engfuncs.pfnGetGameDir)
	#endif


	namespace FranUtils
	{
		inline std::string GetModDirectory(std::string endLine = "\\") //Yes, string
		{
			std::string temp = std::filesystem::current_path().string();
			#ifdef CLIENT_DLL
				const char* getGamedir;
				getGamedir = GetGameDir();
			#else
				char getGamedir[120] = "\0";
				GetGameDir(getGamedir);
			#endif
			temp = temp + "\\" + getGamedir + endLine;
			return temp;
		}
	}

#else

#error "You defined FRANUTILS_MODDIR but you didn't include standard library filesystem header. Please include it in an appropriate place."

#endif // _FILESYSTEM_

#endif // FRANUTILS_ADVANCED

// ==========================================================
// END OF THE HORRIBLE HORRIBLE PREPROCESSOR SHIT THAT I HATE
// ==========================================================

namespace FranUtils
{
#pragma region Constants

	const int Kilobyte = 1024;
	const int Megabyte = 1048576;				//1024 * 1024
	const int Gigabyte = 1073741824;			//1024 * 1024 * 1024
	const long long Terabyte = 1099511627776;	//1024 * 1024 * 1024 * 1024

#pragma endregion

#pragma region Debug Functions
#ifndef CLIENT_WEAPONS
	/**
	* Prints 2 strings with a boolean value inbetween.
	* Can be used as extrapolation, when lerpfactor is outside of the range [0,1].
	*
	* @param firstStr : First string. Unformattable. You can use it to store variable name (Ex: "Foo: ")
	* @param boolSelf : Boolean
	* @param afterStr : Last string. Formattable.
	* @param Args: Standard C printing format args
	*/
	inline void PRINT_BOOL(std::string firstStr, bool boolSelf, std::string afterStr = "\n", ...)
	{
		#ifdef _DEBUG
			va_list args;
			char* tst = const_cast<char*>(afterStr.c_str());
			va_start(args, tst);
			//firstStr = firstStr + ": %s" + afterStr;
			firstStr = firstStr + "%s" + afterStr;
			#ifdef CLIENT_DLL
			gEngfuncs.Con_Printf(firstStr.c_str(), boolSelf ? "true" : "false", args);
			#else
			ALERT(at_console, firstStr.c_str(), boolSelf ? "true" : "false", args);
			#endif
		#else
			return;
		#endif
	}

	inline void PRINT(const char* cStr, ...) 
	{
		#ifdef _DEBUG
			va_list args;
			va_start(args, cStr);
			#ifdef CLIENT_DLL
			gEngfuncs.Con_Printf(cStr, args);
			#else
			ALERT(at_console, cStr, args);
			#endif
			va_end(args);
		#else
			return;
		#endif
	}

	inline void PRINT(std::string cStr, ...)
	{
		#ifdef _DEBUG
			va_list args;
			char* tst = const_cast<char*>(cStr.c_str());
			va_start(args, tst);
			#ifdef CLIENT_DLL
			gEngfuncs.Con_Printf(cStr.c_str(), args);
			#else
			ALERT(at_console, cStr.c_str(), args);
			#endif
			va_end(args);
		#else
			return;
		#endif
	}
#endif
#pragma endregion

#pragma region Non-ensured Math Funcitons

	/**
	* Basic linear interpolation.
	* Can be used as extrapolation, when lerpfactor is outside of the range [0,1].
	*
	* @see FranUtils::Lerp_s
	* @param lerpfactor : Factor of Interpolation
	* @param A : Starting Point
	* @param B : Ending Point
	* @return Interpolated Value
	*/
	inline float Lerp(float lerpfactor, float A, float B) 
	{ 
		return A + lerpfactor * (B - A); 
	}

	/**
	* (C - A) / (B - A) || Calculation of a float in between 2 floats as a decimal in the range [0,1].
	* Can underflow or overflow. return is not clamped.
	*
	* @see FranUtils::WhereInBetween_s
	* @param find : Number to Find
	* @param min : Starting Point (Minimum)
	* @param max : Ending Point (Maximum)
	* @return Found Value
	*/
	inline float WhereInBetween(float find, float min, float max) 
	{ 
		return (find - min) / (max - min); 
	}

#pragma endregion

#pragma region Ensured Math Funcitons

	/**
	* Ensured linear interpolation.
	* - Ensure : Can't be used as extrapolation. lerpfactor is clamped the range [0,1].
	*
	* @see FranUtils::Lerp
	* @param lerpfactor : Factor of Interpolation
	* @param A : Starting Point
	* @param B : Ending Point
	* @return Interpolated Value
	*/
	inline float Lerp_s(float lerpfactor, float A, float B)
	{
		if (lerpfactor > 1) 
		{
			return A + 1 * (B - A);
		}
		else if (lerpfactor < 0)
		{ 
			return A + 0 * (B - A);
		}
		else
		{
			return A + lerpfactor * (B - A);
		}
	}

	/**
	* (C - A) / (B - A) || Ensured calculation of a float in between 2 floats as a decimal in the range [0,1].
	* - Ensure : Can't underflow or overflow. return is clamped the range [0,1].
	*
	* @see FranUtils::WhereInBetween
	* @param find : Number to Find
	* @param min : Starting Point (Minimum)
	* @param max : Ending Point (Maximum)
	* @return Found Value in the range [0,1]
	*/
	inline float WhereInBetween_s(float find, float min, float max)
	{
		float calc = (find - min) / (max - min);
		if (calc > 1)
			return 1;
		else if (calc < 0)
			return 0;
		else
			return calc;
	}


#pragma endregion

#pragma region String Functions

	inline char* strcharstr(const char* mainstr, const char* substr)
	{
		const char* buffer1 = mainstr;
		const char* buffer2 = substr;
		const char* result = *buffer2 == 0 ? mainstr : 0;

		while (*buffer1 != 0 && *buffer2 != 0)
		{
			if (tolower((unsigned char)*buffer1) == tolower((unsigned char)*buffer2))
			{
				if (result == 0)
				{
					result = buffer1;
				}

				buffer2++;
			}
			else
			{
				buffer2 = substr;
				if (result != 0)
				{
					buffer1 = result + 1;
				}

				if (tolower((unsigned char)*buffer1) == tolower((unsigned char)*buffer2))
				{
					result = buffer1;
					buffer2++;
				}
				else
				{
					result = 0;
				}
			}

			buffer1++;
		}

		return *buffer2 == 0 ? (char*)result : 0;
	}

#ifdef _VECTOR_

	// Split quoted tokens into words
	// e.g. 
	// " \"Hello From The Other Side\" " 
	// returns {"Hello From The Other Side"}
	// e.g. 
	// " \"Hello\" \"From\" \"The\" \"Other\" \"Side\" " 
	// returns {"Hello", "From", "The", "Other", "Side"}
	inline std::vector<std::string> SplitQuotedWords(std::string str)
	{
		std::vector<std::string> out;

		std::string tempStr;

		size_t firstQuotePos = 0;
		size_t secondQuotePos = 0;

		do
		{
			firstQuotePos = str.find("\"");
			secondQuotePos = str.find("\"", firstQuotePos+1);

			if (firstQuotePos == std::string::npos || secondQuotePos == std::string::npos)
			{
				break;
			}
			tempStr = std::string(&str[firstQuotePos], &str[secondQuotePos + 1]);

			// Remove first and last characters of the string, which are the quote marks
			tempStr.pop_back(); tempStr.erase(tempStr.begin());

			out.push_back(tempStr);

			str.erase(firstQuotePos, secondQuotePos+1);
		} 
		while (firstQuotePos != std::string::npos);
			
		return out;
	}

#endif

#ifdef _ALGORITHM_

	inline void LowerCase_Ref(std::string& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char _c)
			{ return std::tolower(_c); });
	}

	inline std::string LowerCase(std::string str)
	{
		LowerCase_Ref(str);
		return str;
	}

#else

	// Thanks to Adversus

	template <class...>
	struct _alwaysfalse { static constexpr bool value = false; };

	template <class... Ts>
	void LowerCase_Ref(Ts&&...)
	{
		static_assert(_alwaysfalse<Ts...>::value, "Lowercase Utility called without Algorihtm header. Please use include it");
	}

	template <class... Ts>
	void LowerCase(Ts&&...)
	{
		static_assert(_alwaysfalse<Ts...>::value, "Lowercase Utility called without Algorihtm header. Please use include it");
	}

#endif

#pragma endregion

#pragma region General Utilities

	/**
	* ASSEMBLY | For HL Messages - Returns a long that contains float information
	*
	* @see FranUtils::ftol
	* @param x : Float to store
	* @return Long to send over
	*/
	inline long ftol_asm(float x)
	{
		__asm mov eax, x;
	}

	/**
	* For HL Messages - Returns a long that contains float information
	*
	* @see FranUtils::ftol_asm
	* @param x : Float to store
	* @return Long to send over
	*/
	inline long ftol(float x)
	{
		return *(long*)(&x);
	}

#if defined(ENGINECALLBACK_H) && !defined(CLIENT_DLL)
	/**
	* For Emitting A Dynamic Light
	*
	* @param emitOrigin : Position to Create
	* @param radius : Radius of the Light * 0.1
	* @param colour : Colour of the Light. XYZ ~ RGB
	* @param time : Lifetime
	* @param decay : Decay Time * 0.1
	*/
	inline void EmitDlight(Vector emitOrigin, int radius, Vector colour, float time, int decay)
	{
		MESSAGE_BEGIN(MSG_PVS, gmsgCreateDLight, emitOrigin);
			//WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(emitOrigin.x);	// X
			WRITE_COORD(emitOrigin.y);	// Y
			WRITE_COORD(emitOrigin.z);	// Z
			WRITE_BYTE(radius);			// radius * 0.1
			WRITE_BYTE(colour.x);		// r
			WRITE_BYTE(colour.y);		// g
			WRITE_BYTE(colour.z);		// b
			WRITE_LONG(FranUtils::ftol_asm(time));  //WRITE_BYTE(time);			// time * 10
			WRITE_BYTE(decay);			// decay * 0.1
		MESSAGE_END();
	}
#endif

#if defined(CLIENT_DLL) && defined(CL_ENGFUNCS_DEF)
	inline void PauseMenu()
	{
		gEngfuncs.pfnClientCmd("toggleconsole");
	}
	inline void QutiGame()
	{
		gHUD.m_clImgui.FinishExtension();
		gEngfuncs.pfnClientCmd("quit");
	}
	inline void RestartGame()
	{
		gHUD.m_clImgui.FinishExtension();
		gEngfuncs.pfnClientCmd("_restart");
	}
#endif

#pragma endregion

	class Globals
	{
	private:

		// Liblist Vars

		inline static std::string fallbackDir;	// Fallback dir from
		inline static std::string startMap;		// Starting Map
		inline static std::string trainMap;		// Training Room Start Map
		inline static std::string gameName;		// Game Visible Name Displayed in the Steam and Modlist

		inline static void ParseLiblist()
		{
#ifdef FRANUTILS_MODDIR
			if (std::filesystem::exists(FranUtils::GetModDirectory() + "liblist.gam"))
			{
				std::ifstream fstream;
				fstream.open(FranUtils::GetModDirectory() + "liblist.gam");

				int lineIteration = 0;
				std::string line;
				while (std::getline(fstream, line))
				{
					lineIteration++;
					//line.erase(remove_if(line.begin(), line.end(), isspace), line.end()); // Remove whitespace
					gEngfuncs.Con_DPrintf("\n liblist.gam - parsing %d", lineIteration);
					if (line.empty()) // Ignore empty lines
					{
						continue;
					}
					else if (line[0] == '/') // Ignore comments
					{
						continue;
					}
					else if (line.substr(0, 13) == "fallback_dir ") // Fallback dir line
					{
						line = line.erase(0, 13);
						auto words = SplitQuotedWords(line);
						std::string& fbdir = words[0];

						if (!fbdir.empty())
						{
							LowerCase_Ref(fbdir);
							fallbackDir = fbdir;
						}
					}
					else if (line.substr(0, 9) == "startmap ") // Startmap line
					{
						line = line.erase(0, 9);
						auto words = SplitQuotedWords(line);
						std::string& stMap = words[0];

						if (!stMap.empty())
						{
							LowerCase_Ref(stMap);
							startMap = stMap;
						}
					}
					else if (line.substr(0, 9) == "trainmap ") // Trainmap line
					{
						line = line.erase(0, 9);
						auto words = SplitQuotedWords(line);
						std::string& trMap = words[0];

						if (!trMap.empty())
						{
							LowerCase_Ref(trMap);
							trainMap = trMap;
						}
					}
					else if (line.substr(0, 5) == "game ") // Game line
					{
						line = line.erase(0, 5);
						auto words = SplitQuotedWords(line);
						std::string& gameStr = words[0];

						if (!gameStr.empty())
						{
							gameName = gameStr;
						}
					}
				}
			}
#endif
		}

	public:
		inline static int isPaused; // Is client paused the game?
		inline static float isPausedLastUpdate;
		inline static bool inMainMenu; // Is client in main menu? (Not pause menu)
		inline static bool in3DMainMenu; // Is client in 3D main menu?
		inline static bool called3DMainMenu; // Is client called 3D menu already?
		inline static bool lastInMainMenu;

		inline static void InitGlobals()
		{
			isPaused = true;
			isPausedLastUpdate = 0.0f;
			in3DMainMenu = false;
			inMainMenu = true;
			called3DMainMenu = false;
			lastInMainMenu = false;

			ParseLiblist();
		}

		inline static std::string GetFallbackDir()
		{
			if (fallbackDir.empty())
				return "valve";

			return fallbackDir;
		}

		inline static std::string GetStartMap()
		{
			if (startMap.empty())
				return "c1a0";

			return startMap;
		}

		inline static std::string GetTrainingStartMap()
		{
			if (trainMap.empty())
				return "t1a0";

			return trainMap;
		}

		inline static std::string GetGameDisplayName()
		{
			if (gameName.empty())
				return "Half-Life"; // Replace With Spirinity?

			return gameName;
		}
	};

}

#endif