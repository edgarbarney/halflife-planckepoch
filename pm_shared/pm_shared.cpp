/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/


//Thank you, C++
//Compile time will be buggered but, meh.
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "Platform.h"

#include <assert.h>
#include "mathlib.h"
#include "cdll_dll.h"
#include "const.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_shared.h"
#include "pm_movevars.h"
#include "pm_debug.h"
#include <stdio.h>  // NULL
//#include <string.h> // strcpy
#include <stdlib.h> // atoi
#include <ctype.h>	// isspace

//Shared Structs
#include "pm_structs.h"

#ifdef CLIENT_DLL
#include "cl_dll.h"
#include "cdll_int.h"
extern cl_enginefunc_t gEngfuncs;
#define GetGameDir    (*gEngfuncs.pfnGetGameDirectory)
#else
#include "eiface.h"
extern enginefuncs_t g_engfuncs;
#define GetGameDir    (*g_engfuncs.pfnGetGameDir)
#endif

#ifdef CLIENT_DLL
	// Spectator Mode
	bool iJumpSpectator;
	float vJumpOrigin[3];
	float vJumpAngles[3];
#endif

static int pm_shared_initialized = 0;

#pragma warning(disable : 4305)

typedef enum
{
	mod_brush,
	mod_sprite,
	mod_alias,
	mod_studio
} modtype_t;

playermove_t *pmove = nullptr;

typedef struct
{
	int planenum;
	short children[2]; // negative numbers are contents
} dclipnode_t;

typedef struct mplane_s
{
	Vector normal; // surface normal
	float dist;	   // closest appoach to origin
	byte type;	   // for texture axis selection and fast side tests
	byte signbits; // signx + signy<<1 + signz<<1
	byte pad[2];
} mplane_t;

typedef struct hull_s
{
	dclipnode_t* clipnodes;
	mplane_t* planes;
	int firstclipnode;
	int lastclipnode;
	Vector clip_mins;
	Vector clip_maxs;
} hull_t;

// Ducking time
#define TIME_TO_DUCK 0.4
#define MAX_CLIMB_SPEED 200
#define STUCK_MOVEUP 1
#define STUCK_MOVEDOWN -1
#define STOP_EPSILON 0.1

#define CTEXTURESMAX 512	// max number of textures loaded
#define CBTEXTURENAMEMAX 13 // only load first n chars of name

#define PLAYER_FATAL_FALL_SPEED		1024// approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED	580// approx 20 feet
#define DAMAGE_FOR_FALL_SPEED		(float) 100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )// damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED		200
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

#define PLAYER_LONGJUMP_SPEED 350 // how fast we longjump

#define PLAYER_DUCKING_MULTIPLIER 0.333

// double to float warning
#pragma warning(disable : 4244)
// up / down
#define PITCH 0
// left / right
#define YAW 1
// fall over
#define ROLL 2

#define CONTENTS_CURRENT_0 -9
#define CONTENTS_CURRENT_90 -10
#define CONTENTS_CURRENT_180 -11
#define CONTENTS_CURRENT_270 -12
#define CONTENTS_CURRENT_UP -13
#define CONTENTS_CURRENT_DOWN -14

#define CONTENTS_TRANSLUCENT -15
//LRC
#define CONTENTS_FLYFIELD -17
#define CONTENTS_FLYFIELD_GRAVITY -18
#define CONTENTS_FOG -19

static Vector rgv3tStuckTable[54];
static int rgStuckLast[MAX_PLAYERS][2];

// Texture names
//static char grgszTextureName[CTEXTURESMAX][CBTEXTURENAMEMAX];	
//static char grgchTextureType[CTEXTURESMAX];

bool g_onladder = false;

//Contains texture types
//   TextureTypeName, Texture Type
std::map<std::string, textureType_s> g_TextureTypeMap;

//Contains Step types
//		StepTypeName, Step Type
std::map<std::string, stepType_s> g_StepTypeMap;

//Contains texture names and corresponding types
//		 TextureName, TextureType
//std::map<std::string, std::string> g_TypedTextureMap;

//Contains texture names and corresponding types
//		 TextureName, TextureType
CTextureAndTypesMap g_TypedTextureMap;

//Contains special attributes and corresponding step types
//				SpecialType, StepType
std::map<StepSpecialType, stepType_s> g_SpecialStepMap;

//Contains Texture impact types
std::vector<impactGroupType_s> g_texTypeImpactTypeVector;

/*FORCEINLINE textureType_s& CTextureAndTypesMap::operator[](std::string str)
{
	size_t foundIndex;

	for (auto& [key, value] : textureTypeMap)
	{
		foundIndex = key.find(str);
		if (foundIndex != std::string::npos)
			return value;
	}

	return g_TextureTypeMap["CHAR_TEX_CONCRETE"];
}
*/

/*FORCEINLINE*/ auto CTextureAndTypesMap::insert(std::pair<std::string, textureType_s>&& val)
{
	auto& temp = textureTypeMap.insert(val);
	if (firstmap.texTypeID == INT32_MAX)
		firstmap = temp.second;
	return temp;
}

/*FORCEINLINE*/ int CTextureAndTypesMap::findInMap(std::string name)
{
	transform(name.begin(), name.end(), name.begin(), ::toupper);

	if (textureTypeMap.find(name) == textureTypeMap.end())
	{
		return g_TextureTypeMap["CHAR_TEX_CONCRETE"].texTypeID;
	}
	return textureTypeMap[name].texTypeID;
}


std::string PM_GetModdir(std::string endLine = "\\") //Yes, string
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


void PM_DefaultStepTypes()
{
	// Default things in case the file is not there
	g_StepTypeMap =
	{
	{ "STEP_CONCRETE",			stepType_s(0, false)},		// default step sound
	{ "STEP_METAL",				stepType_s(1, false)},		// metal floor
	{ "STEP_DIRT",				stepType_s(2, false)},		// dirt, sand, rock
	{ "STEP_VENT",				stepType_s(3, false)},		// ventillation duct
	{ "STEP_GRATE",				stepType_s(4, false)},		// metal grating
	{ "STEP_TILE",				stepType_s(5, false)},		// floor tiles
	{ "STEP_SLOSH",				stepType_s(6, false)},		// shallow liquid puddle
	{ "STEP_WADE",				stepType_s(7, false)},		// wading in liquid
	{ "STEP_LADDER",			stepType_s(8, false)},		// climbing ladder
	};
}

void PM_DefaultTextureTypes() 
{
	// Default things in case the file is not there
	g_TextureTypeMap =
	{
	{ "CHAR_TEX_CONCRETE",		textureType_s{0, "C"}},
	{ "CHAR_TEX_METAL",			textureType_s{1, "M"}},
	{ "CHAR_TEX_DIRT",			textureType_s{2, "D"}},
	{ "CHAR_TEX_VENT",			textureType_s{3, "V"}},
	{ "CHAR_TEX_GRATE",			textureType_s{4, "G"}},
	{ "CHAR_TEX_TILE",			textureType_s{5, "T"}},
	{ "CHAR_TEX_SLOSH",			textureType_s{6, "S"}},
	{ "CHAR_TEX_WOOD",			textureType_s{7, "W"}},
	{ "CHAR_TEX_COMPUTER",		textureType_s{8, "P"}},
	{ "CHAR_TEX_GLASS",			textureType_s{9, "Y"}},
	{ "CHAR_TEX_FLESH",			textureType_s{10,"F"}},
	};
}

std::string PM_MapTextureTypeStepType(std::string textureType)
{
	for (const auto& [tKey, tValue] : g_TextureTypeMap)
	{
		if (tValue.texType == textureType)
			return tValue.texStep;
	}
	return "STEP_CONCRETE";
}

std::string PM_MapTextureTypeIDStepTypeName(int textureTypeID)
{
	for (const auto& [tKey, tValue] : g_TextureTypeMap)
	{
		if (tValue.texTypeID == textureTypeID)
			return tValue.texStep;
	}
	return g_TextureTypeMap["STEP_CONCRETE"].texStep;
}

stepType_s PM_MapTextureTypeIDStepType(int textureTypeID)
{
	for (const auto& [tKey, tValue] : g_TextureTypeMap)
	{
		if (tValue.texTypeID == textureTypeID)
			return g_StepTypeMap[tValue.texStep];
	}
	return g_StepTypeMap[g_TextureTypeMap["CHAR_TEX_CONCRETE"].texStep];
}

stepType_s* PM_MapTextureTypeIDStepTypePtr(int textureTypeID)
{
	for (const auto& [tKey, tValue] : g_TextureTypeMap)
	{
		if (tValue.texTypeID == textureTypeID)
			return &g_StepTypeMap[tValue.texStep];
	}
	return &g_StepTypeMap[g_TextureTypeMap["CHAR_TEX_CONCRETE"].texStep];
}

int PM_MapTextureTypeIDStepTypeID(int textureTypeID)
{
	for (const auto& [tKey, tValue] : g_TextureTypeMap)
	{
		if (tValue.texTypeID == textureTypeID)
			return g_StepTypeMap[tValue.texStep].stepNum;
	}
	return g_TextureTypeMap["CHAR_TEX_CONCRETE"].texTypeID;
}

textureType_s* PM_MaterialAliasToTextureTypePtr(std::string alias)
{
	for (const auto& [tKey, tValue] : g_TextureTypeMap)
	{
		if (tValue.texType == alias)
			return &g_TextureTypeMap[tKey];
	}
	return &g_TextureTypeMap["CHAR_TEX_CONCRETE"];
}

textureType_s PM_MaterialAliasToTextureType(std::string alias)
{
	for (const auto& [tKey, tValue] : g_TextureTypeMap)
	{
		if (tValue.texType == alias)
		{
			//auto tt = g_TextureTypeMap[tKey].texType;
			return g_TextureTypeMap[tKey];
		}
	}
	return g_TextureTypeMap["CHAR_TEX_CONCRETE"];
}

std::string PM_GetMaterialNameFromAlias(std::string alias)
{
	for (const auto& [key, value] : g_TextureTypeMap)
	{
		if (value.texType == alias)
			return key;
	}

	return "CHAR_TEX_CONCRETE";
}


void PM_ParseTextureMaterialsFile(std::string path, bool relative = true)
{
	std::ifstream fstream;

	if (relative)
	{
		fstream.open(PM_GetModdir() + path);
		pmove->Con_Printf("\n =========== Parsing file: %s =========== \n", (PM_GetModdir() + path).c_str());
	}
	else
	{
		fstream.open(std::filesystem::current_path().string() + "\\" + path);
		pmove->Con_Printf("\n =========== Parsing file: %s =========== \n", (std::filesystem::current_path().string() + "\\" + path).c_str());
	}

	
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
		else if (line[0] == '#') // Process Special Atrribute
		{
			std::istringstream iss(line);
			std::string command, value;
			if (!(iss >> command >> value)) // "Syntax" check
			{
				// error, skip to nextline
				pmove->Con_DPrintf("\nERR:  %s - Can't parse line %d. Are you sure the syntax is correct?\n\n %s", path.c_str(), lineIteration, line.c_str());
				continue;
			}
			// Recurse texture materials
			if (command == "#include")
			{
				value.pop_back(); value.erase(value.begin()); // Remove first and last characters of the string, which are the quote marks

				PM_ParseTextureMaterialsFile(value, false);
				continue;
			}
		}
		else
		{
			std::istringstream iss(line);
			std::string type, texture;
			if (!(iss >> type >> texture)) // "Syntax" check
			{
				pmove->Con_DPrintf("\nERR:  %s - Can't parse line %d. Are you sure the syntax is correct? \n\n %s", path.c_str(), lineIteration, line.c_str() );
				continue;
			}

			//To Uppercase
			transform(type.begin(), type.end(), type.begin(), ::toupper);
			transform(texture.begin(), texture.end(), texture.begin(), ::toupper);

			//pmove->Con_DPrintf("\nERR:  %s - line %d. Insterted %s - %s \n Line is: %s\n ", path.c_str(), lineIteration, texture.c_str(), PM_MaterialAliasToTextureType(type).texType.c_str(), line.c_str());
			g_TypedTextureMap.insert({ texture, PM_MaterialAliasToTextureType(type)});
			continue;
		}
	}
}

void PM_ParseStepTypesFile()
{
	std::ifstream fstream;
	fstream.open(PM_GetModdir() + "sound\\steptypes.txt");

	std::string lastType;

	StepSpecialType lastSpecialType = StepSpecialType::Default;

	bool inSection = false;
	int stepIteration = 0;
	int lineIteration = 0;
	std::string line;
	while (std::getline(fstream, line))
	{	
		lineIteration++;
		line.erase(remove_if(line.begin(), line.end(), isspace), line.end()); // Remove whitespace

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
			if (lastSpecialType == StepSpecialType::Ladder)
			{
				g_SpecialStepMap[StepSpecialType::Ladder] = g_StepTypeMap[lastType];
			}
			else if (lastSpecialType == StepSpecialType::LiquidFeet)
			{
				g_SpecialStepMap[StepSpecialType::LiquidFeet] = g_StepTypeMap[lastType];
			}
			else if (lastSpecialType == StepSpecialType::LiquidKnee)
			{
				g_SpecialStepMap[StepSpecialType::LiquidKnee] = g_StepTypeMap[lastType];
			}
			else if (lastSpecialType == StepSpecialType::Flesh)
			{
				g_SpecialStepMap[StepSpecialType::Flesh] = g_StepTypeMap[lastType];
			}
			inSection = false;
			lastType = "";
			lastSpecialType = StepSpecialType::Default;
			continue;
		}
		else if (line == "true")
		{
			g_StepTypeMap.insert({ lastType, stepType_s(stepIteration, true) });
			stepIteration++;
			continue;
		}
		else if (line == "false")
		{
			g_StepTypeMap.insert({ lastType, stepType_s(stepIteration, false) });
			stepIteration++;
			continue;
		}
		else if (line[0] == '#') // Process Special Atrribute
		{
			if (line == "#ladder")
			{
				lastSpecialType = StepSpecialType::Ladder;
			}
			else if (line == "#liquidfeet")
			{
				lastSpecialType = StepSpecialType::LiquidFeet;
			}
			else if (line == "#liquidknee")
			{
				lastSpecialType = StepSpecialType::LiquidKnee;
			}
			else if (line == "#flesh")
			{
				lastSpecialType = StepSpecialType::Flesh;
			}
			continue;
		}
		else if (line[0] == '$') // Process variables
		{
			std::string walkKey				= "$walkingVolume=";
			std::string normalKey			= "$normalVolume=";
			std::string walkStepKey			= "$walkingStepTime=";
			std::string normalStepKey		= "$normalStepTime=";
			std::string crouchMultiplierKey	= "$crouchMultiplier=";

			// Go through individual variable keys

			if (line.find(walkKey) != std::string::npos)
			{
				unsigned int pos = line.find(walkKey);
				if (pos != std::string::npos)
				{
					line.erase(pos, walkKey.length());

					g_StepTypeMap[lastType].walkingVolume = atof(line.c_str());
				}
			}
			else if (line.find(normalKey) != std::string::npos)
			{
				unsigned int pos = line.find(normalKey);
				if (pos != std::string::npos)
				{
					line.erase(pos, normalKey.length());

					g_StepTypeMap[lastType].normalVolume = atof(line.c_str());
				}
			}
			else if (line.find(walkStepKey) != std::string::npos)
			{
				unsigned int pos = line.find(walkStepKey);
				if (pos != std::string::npos)
				{
					line.erase(pos, walkStepKey.length());

					g_StepTypeMap[lastType].walkingStepTime = atof(line.c_str());
				}
			}
			else if (line.find(normalStepKey) != std::string::npos)
			{
				unsigned int pos = line.find(normalStepKey);
				if (pos != std::string::npos)
				{
					line.erase(pos, normalStepKey.length());

					g_StepTypeMap[lastType].normalStepTime = atof(line.c_str());
				}
			}
			else if (line.find(crouchMultiplierKey) != std::string::npos)
			{
				unsigned int pos = line.find(crouchMultiplierKey);
				if (pos != std::string::npos)
				{
					line.erase(pos, crouchMultiplierKey.length());

					g_StepTypeMap[lastType].crouchMultiplier = atof(line.c_str());
				}
			}
			continue;
		}
		else if (line[0] == '"')
		{
			line.pop_back(); line.erase(line.begin()); // Remove first and last characters of the string, which are the quote marks

			if (!inSection)
			{
				lastType = line;

				//To uppercase
				transform(lastType.begin(), lastType.end(), lastType.begin(), ::toupper);

				continue;
			}
			else
			{
				g_StepTypeMap[lastType].stepSounds.push_back(line); // Add the sound
				continue;
			}
		}
		else
		{
			pmove->Con_DPrintf("\nERR: steptypes.txt - Can't parse line %d. Are you sure the syntax is correct?", lineIteration);
		}
	}
}

void PM_ParseMaterialImpactsFile()
{
	std::ifstream fstream;
	fstream.open(PM_GetModdir() + "sound\\materialimpacts.txt");

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
			g_texTypeImpactTypeVector.push_back(impactGroupType_s(NOCHECK, NOCHECK, NOCHECK));
			continue;
		}
		else if (line[0] == '}') // Closing braces will terminate the operation
		{
			inSection = false;
			lastType = "";
			stepIteration++;
			continue;
		}
		else if (line[0] == '$') // Process variables
		{
			std::istringstream iss(line);
			std::string key, value;
			if (!(iss >> key >> value)) // "Syntax" check
			{
				// error, skip to nextline
				pmove->Con_DPrintf("\nERR: materialimpacts.txt - Can't parse line %d. Are you sure the syntax is correct?\n\n %s", lineIteration, line.c_str());
				continue;
			}

			if (key == "$RenderMode")
			{
				if (value == "kRenderNormal")
					g_texTypeImpactTypeVector[stepIteration].renderMode = kRenderNormal;
				else if (value == "kRenderTransColor")
					g_texTypeImpactTypeVector[stepIteration].renderMode = kRenderTransColor;
				else if (value == "kRenderTransTexture")
					g_texTypeImpactTypeVector[stepIteration].renderMode = kRenderTransTexture;
				else if (value == "kRenderGlow")
					g_texTypeImpactTypeVector[stepIteration].renderMode = kRenderGlow;
				else if (value == "kRenderTransAlpha")
					g_texTypeImpactTypeVector[stepIteration].renderMode = kRenderTransAlpha;
				else if (value == "kRenderTransAdd")
					g_texTypeImpactTypeVector[stepIteration].renderMode = kRenderTransAdd;
				else
					g_texTypeImpactTypeVector[stepIteration].renderMode = NOCHECK;
					
			}
			else if (key == "$RenderAmt")
			{
				if (value != "NOCHECK")
					g_texTypeImpactTypeVector[stepIteration].renderAmt = atoi(value.c_str());
				else
					g_texTypeImpactTypeVector[stepIteration].classnumber = NOCHECK;
			}
			else if (key == "$Classnumber")
			{
				if (value != "NOCHECK")
					g_texTypeImpactTypeVector[stepIteration].classnumber = atoi(value.c_str());
				else
					g_texTypeImpactTypeVector[stepIteration].classnumber = NOCHECK;
			}
			continue;
		}
		else if (line[0] == '!') // Process NOT variables TODO: NO NEED OR ANOTHER ELSE IF
		{
			std::istringstream iss(line);
			std::string key, value;
			if (!(iss >> key >> value)) // "Syntax" check
			{
				// error, skip to nextline
				pmove->Con_DPrintf("\nERR: materialimpacts.txt - Can't parse line %d. Are you sure the syntax is correct?\n\n %s", lineIteration, line.c_str());
				continue;
			}

			if (key == "!RenderMode")
			{
				if (value == "kRenderNormal")
					g_texTypeImpactTypeVector[stepIteration].renderMode = -1 * kRenderNormal;
				else if (value == "kRenderTransColor")
					g_texTypeImpactTypeVector[stepIteration].renderMode = -1 * kRenderTransColor;
				else if (value == "kRenderTransTexture")
					g_texTypeImpactTypeVector[stepIteration].renderMode = -1 * kRenderTransTexture;
				else if (value == "kRenderGlow")
					g_texTypeImpactTypeVector[stepIteration].renderMode = -1 * kRenderGlow;
				else if (value == "kRenderTransAlpha")
					g_texTypeImpactTypeVector[stepIteration].renderMode = -1 * kRenderTransAlpha;
				else if (value == "kRenderTransAdd")
					g_texTypeImpactTypeVector[stepIteration].renderMode = -1 * kRenderTransAdd;
				else
					g_texTypeImpactTypeVector[stepIteration].renderMode = NOCHECK;

			}
			else if (key == "!RenderAmt")
			{
				if (value != "NOCHECK")
					g_texTypeImpactTypeVector[stepIteration].renderAmt = -1 * atoi(value.c_str());
				else
					g_texTypeImpactTypeVector[stepIteration].classnumber = NOCHECK;
			}
			else if (key == "!Classnumber")
			{
				if (value != "NOCHECK")
					g_texTypeImpactTypeVector[stepIteration].classnumber = -1 * atoi(value.c_str());
				else
					g_texTypeImpactTypeVector[stepIteration].classnumber = NOCHECK;
			}
			continue;
		}
		else if (line[0] == '"')
		{

			if (!inSection)
			{
				lastType = line;
				line.pop_back(); line.erase(line.begin()); // Remove first and last characters of the string, which are the quote marks
				continue;
			}
			else
			{
				std::istringstream iss(line);
				std::string alias, decal, particle;
				if (!(iss >> alias >> decal >> particle)) // "Syntax" check
				{
					// error, skip to nextline
					pmove->Con_DPrintf("\nERR: materialimpacts.txt - Can't parse line %d. Are you sure the syntax is correct?\n\n %s", lineIteration, line.c_str());
					continue;
				}
				
				//To uppercase
				transform(alias.begin(), alias.end(), alias.begin(), ::toupper);

				// Remove first and last characters of the string, which are the quote marks
				alias.pop_back(); alias.erase(alias.begin());
				particle.pop_back(); particle.erase(particle.begin());
				decal.pop_back(); decal.erase(decal.begin());

				g_texTypeImpactTypeVector[stepIteration].impactTypes.push_back(impactType_s(alias,decal,particle));

				continue;
			}
		}
		else
		{
		pmove->Con_DPrintf("\nERR: materialimpacts.txt - Can't parse line %d. Are you sure the syntax is correct?\n\n %s", lineIteration, line.c_str());
		}
	}
}

void PM_ParseMaterialTypesFile()
{
	std::ifstream fstream;
	fstream.open(PM_GetModdir() + "sound\\materialtypes.txt");

	int lineIteration = 0;
	int texTypeIteration = 0;
	std::string line;
	while (std::getline(fstream, line))
	{
		lineIteration++;

		if (line.empty()) // Ignore empty lines
		{
			continue;
		}
		else if (line[0] == '/') // Ignore comments
		{
			continue;
		}
		else if (line[0] == '"')
		{
			std::istringstream iss(line);
			std::string name, textype, steptype;
			float impVol, weapVol, atnVal;
			if (!(iss >> name >> textype >> steptype >> impVol >> weapVol >> atnVal)) // "Syntax" check
			{ 
				// error, skip to nextline
				pmove->Con_DPrintf("\nERR: materialtypes.txt - Can't parse line %d. Are you sure the syntax is correct?", lineIteration);
				continue; 
			} 

			// Remove first and last characters of the string, which are the quote marks
			name.pop_back(); name.erase(name.begin());
			textype.pop_back(); textype.erase(textype.begin());
			steptype.pop_back(); steptype.erase(steptype.begin());

			//To Uppercase
			transform(name.begin(), name.end(), name.begin(), ::toupper);
			transform(textype.begin(), textype.end(), textype.begin(), ::toupper);
			transform(steptype.begin(), steptype.end(), steptype.begin(), ::toupper);

			g_TextureTypeMap.insert({ name, textureType_s(texTypeIteration, textype, steptype, impVol, weapVol, atnVal) });
			texTypeIteration++;
			
		}
		else
		{
			pmove->Con_DPrintf("\nERR: materialtypes.txt - Can't parse line %d. Are you sure the syntax is correct?", lineIteration);
		}
	}
}

/*
std::string PM_FindTextureType(std::string name)
{
	if (g_TypedTextureMap.find(name) == g_TypedTextureMap.end())
	{
		return g_TextureTypeMap["CHAR_TEX_CONCRETE"].texType;
	}
	return g_TypedTextureMap[name];
}
*/

/*
int PM_FindTextureTypeID(std::string name)
{
	if (g_TypedTextureMap.find(name) == g_TypedTextureMap.end())
	{
		return g_TextureTypeMap["CHAR_TEX_CONCRETE"].texTypeID;
	}
	return g_TypedTextureMap[name].texTypeID;
}
*/


int PM_FindTextureTypeID(std::string name)
{
	return g_TypedTextureMap.findInMap(name);
}

int PM_FindStepTypeID(std::string name)
{
	if (g_StepTypeMap.find(name) == g_StepTypeMap.end())
	{
		return g_StepTypeMap["C"].stepNum;
	}
	return g_StepTypeMap[name].stepNum;
}

void PM_PlayGroupSound( const char* szValue, int irand, float fvol )
{
	static char szBuf[128];
	int i;
	for (i = 0; szValue[i]; i++)
	{
		if (szValue[i] == '?')
		{
			strcpy(szBuf, szValue);
			switch (irand)
			{
			// right foot
			case 0: szBuf[i] = '1'; break;
			case 1: szBuf[i] = '3'; break;
			// left foot
			case 2: szBuf[i] = '2'; break;
			case 3: szBuf[i] = '4'; break;
			default: szBuf[i] = '#';
			}
			pmove->PM_PlaySound(CHAN_BODY, szBuf, fvol, ATTN_NORM, 0, PITCH_NORM);
			return;
		}
	}
	pmove->PM_PlaySound(CHAN_BODY, szValue, fvol, ATTN_NORM, 0, PITCH_NORM);
}

void PM_PlayStepSound(stepType_s step, float fvol)
{
	static int iSkipStep = 0;
	int irand = 0;
	Vector hvel;

	pmove->iStepLeft = 0 != pmove->iStepLeft ? 0 : 1;

	if (0 == pmove->runfuncs)
	{
		return;
	}
	
	// FIXME mp_footsteps needs to be a movevar
	if (0 != pmove->multiplayer && 0 == pmove->movevars->footsteps)
		return;

	VectorCopy(pmove->velocity, hvel);
	hvel[2] = 0.0;

	if (0 != pmove->multiplayer && (!g_onladder && Length(hvel) <= 220))
		return;

	// irand - 0,1 for right foot, 2,3 for left foot
	// used to alternate left and right foot
	// FIXME, move to player state

	//for (const auto& [stepKey, stepValue] : g_StepTypeMap)
	//{
	auto stepValue = step;

		if (stepValue.stepSkip)
		{
			if (iSkipStep == 0)
			{
				iSkipStep++;
				//break;
				return;
			}

			if (iSkipStep++ == 3)
			{
				iSkipStep = 0;
			}
		}

	//if (step == stepValue.stepNum)
	//{
			irand = pmove->RandomLong(0, stepValue.stepSounds.size() - 1);
			if (pmove->iStepLeft)
				irand += (irand%2);
			else
				irand += !(irand%2);
		
			if (irand > (int)stepValue.stepSounds.size() - 1) irand -= 2;


		pmove->PM_PlaySound(CHAN_BODY, stepValue.stepSounds[irand].c_str(), fvol, ATTN_NORM, 0, PITCH_NORM);
		 
		//pmove->PM_PlaySound(CHAN_BODY, "player/pl_step1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);
	//}
	//}

	
}	

void PM_PlayStepSound(std::string step, float fvol)
{
	PM_PlayStepSound(g_StepTypeMap[step], fvol);
}


/*
====================
PM_CatagorizeTextureType

Determine texture info for the texture we are standing on.
====================
*/
void PM_CatagorizeTextureType()
{
	Vector start, end;
	const char* pTextureName;

	VectorCopy(pmove->origin, start);
	VectorCopy(pmove->origin, end);

	// Straight down
	end[2] -= 64;

	// Fill in default values, just in case.
	pmove->sztexturename[0] = '\0';
	//pmove->chtexturetype = CHAR_TEX_CONCRETE;
	pmove->iuser4 = 0;

	pTextureName = pmove->PM_TraceTexture(pmove->onground, start, end);
	if (!pTextureName)
		return;

	// strip leading '-0' or '+0~' or '{' or '!'
	if (*pTextureName == '-' || *pTextureName == '+')
		pTextureName += 2;

	if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
		pTextureName++;
	// '}}'

	strcpy(pmove->sztexturename, pTextureName);
	pmove->sztexturename[CBTEXTURENAMEMAX - 1] = 0;

	// get texture type
	//pmove->chtexturetype = PM_FindTextureType( pmove->sztexturename );

	pmove->iuser4 = PM_FindTextureTypeID(pmove->sztexturename);
}

void PM_UpdateStepSound()
{
	float fvol = 1;
	Vector knee;
	Vector feet;
	Vector center;
	float height;
	float speed;
	float velrun;
	float velwalk;
	float flduck;
	stepType_s step;

	if (pmove->flTimeStepSound > 0)
		return;

	if ((pmove->flags & FL_FROZEN) != 0)
		return;

	PM_CatagorizeTextureType();

	speed = Length(pmove->velocity);

	// determine if we are on a ladder
	//The Barnacle Grapple sets the FL_IMMUNE_LAVA flag to indicate that the player is not on a ladder - Solokiller
	const bool fLadder = (pmove->movetype == MOVETYPE_FLY) && !(pmove->flags & FL_IMMUNE_LAVA); // IsOnLadder();

	// UNDONE: need defined numbers for run, walk, crouch, crouch run velocities!!!!
	if ((pmove->flags & FL_DUCKING) != 0 || fLadder)
	{
		velwalk = 60; // These constants should be based on cl_movespeedkey * cl_forwardspeed somehow
		velrun = 80;  // UNDONE: Move walking to server
		flduck = 100;
	}
	else
	{
		velwalk = 120;
		velrun = 210;
		flduck = 0;
	}

	// If we're on a ladder or on the ground, and we're moving fast enough,
	//  play step sound.  Also, if pmove->flTimeStepSound is zero, get the new
	//  sound right away - we just started moving in new level.
	if ((fLadder || (pmove->onground != -1)) &&
		(Length(pmove->velocity) > 0.0) &&
		(speed >= velwalk || 0 == pmove->flTimeStepSound))
	{
		const bool fWalking = speed < velrun;

		VectorCopy(pmove->origin, center);
		VectorCopy(pmove->origin, knee);
		VectorCopy(pmove->origin, feet);

		height = pmove->player_maxs[pmove->usehull][2] - pmove->player_mins[pmove->usehull][2];

		knee[2] = pmove->origin[2] - 0.3 * height;
		feet[2] = pmove->origin[2] - 0.5 * height;


		// find out what we're stepping in or on...
		if (fLadder)
		{
			step = g_SpecialStepMap[StepSpecialType::Ladder];
		}
		else if ( pmove->PM_PointContents ( knee, nullptr ) == CONTENTS_WATER )
		{
			step = g_SpecialStepMap[StepSpecialType::LiquidKnee];
		}
		else if ( pmove->PM_PointContents ( feet, nullptr ) == CONTENTS_WATER )
		{
			step = g_SpecialStepMap[StepSpecialType::LiquidFeet];
		}
		else
		{
			// find texture under player, if different from current texture,
			// get material type
			step = PM_MapTextureTypeIDStepType(pmove->iuser4);
		}

		fvol = fWalking ? step.walkingVolume : step.normalVolume;
		pmove->flTimeStepSound = fWalking ? step.walkingStepTime : step.normalStepTime;
		
		pmove->flTimeStepSound += flduck; // slower step time if ducking

		// play the sound
		// 35% volume if ducking
		if ((pmove->flags & FL_DUCKING) != 0)
		{
			fvol *= step.crouchMultiplier;
		}

		PM_PlayStepSound(step, fvol);
	}
}

/*
================
PM_AddToTouched

Add's the trace result to touch list, if contact is not already in list.
================
*/
bool PM_AddToTouched(pmtrace_t tr, Vector impactvelocity)
{
	int i;

	for (i = 0; i < pmove->numtouch; i++)
	{
		if (pmove->touchindex[i].ent == tr.ent)
			break;
	}
	if (i != pmove->numtouch) // Already in list.
		return false;

	VectorCopy(impactvelocity, tr.deltavelocity);

	if (pmove->numtouch >= MAX_PHYSENTS)
		pmove->Con_DPrintf("Too many entities were touched!\n");

	pmove->touchindex[pmove->numtouch++] = tr;
	return true;
}

/*
================
PM_CheckVelocity

See if the player has a bogus velocity value.
================
*/
void PM_CheckVelocity()
{
	int i;

	//
	// bound velocity
	//
	for (i = 0; i < 3; i++)
	{
		// See if it's bogus.
		if (IS_NAN(pmove->velocity[i]))
		{
			pmove->Con_Printf("PM  Got a NaN velocity %i\n", i);
			pmove->velocity[i] = 0;
		}
		if (IS_NAN(pmove->origin[i]))
		{
			pmove->Con_Printf("PM  Got a NaN origin on %i\n", i);
			pmove->origin[i] = 0;
		}

		// Bound it.
		if (pmove->velocity[i] > pmove->movevars->maxvelocity)
		{
			pmove->Con_DPrintf("PM  Got a velocity too high on %i\n", i);
			pmove->velocity[i] = pmove->movevars->maxvelocity;
		}
		else if (pmove->velocity[i] < -pmove->movevars->maxvelocity)
		{
			pmove->Con_DPrintf("PM  Got a velocity too low on %i\n", i);
			pmove->velocity[i] = -pmove->movevars->maxvelocity;
		}
	}
}

/*
==================
PM_ClipVelocity

Slide off of the impacting object
returns the blocked flags:
0x01 == floor
0x02 == step / wall
==================
*/
int PM_ClipVelocity(Vector in, Vector normal, Vector& out, float overbounce)
{
	float backoff;
	float change;
	float angle;
	int i, blocked;

	angle = normal[2];

	blocked = 0x00;		 // Assume unblocked.
	if (angle > 0)		 // If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01; //
	if (0 == angle)		 // If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02; //

	// Determine how far along plane to slide based on incoming direction.
	// Scale by overbounce factor.
	backoff = DotProduct(in, normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		// If out velocity is too small, zero it out.
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}

	// Return blocking flags.
	return blocked;
}

void PM_AddCorrectGravity()
{
	float ent_gravity;

	if (0 != pmove->waterjumptime)
		return;

	if (0 != pmove->gravity)
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.
	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * 0.5 * pmove->frametime);
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;

	PM_CheckVelocity();
}


void PM_FixupGravityVelocity()
{
	float ent_gravity;

	if (0 != pmove->waterjumptime)
		return;

	if (0 != pmove->gravity)
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;

	// Get the correct velocity for the end of the dt
	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * pmove->frametime * 0.5);

	PM_CheckVelocity();
}

/*
============
PM_FlyMove

The basic solid body movement clip that slides along multiple planes
============
*/
int PM_FlyMove()
{
	int bumpcount, numbumps;
	Vector dir;
	float d;
	int numplanes;
	Vector planes[MAX_CLIP_PLANES];
	Vector primal_velocity, original_velocity;
	Vector new_velocity;
	int i, j;
	pmtrace_t trace;
	Vector end;
	float time_left, allFraction;
	int blocked;

	numbumps = 4; // Bump up to four times

	blocked = 0;									// Assume not blocked
	numplanes = 0;									//  and not sliding along any planes
	VectorCopy(pmove->velocity, original_velocity); // Store original velocity
	VectorCopy(pmove->velocity, primal_velocity);

	allFraction = 0;
	time_left = pmove->frametime; // Total time for this movement operation.

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		if (pmove->velocity == g_vecZero)
			break;

		// Assume we can move all the way from the current origin to the
		//  end point.
		for (i = 0; i < 3; i++)
			end[i] = pmove->origin[i] + time_left * pmove->velocity[i];

		// See if we can make it from origin to end point.
		trace = pmove->PM_PlayerTrace(pmove->origin, end, PM_NORMAL, -1);

		allFraction += trace.fraction;
		// If we started in a solid object, or we were in solid space
		//  the whole way, zero out our velocity and return that we
		//  are blocked by floor and wall.
		if (0 != trace.allsolid)
		{ // entity is trapped in another solid
			VectorCopy(vec3_origin, pmove->velocity);
			//Con_DPrintf("Trapped 4\n");
			return 4;
		}

		// If we moved some portion of the total distance, then
		//  copy the end position into the pmove->origin and
		//  zero the plane counter.
		if (trace.fraction > 0)
		{ // actually covered some distance
			VectorCopy(trace.endpos, pmove->origin);
			VectorCopy(pmove->velocity, original_velocity);
			numplanes = 0;
		}

		// If we covered the entire distance, we are done
		//  and can return.
		if (trace.fraction == 1)
			break; // moved the entire distance

		//if (!trace.ent)
		//	Sys_Error ("PM_PlayerTrace: !trace.ent");

		// Save entity that blocked us (since fraction was < 1.0)
		//  for contact
		// Add it if it's not already in the list!!!
		PM_AddToTouched(trace, pmove->velocity);

		// If the plane we hit has a high z component in the normal, then
		//  it's probably a floor
		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1; // floor
		}
		// If the plane has a zero z component in the normal, then it's a
		//  step or wall
		if (0 == trace.plane.normal[2])
		{
			blocked |= 2; // step / wall
						  //Con_DPrintf("Blocked by %i\n", trace.ent);
		}

		// Reduce amount of pmove->frametime left by total time left * fraction
		//  that we covered.
		time_left -= time_left * trace.fraction;

		// Did we run out of planes to clip against?
		if (numplanes >= MAX_CLIP_PLANES)
		{ // this shouldn't really happen
			//  Stop our movement if so.
			VectorCopy(vec3_origin, pmove->velocity);
			//Con_DPrintf("Too many planes 4\n");

			break;
		}

		// Set up next clipping plane
		VectorCopy(trace.plane.normal, planes[numplanes]);
		numplanes++;
		//

		// modify original_velocity so it parallels all of the clip planes
		//
		if (pmove->movetype == MOVETYPE_WALK &&
			((pmove->onground == -1) || (pmove->friction != 1))) // relfect player velocity
		{
			for (i = 0; i < numplanes; i++)
			{
				if (planes[i][2] > 0.7)
				{ // floor or slope
					PM_ClipVelocity(original_velocity, planes[i], new_velocity, 1);
					VectorCopy(new_velocity, original_velocity);
				}
				else
					PM_ClipVelocity(original_velocity, planes[i], new_velocity, 1.0 + pmove->movevars->bounce * (1 - pmove->friction));
			}

			VectorCopy(new_velocity, pmove->velocity);
			VectorCopy(new_velocity, original_velocity);
		}
		else
		{
			for (i = 0; i < numplanes; i++)
			{
				PM_ClipVelocity(
					original_velocity,
					planes[i],
					pmove->velocity,
					1);
				for (j = 0; j < numplanes; j++)
					if (j != i)
					{
						// Are we now moving against this plane?
						if (DotProduct(pmove->velocity, planes[j]) < 0)
							break; // not ok
					}
				if (j == numplanes) // Didn't have to clip, so we're ok
					break;
			}

			// Did we go all the way through plane set
			if (i != numplanes)
			{ // go along this plane
				// pmove->velocity is set in clipping call, no need to set again.
				;
			}
			else
			{ // go along the crease
				if (numplanes != 2)
				{
					//Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
					VectorCopy(vec3_origin, pmove->velocity);
					//Con_DPrintf("Trapped 4\n");

					break;
				}
				CrossProduct(planes[0], planes[1], dir);
				d = DotProduct(dir, pmove->velocity);
				VectorScale(dir, d, pmove->velocity);
			}

			//
			// if original velocity is against the original velocity, stop dead
			// to avoid tiny occilations in sloping corners
			//
			if (DotProduct(pmove->velocity, primal_velocity) <= 0)
			{
				//Con_DPrintf("Back\n");
				VectorCopy(vec3_origin, pmove->velocity);
				break;
			}
		}
	}

	if (allFraction == 0)
	{
		VectorCopy(vec3_origin, pmove->velocity);
		//Con_DPrintf( "Don't stick\n" );
	}

	return blocked;
}

/*
==============
PM_Accelerate
==============
*/
void PM_Accelerate(Vector wishdir, float wishspeed, float accel)
{
	int i;
	float addspeed, accelspeed, currentspeed;

	// Dead player's don't accelerate
	if (0 != pmove->dead)
		return;

	// If waterjumping, don't accelerate
	if (0 != pmove->waterjumptime)
		return;

	// See if we are changing direction a bit
	currentspeed = DotProduct(pmove->velocity, wishdir);

	// Reduce wishspeed by the amount of veer.
	addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if (addspeed <= 0)
		return;

	// Determine amount of accleration.
	accelspeed = accel * pmove->frametime * wishspeed * pmove->friction;

	// Cap at addspeed
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust velocity.
	for (i = 0; i < 3; i++)
	{
		pmove->velocity[i] += accelspeed * wishdir[i];
	}
}

/*
=====================
PM_WalkMove

Only used by players.  Moves along the ground when player is a MOVETYPE_WALK.
======================
*/
void PM_WalkMove()
{
	int clip;
	int oldonground;
	int i;

	Vector wishvel;
	float spd;
	float fmove, smove;
	Vector wishdir;
	float wishspeed;

	Vector dest, start;
	Vector original, originalvel;
	Vector down, downvel;
	float downdist, updist;

	pmtrace_t trace;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2] = 0;

	VectorNormalize(pmove->forward); // Normalize remainder of vectors.
	VectorNormalize(pmove->right);	 //

	for (i = 0; i < 2; i++) // Determine x and y parts of velocity
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;

	wishvel[2] = 0; // Zero out z part of velocity

	VectorCopy(wishvel, wishdir); // Determine maginitude of speed of move
	wishspeed = VectorNormalize(wishdir);

	//
	// Clamp to server defined max speed
	//
	if (wishspeed > pmove->maxspeed)
	{
		VectorScale(wishvel, pmove->maxspeed / wishspeed, wishvel);
		wishspeed = pmove->maxspeed;
	}

	// Set pmove velocity
	pmove->velocity[2] = 0;
	PM_Accelerate(wishdir, wishspeed, pmove->movevars->accelerate);
	pmove->velocity[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd(pmove->velocity, pmove->basevelocity, pmove->velocity);

	spd = Length(pmove->velocity);

	if (spd < 1.0f)
	{
		VectorClear(pmove->velocity);
		return;
	}

	// If we are not moving, do nothing
	//if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
	//	return;

	oldonground = pmove->onground;

	// first try just moving to the destination
	dest[0] = pmove->origin[0] + pmove->velocity[0] * pmove->frametime;
	dest[1] = pmove->origin[1] + pmove->velocity[1] * pmove->frametime;
	dest[2] = pmove->origin[2];

	// first try moving directly to the next spot
	VectorCopy(dest, start);
	trace = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);
	// If we made it all the way, then copy trace end
	//  as new player position.
	if (trace.fraction == 1)
	{
		VectorCopy(trace.endpos, pmove->origin);
		return;
	}

	if (oldonground == -1 && // Don't walk up stairs if not on ground.
		(pmove->waterlevel == 0 || pmove->watertype == CONTENT_FOG))
		return;

	if (0 != pmove->waterjumptime) // If we are jumping out of water, don't do anything more.
		return;

	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	VectorCopy(pmove->origin, original);	  // Save out original pos &
	VectorCopy(pmove->velocity, originalvel); //  velocity.

	// Slide move
	clip = PM_FlyMove();

	// Copy the results out
	VectorCopy(pmove->origin, down);
	VectorCopy(pmove->velocity, downvel);

	// Reset original values.
	VectorCopy(original, pmove->origin);

	VectorCopy(originalvel, pmove->velocity);

	// Start out up one stair height
	VectorCopy(pmove->origin, dest);
	dest[2] += pmove->movevars->stepsize;

	trace = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);
	// If we started okay and made it part of the way at least,
	//  copy the results to the movement start position and then
	//  run another move try.
	if (0 == trace.startsolid && 0 == trace.allsolid)
	{
		VectorCopy(trace.endpos, pmove->origin);
	}

	// slide move the rest of the way.
	clip = PM_FlyMove();

	// Now try going back down from the end point
	//  press down the stepheight
	VectorCopy(pmove->origin, dest);
	dest[2] -= pmove->movevars->stepsize;

	trace = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);

	// If we are not on the ground any more then
	//  use the original movement attempt
	if (trace.plane.normal[2] < 0.7)
		goto usedown;
	// If the trace ended up in empty space, copy the end
	//  over to the origin.
	if (0 == trace.startsolid && 0 == trace.allsolid)
	{
		VectorCopy(trace.endpos, pmove->origin);
	}
	// Copy this origion to up.
	VectorCopy(pmove->origin, pmove->up);

	// decide which one went farther
	downdist = (down[0] - original[0]) * (down[0] - original[0]) + (down[1] - original[1]) * (down[1] - original[1]);
	updist = (pmove->up[0] - original[0]) * (pmove->up[0] - original[0]) + (pmove->up[1] - original[1]) * (pmove->up[1] - original[1]);

	if (downdist > updist)
	{
	usedown:
		VectorCopy(down, pmove->origin);
		VectorCopy(downvel, pmove->velocity);
	}
	else // copy z value from slide move
		pmove->velocity[2] = downvel[2];
}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void PM_Friction()
{
	float* vel;
	float speed, newspeed, control;
	float friction;
	float drop;
	Vector newvel;

	// If we are in water jump cycle, don't apply friction
	if (0 != pmove->waterjumptime)
		return;

	// Get velocity
	vel = pmove->velocity;

	// Calculate speed
	speed = sqrt(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);

	// If too slow, return
	if (speed < 0.1f)
	{
		return;
	}

	drop = 0;

	// apply ground friction
	if (pmove->onground != -1) // On an entity that is the ground
	{
		Vector start, stop;
		pmtrace_t trace;

		start[0] = stop[0] = pmove->origin[0] + vel[0] / speed * 16;
		start[1] = stop[1] = pmove->origin[1] + vel[1] / speed * 16;
		start[2] = pmove->origin[2] + pmove->player_mins[pmove->usehull][2];
		stop[2] = start[2] - 34;

		trace = pmove->PM_PlayerTrace(start, stop, PM_NORMAL, -1);

		if (trace.fraction == 1.0)
			friction = pmove->movevars->friction * pmove->movevars->edgefriction;
		else
			friction = pmove->movevars->friction;

		// Grab friction value.
		//friction = pmove->movevars->friction;

		friction *= pmove->friction; // player friction?

		// Bleed off some speed, but if we have less than the bleed
		//  threshhold, bleed the theshold amount.
		control = (speed < pmove->movevars->stopspeed) ? pmove->movevars->stopspeed : speed;
		// Add the amount to t'he drop amount.
		drop += control * friction * pmove->frametime;
	}

	// apply water friction
	//	if (pmove->waterlevel)
	//		drop += speed * pmove->movevars->waterfriction * waterlevel * pmove->frametime;

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	// Determine proportion of old speed we are using.
	newspeed /= speed;

	// Adjust velocity according to proportion.
	newvel[0] = vel[0] * newspeed;
	newvel[1] = vel[1] * newspeed;
	newvel[2] = vel[2] * newspeed;

	VectorCopy(newvel, pmove->velocity);
}

void PM_AirAccelerate(Vector wishdir, float wishspeed, float accel)
{
	int i;
	float addspeed, accelspeed, currentspeed, wishspd = wishspeed;

	if (0 != pmove->dead)
		return;
	if (0 != pmove->waterjumptime)
		return;

	// Cap speed
	//wishspd = VectorNormalize (pmove->wishveloc);

	if (wishspd > 30)
		wishspd = 30;
	// Determine veer amount
	currentspeed = DotProduct(pmove->velocity, wishdir);
	// See how much to add
	addspeed = wishspd - currentspeed;
	// If not adding any, done.
	if (addspeed <= 0)
		return;
	// Determine acceleration speed after acceleration

	accelspeed = accel * wishspeed * pmove->frametime * pmove->friction;
	// Cap it
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust pmove vel.
	for (i = 0; i < 3; i++)
	{
		pmove->velocity[i] += accelspeed * wishdir[i];
	}
}

/*
===================
PM_WaterMove

===================
*/
void PM_WaterMove()
{
	int i;
	Vector wishvel;
	float wishspeed;
	Vector wishdir;
	Vector start, dest;
	Vector temp;
	pmtrace_t trace;

	float speed, newspeed, addspeed, accelspeed;

	//
	// user intentions
	//
	for (i = 0; i < 3; i++)
		wishvel[i] = pmove->forward[i] * pmove->cmd.forwardmove + pmove->right[i] * pmove->cmd.sidemove;

	// Sinking after no other movement occurs
	if (0 == pmove->cmd.forwardmove && 0 == pmove->cmd.sidemove && 0 == pmove->cmd.upmove && pmove->watertype != CONTENT_FLYFIELD) //LRC
		wishvel[2] -= 60;																										   // drift towards bottom
	else																														   // Go straight up by upmove amount.
		wishvel[2] += pmove->cmd.upmove;

	// Copy it over and determine speed
	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	// Cap speed.
	if (wishspeed > pmove->maxspeed)
	{
		VectorScale(wishvel, pmove->maxspeed / wishspeed, wishvel);
		wishspeed = pmove->maxspeed;
	}
	// Slow us down a bit.
	wishspeed *= 0.8;

	VectorAdd(pmove->velocity, pmove->basevelocity, pmove->velocity);
	// Water friction
	VectorCopy(pmove->velocity, temp);
	speed = VectorNormalize(temp);
	if (0 != speed)
	{
		newspeed = speed - pmove->frametime * speed * pmove->movevars->friction * pmove->friction;

		if (newspeed < 0)
			newspeed = 0;
		VectorScale(pmove->velocity, newspeed / speed, pmove->velocity);
	}
	else
		newspeed = 0;

	//
	// water acceleration
	//
	if (wishspeed < 0.1f)
	{
		return;
	}

	addspeed = wishspeed - newspeed;
	if (addspeed > 0)
	{

		VectorNormalize(wishvel);
		accelspeed = pmove->movevars->accelerate * wishspeed * pmove->frametime * pmove->friction;
		if (accelspeed > addspeed)
			accelspeed = addspeed;

		for (i = 0; i < 3; i++)
			pmove->velocity[i] += accelspeed * wishvel[i];
	}

	// Now move
	// assume it is a stair or a slope, so press down from stepheight above
	VectorMA(pmove->origin, pmove->frametime, pmove->velocity, dest);
	VectorCopy(dest, start);
	start[2] += pmove->movevars->stepsize + 1;
	trace = pmove->PM_PlayerTrace(start, dest, PM_NORMAL, -1);
	if (0 == trace.startsolid && 0 == trace.allsolid) // FIXME: check steep slope?
	{												  // walked up the step, so just keep result and exit
		VectorCopy(trace.endpos, pmove->origin);
		return;
	}

	// Try moving straight along out normal path.
	PM_FlyMove();
}


/*
===================
PM_AirMove

===================
*/
void PM_AirMove()
{
	int i;
	Vector wishvel;
	float fmove, smove;
	Vector wishdir;
	float wishspeed;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2] = 0;
	// Renormalize
	VectorNormalize(pmove->forward);
	VectorNormalize(pmove->right);

	// Determine x and y parts of velocity
	for (i = 0; i < 2; i++)
	{
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
	}
	// Zero out z part of velocity
	wishvel[2] = 0;

	// Determine maginitude of speed of move
	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	// Clamp to server defined max speed
	if (wishspeed > pmove->maxspeed)
	{
		VectorScale(wishvel, pmove->maxspeed / wishspeed, wishvel);
		wishspeed = pmove->maxspeed;
	}

	PM_AirAccelerate(wishdir, wishspeed, pmove->movevars->airaccelerate);

	// Add in any base velocity to the current velocity.
	VectorAdd(pmove->velocity, pmove->basevelocity, pmove->velocity);

	PM_FlyMove();
}

bool PM_InWater()
{
	return (pmove->waterlevel > 1 && pmove->watertype != CONTENT_FOG);
}

/*
=============
PM_CheckWater

Sets pmove->waterlevel and pmove->watertype values.
=============
*/
bool PM_CheckWater()
{
	Vector point;
	int cont;
	int truecont;
	float height;
	float heightover2;

	// Pick a spot just above the players feet.
	point[0] = pmove->origin[0] + (pmove->player_mins[pmove->usehull][0] + pmove->player_maxs[pmove->usehull][0]) * 0.5;
	point[1] = pmove->origin[1] + (pmove->player_mins[pmove->usehull][1] + pmove->player_maxs[pmove->usehull][1]) * 0.5;
	point[2] = pmove->origin[2] + pmove->player_mins[pmove->usehull][2] + 1;

	// Assume that we are not in water at all.
	pmove->waterlevel = 0;
	pmove->watertype = CONTENTS_EMPTY;

	// Grab point contents.
	cont = pmove->PM_PointContents(point, &truecont);
	// Are we under water? (not solid and not empty?)
	if ((cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT) || (cont >= CONTENTS_FOG && cont <= CONTENTS_FLYFIELD))
	{
		// Set water type
		pmove->watertype = cont;

		// We are at least at level one
		pmove->waterlevel = 1;

		height = (pmove->player_mins[pmove->usehull][2] + pmove->player_maxs[pmove->usehull][2]);
		heightover2 = height * 0.5;

		// Now check a point that is at the player hull midpoint.
		point[2] = pmove->origin[2] + heightover2;
		cont = pmove->PM_PointContents (point, nullptr );
		// If that point is also under water...
		if ((cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT) || (cont >= CONTENTS_FOG && cont <= CONTENTS_FLYFIELD))
		{
			// Set a higher water level.
			pmove->waterlevel = 2;

			// Now check the eye position.  (view_ofs is relative to the origin)
			point[2] = pmove->origin[2] + pmove->view_ofs[2];

			cont = pmove->PM_PointContents (point, nullptr );
			if ((cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT ) || cont == CONTENTS_FOG) // Flyfields never cover the eyes
				pmove->waterlevel = 3;  // In over our eyes
		}

		// Adjust velocity based on water current, if any.
		if ((truecont <= CONTENTS_CURRENT_0) &&
			(truecont >= CONTENTS_CURRENT_DOWN))
		{
			// The deeper we are, the stronger the current.
			static Vector current_table[] =
				{
					{1, 0, 0}, {0, 1, 0}, {-1, 0, 0},
					{0, -1, 0}, {0, 0, 1}, {0, 0, -1}};

			VectorMA(pmove->basevelocity, 50.0 * pmove->waterlevel, current_table[CONTENTS_CURRENT_0 - truecont], pmove->basevelocity);
		}
	}

	return pmove->waterlevel > 1;
}

/*
=============
PM_CatagorizePosition
=============
*/
void PM_CatagorizePosition()
{
	Vector point;
	pmtrace_t tr;

	// if the player hull point one unit down is solid, the player
	// is on ground

	// see if standing on something solid

	// Doing this before we move may introduce a potential latency in water detection, but
	// doing it after can get us stuck on the bottom in water if the amount we move up
	// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
	// this several times per frame, so we really need to avoid sticking to the bottom of
	// water on each call, and the converse case will correct itself if called twice.
	PM_CheckWater();

	point[0] = pmove->origin[0];
	point[1] = pmove->origin[1];
	point[2] = pmove->origin[2] - 2;

	if (pmove->velocity[2] > 180) // Shooting up really fast.  Definitely not on ground.
	{
		pmove->onground = -1;
	}
	else
	{
		// Try and move down.
		tr = pmove->PM_PlayerTrace(pmove->origin, point, PM_NORMAL, -1);
		// If we hit a steep plane, we are not on ground
		if (tr.plane.normal[2] < 0.7)
			pmove->onground = -1; // too steep
		else
			pmove->onground = tr.ent; // Otherwise, point to index of ent under us.

		// If we are on something...
		if (pmove->onground != -1)
		{
			// Then we are not in water jump sequence
			pmove->waterjumptime = 0;
			// If we could make the move, drop us down that 1 pixel
			if (pmove->waterlevel < 2 && 0 == tr.startsolid && 0 == tr.allsolid)
				VectorCopy(tr.endpos, pmove->origin);
		}

		// Standing on an entity other than the world
		if (tr.ent > 0) // So signal that we are touching something.
		{
			PM_AddToTouched(tr, pmove->velocity);
		}
	}
}

/*
=================
PM_GetRandomStuckOffsets

When a player is stuck, it's costly to try and unstick them
Grab a test offset for the player based on a passed in index
=================
*/
int PM_GetRandomStuckOffsets(int nIndex, int server, Vector& offset)
{
	// Last time we did a full
	int idx;
	idx = rgStuckLast[nIndex][server]++;

	VectorCopy(rgv3tStuckTable[idx % 54], offset);

	return (idx % 54);
}

void PM_ResetStuckOffsets(int nIndex, int server)
{
	rgStuckLast[nIndex][server] = 0;
}

/*
=================
NudgePosition

If pmove->origin is in a solid position,
try nudging slightly on all axis to
allow for the cut precision of the net coordinates
=================
*/
#define PM_CHECKSTUCK_MINTIME 0.05 // Don't check again too quickly.

bool PM_CheckStuck()
{
	Vector base;
	Vector offset;
	Vector test;
	int hitent;
	int idx;
	float fTime;
	int i;
	pmtrace_t traceresult;

	static float rgStuckCheckTime[MAX_PLAYERS][2]; // Last time we did a full

	// If position is okay, exit
	hitent = pmove->PM_TestPlayerPosition(pmove->origin, &traceresult);
	if (hitent == -1)
	{
		PM_ResetStuckOffsets(pmove->player_index, pmove->server);
		return false;
	}

	VectorCopy(pmove->origin, base);

	//
	// Deal with precision error in network.
	//
	if (0 == pmove->server)
	{
		// World or BSP model
		if ( ( hitent == 0 ) ||
			 ( pmove->physents[hitent].model != nullptr ) )
		{
			int nReps = 0;
			PM_ResetStuckOffsets(pmove->player_index, pmove->server);
			do
			{
				i = PM_GetRandomStuckOffsets(pmove->player_index, pmove->server, offset);

				VectorAdd(base, offset, test);
				if (pmove->PM_TestPlayerPosition(test, &traceresult) == -1)
				{
					PM_ResetStuckOffsets(pmove->player_index, pmove->server);

					VectorCopy(test, pmove->origin);
					return false;
				}
				nReps++;
			} while (nReps < 54);
		}
	}

	// Only an issue on the client.

	if (0 != pmove->server)
		idx = 0;
	else
		idx = 1;

	fTime = pmove->Sys_FloatTime();
	// Too soon?
	if (rgStuckCheckTime[pmove->player_index][idx] >=
		(fTime - PM_CHECKSTUCK_MINTIME))
	{
		return true;
	}
	rgStuckCheckTime[pmove->player_index][idx] = fTime;

	pmove->PM_StuckTouch(hitent, &traceresult);

	i = PM_GetRandomStuckOffsets(pmove->player_index, pmove->server, offset);

	VectorAdd(base, offset, test);
	if ( ( hitent = pmove->PM_TestPlayerPosition ( test, nullptr ) ) == -1 )
	{
		//Con_DPrintf("Nudged\n");

		PM_ResetStuckOffsets(pmove->player_index, pmove->server);

		if (i >= 27)
			VectorCopy(test, pmove->origin);

		return false;
	}

	// If player is flailing while stuck in another player ( should never happen ), then see
	//  if we can't "unstick" them forceably.
	if ((pmove->cmd.buttons & (IN_JUMP | IN_DUCK | IN_ATTACK)) != 0 && (pmove->physents[hitent].player != 0))
	{
		float x, y, z;
		float xystep = 8.0;
		float zstep = 18.0;
		float xyminmax = xystep;
		float zminmax = 4 * zstep;

		for (z = 0; z <= zminmax; z += zstep)
		{
			for (x = -xyminmax; x <= xyminmax; x += xystep)
			{
				for (y = -xyminmax; y <= xyminmax; y += xystep)
				{
					VectorCopy(base, test);
					test[0] += x;
					test[1] += y;
					test[2] += z;

					if ( pmove->PM_TestPlayerPosition ( test, nullptr ) == -1 )
					{
						VectorCopy(test, pmove->origin);
						return false;
					}
				}
			}
		}
	}

	//VectorCopy (base, pmove->origin);

	return true;
}

/*
===============
PM_SpectatorMove
===============
*/
void PM_SpectatorMove()
{
	float speed, drop, friction, control, newspeed;
	//float   accel;
	float currentspeed, addspeed, accelspeed;
	int i;
	Vector wishvel;
	float fmove, smove;
	Vector wishdir;
	float wishspeed;
	// this routine keeps track of the spectators psoition
	// there a two different main move types : track player or moce freely (OBS_ROAMING)
	// doesn't need excate track position, only to generate PVS, so just copy
	// targets position and real view position is calculated on client (saves server CPU)

	if (pmove->iuser1 == OBS_ROAMING)
	{

#ifdef CLIENT_DLL
		// jump only in roaming mode
		if (iJumpSpectator)
		{
			VectorCopy(vJumpOrigin, pmove->origin);
			VectorCopy(vJumpAngles, pmove->angles);
			VectorCopy(vec3_origin, pmove->velocity);
			iJumpSpectator = false;
			return;
		}
#endif
		// Move around in normal spectator method

		speed = Length(pmove->velocity);
		if (speed < 1)
		{
			VectorCopy(vec3_origin, pmove->velocity)
		}
		else
		{
			drop = 0;

			friction = pmove->movevars->friction * 1.5; // extra friction
			control = speed < pmove->movevars->stopspeed ? pmove->movevars->stopspeed : speed;
			drop += control * friction * pmove->frametime;

			// scale the velocity
			newspeed = speed - drop;
			if (newspeed < 0)
				newspeed = 0;
			newspeed /= speed;

			VectorScale(pmove->velocity, newspeed, pmove->velocity);
		}

		// accelerate
		fmove = pmove->cmd.forwardmove;
		smove = pmove->cmd.sidemove;

		VectorNormalize(pmove->forward);
		VectorNormalize(pmove->right);

		for (i = 0; i < 3; i++)
		{
			wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
		}
		wishvel[2] += pmove->cmd.upmove;

		VectorCopy(wishvel, wishdir);
		wishspeed = VectorNormalize(wishdir);

		//
		// clamp to server defined max speed
		//
		if (wishspeed > pmove->movevars->spectatormaxspeed)
		{
			VectorScale(wishvel, pmove->movevars->spectatormaxspeed / wishspeed, wishvel);
			wishspeed = pmove->movevars->spectatormaxspeed;
		}

		currentspeed = DotProduct(pmove->velocity, wishdir);
		addspeed = wishspeed - currentspeed;
		if (addspeed <= 0)
			return;

		accelspeed = pmove->movevars->accelerate * pmove->frametime * wishspeed;
		if (accelspeed > addspeed)
			accelspeed = addspeed;

		for (i = 0; i < 3; i++)
			pmove->velocity[i] += accelspeed * wishdir[i];

		// move
		VectorMA(pmove->origin, pmove->frametime, pmove->velocity, pmove->origin);
	}
	else
	{
		// all other modes just track some kind of target, so spectator PVS = target PVS

		int target;

		// no valid target ?
		if (pmove->iuser2 <= 0)
			return;

		// Find the client this player's targeting
		for (target = 0; target < pmove->numphysent; target++)
		{
			if (pmove->physents[target].info == pmove->iuser2)
				break;
		}

		if (target == pmove->numphysent)
			return;

		// use targets position as own origin for PVS
		VectorCopy(pmove->physents[target].angles, pmove->angles);
		VectorCopy(pmove->physents[target].origin, pmove->origin);

		// no velocity
		VectorCopy(vec3_origin, pmove->velocity);
	}
}

/*
==================
PM_SplineFraction

Use for ease-in, ease-out style interpolation (accel/decel)
Used by ducking code.
==================
*/
float PM_SplineFraction(float value, float scale)
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void PM_FixPlayerCrouchStuck(int direction)
{
	int hitent;
	int i;
	Vector test;

	hitent = pmove->PM_TestPlayerPosition ( pmove->origin, nullptr );
	if (hitent == -1 )
		return;

	VectorCopy(pmove->origin, test);
	for (i = 0; i < 36; i++)
	{
		pmove->origin[2] += direction;
		hitent = pmove->PM_TestPlayerPosition ( pmove->origin, nullptr );
		if (hitent == -1 )
			return;
	}

	VectorCopy(test, pmove->origin); // Failed
}

void PM_UnDuck()
{
	int i;
	pmtrace_t trace;
	Vector newOrigin;

	VectorCopy(pmove->origin, newOrigin);

	if (pmove->onground != -1)
	{
		for (i = 0; i < 3; i++)
		{
			newOrigin[i] += (pmove->player_mins[1][i] - pmove->player_mins[0][i]);
		}
	}

	trace = pmove->PM_PlayerTrace(newOrigin, newOrigin, PM_NORMAL, -1);

	if (0 == trace.startsolid)
	{
		pmove->usehull = 0;

		// Oh, no, changing hulls stuck us into something, try unsticking downward first.
		trace = pmove->PM_PlayerTrace(newOrigin, newOrigin, PM_NORMAL, -1);
		if (0 != trace.startsolid)
		{
			// See if we are stuck?  If so, stay ducked with the duck hull until we have a clear spot
			//Con_Printf( "unstick got stuck\n" );
			pmove->usehull = 1;
			return;
		}

		pmove->flags &= ~FL_DUCKING;
		pmove->bInDuck = 0;
		pmove->view_ofs = VEC_VIEW;
		pmove->flDuckTime = 0;

		VectorCopy(newOrigin, pmove->origin);

		// Recatagorize position since ducking can change origin
		PM_CatagorizePosition();
	}
}

void PM_Duck()
{
	int i;
	float time;
	float duckFraction;

	int buttonsChanged = (pmove->oldbuttons ^ pmove->cmd.buttons); // These buttons have changed this frame
	int nButtonPressed = buttonsChanged & pmove->cmd.buttons;	   // The changed ones still down are "pressed"

	bool duckchange = (buttonsChanged & IN_DUCK) != 0;
	bool duckpressed = (nButtonPressed & IN_DUCK) != 0;

	if ((pmove->cmd.buttons & IN_DUCK) != 0)
	{
		pmove->oldbuttons |= IN_DUCK;
	}
	else
	{
		pmove->oldbuttons &= ~IN_DUCK;
	}

	// Prevent ducking if the iuser3 variable is set
	if (0 != pmove->iuser3 || 0 != pmove->dead)
	{
		// Try to unduck
		if ((pmove->flags & FL_DUCKING) != 0)
		{
			PM_UnDuck();
		}
		return;
	}

	if ((pmove->flags & FL_DUCKING) != 0)
	{
		pmove->cmd.forwardmove *= PLAYER_DUCKING_MULTIPLIER;
		pmove->cmd.sidemove *= PLAYER_DUCKING_MULTIPLIER;
		pmove->cmd.upmove *= PLAYER_DUCKING_MULTIPLIER;
	}

	if ((pmove->cmd.buttons & IN_DUCK) != 0 || (0 != pmove->bInDuck) || (pmove->flags & FL_DUCKING) != 0)
	{
		if ((pmove->cmd.buttons & IN_DUCK) != 0)
		{
			if ((nButtonPressed & IN_DUCK) != 0 && (pmove->flags & FL_DUCKING) == 0)
			{
				// Use 1 second so super long jump will work
				pmove->flDuckTime = 1000;
				pmove->bInDuck = 1;
			}

			time = V_max(0.0, (1.0 - (float)pmove->flDuckTime / 1000.0));

			if (0 != pmove->bInDuck)
			{
				// Finish ducking immediately if duck time is over or not on ground
				if (((float)pmove->flDuckTime / 1000.0 <= (1.0 - TIME_TO_DUCK)) ||
					(pmove->onground == -1))
				{
					pmove->usehull = 1;
					pmove->view_ofs = VEC_DUCK_VIEW;
					pmove->flags |= FL_DUCKING;
					pmove->bInDuck = 0;

					// HACKHACK - Fudge for collision bug - no time to fix this properly
					if (pmove->onground != -1)
					{
						for (i = 0; i < 3; i++)
						{
							pmove->origin[i] -= (pmove->player_mins[1][i] - pmove->player_mins[0][i]);
						}
						// See if we are stuck?
						PM_FixPlayerCrouchStuck(STUCK_MOVEUP);

						// Recatagorize position since ducking can change origin
						PM_CatagorizePosition();
					}
				}
				else
				{
					float fMore = (VEC_DUCK_HULL_MIN[2] - VEC_HULL_MIN[2]);

					// Calc parametric time
					duckFraction = PM_SplineFraction(time, (1.0 / TIME_TO_DUCK));
					pmove->view_ofs[2] = ((VEC_DUCK_VIEW[2] - fMore) * duckFraction) + (VEC_VIEW[2] * (1 - duckFraction));
				}
			}
		}
		else
		{
			// Try to unduck
			PM_UnDuck();
		}
	}
}

void PM_LadderMove(physent_t* pLadder)
{
	Vector ladderCenter;
	trace_t trace;
	Vector floor;
	Vector modelmins, modelmaxs;
	bool onFloor;

	if (pmove->movetype == MOVETYPE_NOCLIP)
		return;


	pmove->PM_GetModelBounds(pLadder->model, modelmins, modelmaxs);

	VectorAdd(modelmins, modelmaxs, ladderCenter);
	VectorScale(ladderCenter, 0.5, ladderCenter);
	VectorAdd(ladderCenter, pLadder->origin, ladderCenter); //LRC- allow for ladders moving around

	pmove->movetype = MOVETYPE_FLY;

	// On ladder, convert movement to be relative to the ladder

	VectorCopy(pmove->origin, floor);
	floor[2] += pmove->player_mins[pmove->usehull][2] - 1;

	if ( pmove->PM_PointContents( floor, nullptr ) == CONTENTS_SOLID )
		onFloor = true;
	else
		onFloor = false;

	pmove->gravity = 0;
	pmove->PM_TraceModel(pLadder, pmove->origin, ladderCenter, &trace);
	if (trace.fraction != 1.0)
	{
		float forward = 0, right = 0;
		Vector vpn, v_right;
		float flSpeed = MAX_CLIMB_SPEED;

		// they shouldn't be able to move faster than their maxspeed
		if (flSpeed > pmove->maxspeed)
		{
			flSpeed = pmove->maxspeed;
		}

		AngleVectors( pmove->angles, &vpn, &v_right, nullptr );

		if ((pmove->flags & FL_DUCKING) != 0)
		{
			flSpeed *= PLAYER_DUCKING_MULTIPLIER;
		}

		if ((pmove->cmd.buttons & IN_BACK) != 0)
		{
			forward -= flSpeed;
		}
		if ((pmove->cmd.buttons & IN_FORWARD) != 0)
		{
			forward += flSpeed;
		}
		if ((pmove->cmd.buttons & IN_MOVELEFT) != 0)
		{
			right -= flSpeed;
		}
		if ((pmove->cmd.buttons & IN_MOVERIGHT) != 0)
		{
			right += flSpeed;
		}

		if ((pmove->cmd.buttons & IN_JUMP) != 0)
		{
			pmove->movetype = MOVETYPE_WALK;
			VectorScale(trace.plane.normal, 270, pmove->velocity);
		}
		else
		{
			if (forward != 0 || right != 0)
			{
				Vector velocity, perp, cross, lateral, tmp;
				float normal;

				//ALERT(at_console, "pev %.2f %.2f %.2f - ",
				//	pev->velocity.x, pev->velocity.y, pev->velocity.z);
				// Calculate player's intended velocity
				//Vector velocity = (forward * gpGlobals->v_forward) + (right * gpGlobals->v_right);
				VectorScale(vpn, forward, velocity);
				VectorMA(velocity, right, v_right, velocity);


				// Perpendicular in the ladder plane
				//					Vector perp = CrossProduct( Vector(0,0,1), trace.vecPlaneNormal );
				//					perp = perp.Normalize();
				VectorClear(tmp);
				tmp[2] = 1;
				CrossProduct(tmp, trace.plane.normal, perp);
				VectorNormalize(perp);


				// decompose velocity into ladder plane
				normal = DotProduct(velocity, trace.plane.normal);
				// This is the velocity into the face of the ladder
				VectorScale(trace.plane.normal, normal, cross);


				// This is the player's additional velocity
				VectorSubtract(velocity, cross, lateral);

				// This turns the velocity into the face of the ladder into velocity that
				// is roughly vertically perpendicular to the face of the ladder.
				// NOTE: It IS possible to face up and move down or face down and move up
				// because the velocity is a sum of the directional velocity and the converted
				// velocity through the face of the ladder -- by design.
				CrossProduct(trace.plane.normal, perp, tmp);
				VectorMA(lateral, -normal, tmp, pmove->velocity);
				if (onFloor && normal > 0) // On ground moving away from the ladder
				{
					VectorMA(pmove->velocity, MAX_CLIMB_SPEED, trace.plane.normal, pmove->velocity);
				}
				//pev->velocity = lateral - (CrossProduct( trace.vecPlaneNormal, perp ) * normal);
			}
			else
			{
				VectorClear(pmove->velocity);
			}
		}
	}
}

physent_t* PM_Ladder()
{
	int i;
	physent_t* pe;
	hull_t* hull;
	int num;
	Vector test;

	for (i = 0; i < pmove->nummoveent; i++)
	{
		pe = &pmove->moveents[i];

		if (pe->model && (modtype_t)pmove->PM_GetModelType(pe->model) == mod_brush && pe->skin == CONTENTS_LADDER)
		{

			hull = (hull_t*)pmove->PM_HullForBsp(pe, test);
			num = hull->firstclipnode;

			// Offset the test point appropriately for this hull.
			VectorSubtract(pmove->origin, test, test);

			// Test the player's hull for intersection with this model
			if (pmove->PM_HullPointContents(hull, num, test) == CONTENTS_EMPTY)
				continue;

			return pe;
		}
	}

	return nullptr;
}



void PM_WaterJump()
{
	if (pmove->waterjumptime > 10000)
	{
		pmove->waterjumptime = 10000;
	}

	if (0 == pmove->waterjumptime)
		return;

	pmove->waterjumptime -= pmove->cmd.msec;
	if (pmove->waterjumptime < 0 ||
		0 == pmove->waterlevel)
	{
		pmove->waterjumptime = 0;
		pmove->flags &= ~FL_WATERJUMP;
	}

	pmove->velocity[0] = pmove->movedir[0];
	pmove->velocity[1] = pmove->movedir[1];
}

/*
============
PM_AddGravity

============
*/
void PM_AddGravity()
{
	float ent_gravity;

	if (0 != pmove->gravity)
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;

	// Add gravity incorrectly
	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * pmove->frametime);
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;
	PM_CheckVelocity();
}
/*
============
PM_PushEntity

Does not change the entities velocity at all
============
*/
pmtrace_t PM_PushEntity(Vector push)
{
	pmtrace_t trace;
	Vector end;

	VectorAdd(pmove->origin, push, end);

	trace = pmove->PM_PlayerTrace(pmove->origin, end, PM_NORMAL, -1);

	VectorCopy(trace.endpos, pmove->origin);

	// So we can run impact function afterwards.
	if (trace.fraction < 1.0 &&
		0 == trace.allsolid)
	{
		PM_AddToTouched(trace, pmove->velocity);
	}

	return trace;
}

/*
============
PM_Physics_Toss()

Dead player flying through air., e.g.
============
*/
void PM_Physics_Toss()
{
	pmtrace_t trace;
	Vector move;
	float backoff;

	PM_CheckWater();

	if (pmove->velocity[2] > 0)
		pmove->onground = -1;

	// If on ground and not moving, return.
	if (pmove->onground != -1)
	{
		if (VectorCompare(pmove->basevelocity, vec3_origin) &&
			VectorCompare(pmove->velocity, vec3_origin))
			return;
	}

	PM_CheckVelocity();

	// add gravity
	if (pmove->movetype != MOVETYPE_FLY &&
		pmove->movetype != MOVETYPE_BOUNCEMISSILE &&
		pmove->movetype != MOVETYPE_FLYMISSILE)
		PM_AddGravity();

	// move origin
	// Base velocity is not properly accounted for since this entity will move again after the bounce without
	// taking it into account
	VectorAdd(pmove->velocity, pmove->basevelocity, pmove->velocity);

	PM_CheckVelocity();
	VectorScale(pmove->velocity, pmove->frametime, move);
	VectorSubtract(pmove->velocity, pmove->basevelocity, pmove->velocity);

	trace = PM_PushEntity(move); // Should this clear basevelocity

	PM_CheckVelocity();

	if (0 != trace.allsolid)
	{
		// entity is trapped in another solid
		pmove->onground = trace.ent;
		VectorCopy(vec3_origin, pmove->velocity);
		return;
	}

	if (trace.fraction == 1)
	{
		PM_CheckWater();
		return;
	}


	if (pmove->movetype == MOVETYPE_BOUNCE)
		backoff = 2.0 - pmove->friction;
	else if (pmove->movetype == MOVETYPE_BOUNCEMISSILE)
		backoff = 2.0;
	else
		backoff = 1;

	PM_ClipVelocity(pmove->velocity, trace.plane.normal, pmove->velocity, backoff);

	// stop if on ground
	if (trace.plane.normal[2] > 0.7)
	{
		float vel;
		Vector base;

		VectorClear(base);
		if (pmove->velocity[2] < pmove->movevars->gravity * pmove->frametime)
		{
			// we're rolling on the ground, add static friction.
			pmove->onground = trace.ent;
			pmove->velocity[2] = 0;
		}

		vel = DotProduct(pmove->velocity, pmove->velocity);

		// Con_DPrintf("%f %f: %.0f %.0f %.0f\n", vel, trace.fraction, ent->velocity[0], ent->velocity[1], ent->velocity[2] );

		if (vel < (30 * 30) || (pmove->movetype != MOVETYPE_BOUNCE && pmove->movetype != MOVETYPE_BOUNCEMISSILE))
		{
			pmove->onground = trace.ent;
			VectorCopy(vec3_origin, pmove->velocity);
		}
		else
		{
			VectorScale(pmove->velocity, (1.0 - trace.fraction) * pmove->frametime * 0.9, move);
			trace = PM_PushEntity(move);
		}
		VectorSubtract(pmove->velocity, base, pmove->velocity)
	}

	// check for in water
	PM_CheckWater();
}

/*
====================
PM_NoClip

====================
*/
void PM_NoClip()
{
	int i;
	Vector wishvel;
	float fmove, smove;
	//	float		currentspeed, addspeed, accelspeed;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	VectorNormalize(pmove->forward);
	VectorNormalize(pmove->right);

	for (i = 0; i < 3; i++) // Determine x and y parts of velocity
	{
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
	}
	wishvel[2] += pmove->cmd.upmove;

	VectorMA(pmove->origin, pmove->frametime, wishvel, pmove->origin);

	// Zero out the velocity so that we don't accumulate a huge downward velocity from
	//  gravity, etc.
	VectorClear(pmove->velocity);
}

// Only allow bunny jumping up to 1.7x server / player maxspeed setting
#define BUNNYJUMP_MAX_SPEED_FACTOR 1.7f

//-----------------------------------------------------------------------------
// Purpose: Corrects bunny jumping ( where player initiates a bunny jump before other
//  movement logic runs, thus making onground == -1 thus making PM_Friction get skipped and
//  running PM_AirMove, which doesn't crop velocity to maxspeed like the ground / other
//  movement logic does.
//-----------------------------------------------------------------------------
void PM_PreventMegaBunnyJumping()
{
	// Current player speed
	float spd;
	// If we have to crop, apply this cropping fraction to velocity
	float fraction;
	// Speed at which bunny jumping is limited
	float maxscaledspeed;

	maxscaledspeed = BUNNYJUMP_MAX_SPEED_FACTOR * pmove->maxspeed;

	// Don't divide by zero
	if (maxscaledspeed <= 0.0f)
		return;

	spd = Length(pmove->velocity);

	if (spd <= maxscaledspeed)
		return;

	fraction = (maxscaledspeed / spd) * 0.65; //Returns the modifier for the velocity

	VectorScale(pmove->velocity, fraction, pmove->velocity); //Crop it down!.
}

/*
=============
PM_Jump
=============
*/
void PM_Jump()
{
	int i;

	//qboolean cansuperjump = false;

	auto sSounds = g_SpecialStepMap[StepSpecialType::LiquidKnee];

	if (pmove->dead)
	{
		pmove->oldbuttons |= IN_JUMP; // don't jump again until released
		return;
	}

	const bool tfc = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "tfc")) == 1;

	// Spy that's feigning death cannot jump
	if (tfc &&
		(pmove->deadflag == (DEAD_DISCARDBODY + 1)))
	{
		return;
	}

	// See if we are waterjumping.  If so, decrement count and return.
	if (0 != pmove->waterjumptime)
	{
		pmove->waterjumptime -= pmove->cmd.msec;
		if (pmove->waterjumptime < 0)
		{
			pmove->waterjumptime = 0;
		}
		return;
	}

	// If we are in the water most of the way...
	if (pmove->waterlevel >= 2 && pmove->watertype != CONTENTS_FOG)
	{ // swimming, not jumping
		pmove->onground = -1;

		if (pmove->watertype == CONTENTS_WATER) // We move up a certain amount
			pmove->velocity[2] = 100;
		else if (pmove->watertype == CONTENTS_SLIME)
			pmove->velocity[2] = 80;
		else // LAVA
			pmove->velocity[2] = 50;

		// play swiming sound
		if (pmove->flSwimTime <= 0)
		{
			// Don't play sound again for 1 second
			pmove->flSwimTime = 1000;

			int rnd = pmove->RandomLong(0, sSounds.stepSounds.size() - 1);

			pmove->PM_PlaySound(CHAN_BODY, sSounds.stepSounds[rnd].c_str(), 1, ATTN_NORM, 0, PITCH_NORM);
		}

		return;
	}

	// No more effect
	if (pmove->onground == -1)
	{
		// Flag that we jumped.
		// HACK HACK HACK
		// Remove this when the game .dll no longer does physics code!!!!
		pmove->oldbuttons |= IN_JUMP; // don't jump again until released
		return;						  // in air, so no effect
	}

	if ((pmove->oldbuttons & IN_JUMP) != 0)
		return; // don't pogo stick

	// In the air now.
	pmove->onground = -1;

	PM_PreventMegaBunnyJumping();

	if (tfc)
	{
		pmove->PM_PlaySound(CHAN_BODY, "player/plyrjmp8.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);
	}
	else
	{
		PM_PlayStepSound( PM_MapTextureTypeIDStepType( pmove->iuser4 ), 1.0 );
	}

	// See if user can super long jump?
	const bool cansuperjump = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "slj")) == 1;

	const bool canjumppackjump = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "jpj")) == 1;

	// Acclerate upward
	// If we are ducking...
	if ((0 != pmove->bInDuck) || (pmove->flags & FL_DUCKING) != 0)
	{
		// Adjust for super long jump module
		// UNDONE -- note this should be based on forward angles, not current velocity.
		if ((cansuperjump || canjumppackjump) &&
			(pmove->cmd.buttons & IN_DUCK) != 0 &&
			(pmove->flDuckTime > 0) &&
			Length(pmove->velocity) > 50)
		{
			pmove->punchangle[0] = -5;

			for (i = 0; i < 2; i++)
			{
				pmove->velocity[i] = pmove->forward[i] * PLAYER_LONGJUMP_SPEED * 1.6;
			}

			pmove->velocity[2] = sqrt(2 * 800 * 56.0);
		}
		else
		{
			pmove->velocity[2] = sqrt(2 * 800 * 45.0);
		}
	}
	else
	{
		pmove->velocity[2] = sqrt(2 * 800 * 45.0);
	}

	// Decay it for simulation
	PM_FixupGravityVelocity();

	// Flag that we jumped.
	pmove->oldbuttons |= IN_JUMP; // don't jump again until released
}

/*
=============
PM_CheckWaterJump
=============
*/
#define WJ_HEIGHT 8
void PM_CheckWaterJump()
{
	Vector vecStart, vecEnd;
	Vector flatforward;
	Vector flatvelocity;
	float curspeed;
	pmtrace_t tr;
	int savehull;

	// Already water jumping.
	if (0 != pmove->waterjumptime)
		return;

	// Don't hop out if we just jumped in
	if (pmove->velocity[2] < -180)
		return; // only hop out if we are moving up

	// See if we are backing up
	flatvelocity[0] = pmove->velocity[0];
	flatvelocity[1] = pmove->velocity[1];
	flatvelocity[2] = 0;

	// Must be moving
	curspeed = VectorNormalize(flatvelocity);

	// see if near an edge
	flatforward[0] = pmove->forward[0];
	flatforward[1] = pmove->forward[1];
	flatforward[2] = 0;
	VectorNormalize(flatforward);

	// Are we backing into water from steps or something?  If so, don't pop forward
	if (curspeed != 0.0 && (DotProduct(flatvelocity, flatforward) < 0.0))
		return;

	VectorCopy(pmove->origin, vecStart);
	vecStart[2] += WJ_HEIGHT;

	VectorMA(vecStart, 24, flatforward, vecEnd);

	// Trace, this trace should use the point sized collision hull
	savehull = pmove->usehull;
	pmove->usehull = 2;
	tr = pmove->PM_PlayerTrace(vecStart, vecEnd, PM_NORMAL, -1);
	if (tr.fraction < 1.0 && fabs(tr.plane.normal[2]) < 0.1f) // Facing a near vertical wall?
	{
		vecStart[2] += pmove->player_maxs[savehull][2] - WJ_HEIGHT;
		VectorMA(vecStart, 24, flatforward, vecEnd);
		VectorMA(vec3_origin, -50, tr.plane.normal, pmove->movedir);

		tr = pmove->PM_PlayerTrace(vecStart, vecEnd, PM_NORMAL, -1);
		if (tr.fraction == 1.0)
		{
			pmove->waterjumptime = 2000;
			pmove->velocity[2] = 225;
			pmove->oldbuttons |= IN_JUMP;
			pmove->flags |= FL_WATERJUMP;
		}
	}

	// Reset the collision hull
	pmove->usehull = savehull;
}

void PM_CheckFalling()
{
	if (pmove->onground != -1 &&
		0 == pmove->dead &&
		pmove->flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD)
	{
		float fvol = 0.5;

		if (pmove->waterlevel > 0)
		{
		}
		else if (pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED)
		{
			// NOTE:  In the original game dll , there were no breaks after these cases, causing the first one to
			// cascade into the second
			//switch ( RandomLong(0,1) )
			//{
			//case 0:
			//pmove->PM_PlaySound( CHAN_VOICE, "player/pl_fallpain2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			//break;
			//case 1:
			pmove->PM_PlaySound(CHAN_VOICE, "player/pl_fallpain3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			//	break;
			//}
			fvol = 1.0;
		}
		else if (pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED / 2)
		{
			const bool tfc = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "tfc")) == 1;

			if (tfc)
			{
				pmove->PM_PlaySound(CHAN_VOICE, "player/pl_fallpain3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			}

			fvol = 0.85;
		}
		else if (pmove->flFallVelocity < PLAYER_MIN_BOUNCE_SPEED)
		{
			fvol = 0;
		}

		if (fvol > 0.0)
		{
			// Play landing step right away
			pmove->flTimeStepSound = 0;

			PM_UpdateStepSound();

			// play step sound for current texture
			PM_PlayStepSound(PM_MapTextureTypeIDStepType(pmove->iuser4), fvol);

			// Knock the screen around a little bit, temporary effect
			pmove->punchangle[2] = pmove->flFallVelocity * 0.013; // punch z axis

			if (pmove->punchangle[0] > 8)
			{
				pmove->punchangle[0] = 8;
			}
		}
	}

	if (pmove->onground != -1)
	{
		pmove->flFallVelocity = 0;
	}
}

/*
=================
PM_PlayWaterSounds

=================
*/
void PM_PlayWaterSounds()
{
	// Did we enter or leave water?
	if ((pmove->oldwaterlevel == 0 && pmove->waterlevel != 0 && pmove->watertype > CONTENT_FLYFIELD) ||
		(pmove->oldwaterlevel != 0 && pmove->waterlevel == 0))
	{
		auto sSounds = g_SpecialStepMap[StepSpecialType::LiquidKnee];

		int rnd = pmove->RandomLong(0, sSounds.stepSounds.size() - 1);

		pmove->PM_PlaySound(CHAN_BODY, sSounds.stepSounds[rnd].c_str(), 1, ATTN_NORM, 0, PITCH_NORM);

	}
}

/*
===============
PM_CalcRoll

===============
*/
float PM_CalcRoll(Vector angles, Vector velocity, float rollangle, float rollspeed)
{
	float sign;
	float side;
	float value;
	Vector forward, right, up;

	AngleVectors(angles, &forward, &right, &up);

	side = DotProduct(velocity, right);

	sign = side < 0 ? -1 : 1;

	side = fabs(side);

	value = rollangle;

	if (side < rollspeed)
	{
		side = side * value / rollspeed;
	}
	else
	{
		side = value;
	}

	return side * sign;
}

/*
=============
PM_DropPunchAngle

=============
*/
void PM_DropPunchAngle(Vector& punchangle)
{
	float len;

	len = VectorNormalize(punchangle);
	len -= (10.0 + len * 0.5) * pmove->frametime;
	len = V_max(len, 0.0);
	VectorScale(punchangle, len, punchangle);
}

/*
==============
PM_CheckParamters

==============
*/
void PM_CheckParamters()
{
	float spd;
	float maxspeed;
	Vector v_angle;

	spd = (pmove->cmd.forwardmove * pmove->cmd.forwardmove) +
		  (pmove->cmd.sidemove * pmove->cmd.sidemove) +
		  (pmove->cmd.upmove * pmove->cmd.upmove);
	spd = sqrt(spd);

	maxspeed = pmove->clientmaxspeed; //atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "maxspd" ) );
	if (maxspeed != 0.0)
	{
		pmove->maxspeed = V_min(maxspeed, pmove->maxspeed);
	}

	if ((spd != 0.0) &&
		(spd > pmove->maxspeed))
	{
		float fRatio = pmove->maxspeed / spd;
		pmove->cmd.forwardmove *= fRatio;
		pmove->cmd.sidemove *= fRatio;
		pmove->cmd.upmove *= fRatio;
	}

	if ((pmove->flags & FL_FROZEN) != 0 || 0 != pmove->dead)
	{
		pmove->cmd.forwardmove = 0;
		pmove->cmd.sidemove = 0;
		pmove->cmd.upmove = 0;
		pmove->cmd.buttons = 0; // LRC - no jump sounds when frozen!
	}

	if (pmove->flags & FL_ONTRAIN)
	{
		pmove->cmd.forwardmove = 0;
		pmove->cmd.sidemove = 0;
		pmove->cmd.upmove = 0;
		//G-Cont Save LRC fix and return tracktrain ducking, if tracktrain "oncontrols"
	}


	PM_DropPunchAngle(pmove->punchangle);

	// Take angles from command.
	if (0 == pmove->dead)
	{
		VectorCopy(pmove->cmd.viewangles, v_angle);
		VectorAdd(v_angle, pmove->punchangle, v_angle);

		// Set up view angles.
		pmove->angles[ROLL] = PM_CalcRoll(v_angle, pmove->velocity, pmove->movevars->rollangle, pmove->movevars->rollspeed) * 4;
		pmove->angles[PITCH] = v_angle[PITCH];
		pmove->angles[YAW] = v_angle[YAW];
	}
	else
	{
		VectorCopy(pmove->oldangles, pmove->angles);
	}

	// Set dead player view_offset
	if (0 != pmove->dead)
	{
		pmove->view_ofs = VEC_DEAD_VIEW;
	}

	// Adjust client view angles to match values used on server.
	if (pmove->angles[YAW] > 180.0f)
	{
		pmove->angles[YAW] -= 360.0f;
	}
}

void PM_ReduceTimers()
{
	if (pmove->flTimeStepSound > 0)
	{
		pmove->flTimeStepSound -= pmove->cmd.msec;
		if (pmove->flTimeStepSound < 0)
		{
			pmove->flTimeStepSound = 0;
		}
	}
	if (pmove->flDuckTime > 0)
	{
		pmove->flDuckTime -= pmove->cmd.msec;
		if (pmove->flDuckTime < 0)
		{
			pmove->flDuckTime = 0;
		}
	}
	if (pmove->flSwimTime > 0)
	{
		pmove->flSwimTime -= pmove->cmd.msec;
		if (pmove->flSwimTime < 0)
		{
			pmove->flSwimTime = 0;
		}
	}
}

/*
=============
PlayerMove

Returns with origin, angles, and velocity modified in place.

Numtouch and touchindex[] will be set if any of the physents
were contacted during the move.
=============
*/
void PM_PlayerMove(qboolean server)
{
	physent_t *pLadder = nullptr;

	// Are we running server code?
	pmove->server = server;

	// Adjust speeds etc.
	PM_CheckParamters();

	// Assume we don't touch anything
	pmove->numtouch = 0;

	// # of msec to apply movement
	pmove->frametime = pmove->cmd.msec * 0.001;

	PM_ReduceTimers();

	// Convert view angles to vectors
	AngleVectors(pmove->angles, &pmove->forward, &pmove->right, &pmove->up);

	// PM_ShowClipBox();

	// Special handling for spectator and observers. (iuser1 is set if the player's in observer mode)
	if (0 != pmove->spectator || pmove->iuser1 > 0)
	{
		PM_SpectatorMove();
		PM_CatagorizePosition();
		return;
	}

	// Always try and unstick us unless we are in NOCLIP mode
	if (pmove->movetype != MOVETYPE_NOCLIP && pmove->movetype != MOVETYPE_NONE)
	{
		if (PM_CheckStuck())
		{
			return; // Can't move, we're stuck
		}
	}

	// Now that we are "unstuck", see where we are ( waterlevel and type, pmove->onground ).
	PM_CatagorizePosition();

	// Store off the starting water level
	pmove->oldwaterlevel = pmove->waterlevel;
	if (pmove->watertype > CONTENT_FLYFIELD)
		pmove->oldwaterlevel = pmove->waterlevel;
	else
		pmove->oldwaterlevel = 0;

	// If we are not on ground, store off how fast we are moving down
	if (pmove->onground == -1)
	{
		pmove->flFallVelocity = -pmove->velocity[2];
	}

	g_onladder = false;
	// Don't run ladder code if dead or on a train
	if (0 == pmove->dead && (pmove->flags & FL_ONTRAIN) == 0)
	{
		pLadder = PM_Ladder();
		if (pLadder)
		{
			g_onladder = true;
		}
	}

	PM_UpdateStepSound();

	PM_Duck();

	// Don't run ladder code if dead or on a train
	if (0 == pmove->dead && (pmove->flags & FL_ONTRAIN) == 0)
	{
		if (pLadder)
		{
			PM_LadderMove(pLadder);
		}
		else if (pmove->movetype != MOVETYPE_WALK &&
				 pmove->movetype != MOVETYPE_NOCLIP)
		{
			// Clear ladder stuff unless player is noclipping
			//  it will be set immediately again next frame if necessary
			pmove->movetype = MOVETYPE_WALK;
		}
	}

	// Slow down, I'm pulling it! (a box maybe) but only when I'm standing on ground
	if ((pmove->onground != -1) && (pmove->cmd.buttons & IN_USE) != 0)
	{
		VectorScale(pmove->velocity, 0.3, pmove->velocity);
	}

	// Handle movement
	switch (pmove->movetype)
	{
	default:
		pmove->Con_DPrintf("Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", pmove->movetype, pmove->server);
		break;

	case MOVETYPE_NONE:
		break;

	case MOVETYPE_NOCLIP:
		PM_NoClip();
		break;

	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		PM_Physics_Toss();
		break;

	case MOVETYPE_FLY:

		PM_CheckWater();

		// Was jump button pressed?
		// If so, set velocity to 270 away from ladder.  This is currently wrong.
		// Also, set MOVE_TYPE to walk, too.
		if ((pmove->cmd.buttons & IN_JUMP) != 0)
		{
			if (!pLadder)
			{
				PM_Jump();
			}
		}
		else
		{
			pmove->oldbuttons &= ~IN_JUMP;
		}

		// Perform the move accounting for any base velocity.
		VectorAdd(pmove->velocity, pmove->basevelocity, pmove->velocity);
		PM_FlyMove();
		VectorSubtract(pmove->velocity, pmove->basevelocity, pmove->velocity);
		break;

	case MOVETYPE_WALK:
		if (!PM_InWater())
		{
			PM_AddCorrectGravity();
		}

		// If we are leaping out of the water, just update the counters.
		if (0 != pmove->waterjumptime)
		{
			PM_WaterJump();
			PM_FlyMove();

			// Make sure waterlevel is set correctly
			PM_CheckWater();
			return;
		}

		// If we are swimming in the water, see if we are nudging against a place we can jump up out
		//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
		if (pmove->waterlevel >= 2 && pmove->watertype != CONTENT_FOG)
		{
			if (pmove->waterlevel == 2)
			{
				PM_CheckWaterJump();
			}

			// If we are falling again, then we must not trying to jump out of water any more.
			if (pmove->velocity[2] < 0 && 0 != pmove->waterjumptime)
			{
				pmove->waterjumptime = 0;
			}

			// Was jump button pressed?
			if ((pmove->cmd.buttons & IN_JUMP) != 0)
			{
				PM_Jump();
			}
			else
			{
				pmove->oldbuttons &= ~IN_JUMP;
			}

			// Perform regular water movement
			PM_WaterMove();

			VectorSubtract(pmove->velocity, pmove->basevelocity, pmove->velocity);

			// Get a final position
			PM_CatagorizePosition();
		}
		else

		// Not underwater
		{
			// Was jump button pressed?
			if ((pmove->cmd.buttons & IN_JUMP) != 0)
			{
				if (!pLadder)
				{
					PM_Jump();
				}
			}
			else
			{
				pmove->oldbuttons &= ~IN_JUMP;
			}

			// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor,
			//  we don't slow when standing still, relative to the conveyor.
			if (pmove->onground != -1)
			{
				pmove->velocity[2] = 0.0;
				PM_Friction();
			}

			// Make sure velocity is valid.
			PM_CheckVelocity();

			// Are we on ground now
			if (pmove->onground != -1)
			{
				PM_WalkMove();
			}
			else
			{
				PM_AirMove(); // Take into account movement when in air.
			}

			// Set final flags.
			PM_CatagorizePosition();

			// Now pull the base velocity back out.
			// Base velocity is set if you are on a moving object, like
			//  a conveyor (or maybe another monster?)
			VectorSubtract(pmove->velocity, pmove->basevelocity, pmove->velocity);

			// Make sure velocity is valid.
			PM_CheckVelocity();

			// Add any remaining gravitational component.
			if (!PM_InWater())
			{
				PM_FixupGravityVelocity();
			}

			// If we are on ground, no downward velocity.
			if (pmove->onground != -1)
			{
				pmove->velocity[2] = 0;
			}

			// See if we landed on the ground with enough force to play
			//  a landing sound.
			PM_CheckFalling();
		}

		// Did we enter or leave the water?
		PM_PlayWaterSounds();
		break;
	}
}

void PM_CreateStuckTable()
{
	float x, y, z;
	int idx;
	int i;
	float zi[3];

	memset(rgv3tStuckTable, 0, 54 * sizeof(Vector));

	idx = 0;
	// Little Moves.
	x = y = 0;
	// Z moves
	for (z = -0.125; z <= 0.125; z += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	x = z = 0;
	// Y moves
	for (y = -0.125; y <= 0.125; y += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for (x = -0.125; x <= 0.125; x += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for (x = -0.125; x <= 0.125; x += 0.250)
	{
		for (y = -0.125; y <= 0.125; y += 0.250)
		{
			for (z = -0.125; z <= 0.125; z += 0.250)
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}

	// Big Moves.
	x = y = 0;
	zi[0] = 0.0f;
	zi[1] = 1.0f;
	zi[2] = 6.0f;

	for (i = 0; i < 3; i++)
	{
		// Z moves
		z = zi[i];
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	x = z = 0;

	// Y moves
	for (y = -2.0f; y <= 2.0f; y += 2.0)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for (x = -2.0f; x <= 2.0f; x += 2.0f)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for (i = 0; i < 3; i++)
	{
		z = zi[i];

		for (x = -2.0f; x <= 2.0f; x += 2.0f)
		{
			for (y = -2.0f; y <= 2.0f; y += 2.0)
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}
}



/*
This modume implements the shared player physics code between any particular game and 
the engine.  The same PM_Move routine is built into the game .dll and the client .dll and is
invoked by each side as appropriate.  There should be no distinction, internally, between server
and client.  This will ensure that prediction behaves appropriately.
*/

void PM_Move(struct playermove_s* ppmove, qboolean server)
{
	assert(pm_shared_initialized);

	pmove = ppmove;

	PM_PlayerMove(server);

	if (pmove->onground != -1)
	{
		pmove->flags |= FL_ONGROUND;
	}
	else
	{
		pmove->flags &= ~FL_ONGROUND;
	}

	// In single player, reset friction after each movement to FrictionModifier Triggers work still.
	if (0 == pmove->multiplayer && (pmove->movetype == MOVETYPE_WALK))
	{
		pmove->friction = 1.0f;
	}
}

int PM_GetVisEntInfo(int ent)
{
	if (ent >= 0 && ent <= pmove->numvisent)
	{
		return pmove->visents[ent].info;
	}
	return -1;
}

int PM_GetPhysEntInfo(int ent)
{
	if (ent >= 0 && ent <= pmove->numphysent)
	{
		return pmove->physents[ent].info;
	}
	return -1;
}

void PM_Init(struct playermove_s* ppmove)
{
	assert(!pm_shared_initialized);

	pmove = ppmove;

	PM_CreateStuckTable();

	PM_ParseStepTypesFile();
	PM_ParseMaterialTypesFile();
	PM_ParseMaterialImpactsFile();

	PM_ParseTextureMaterialsFile("sound\\materials.txt");

	pm_shared_initialized = 1;
}
