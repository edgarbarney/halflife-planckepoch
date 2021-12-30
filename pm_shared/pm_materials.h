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

#if !defined( PM_MATERIALSH )
#define PM_MATERIALSH
#pragma once

#ifndef _MAP_
#include <map>
#endif

#ifndef _STRING_
#include <string>
#endif

#ifndef _VECTOR_
#include <vector>
#endif

#include "pm_structs.h"

#define CBTEXTURENAMEMAX	13			// only load first n chars of name


//Contains texture types
//   TextureTypeName, Texture Type
extern std::map<std::string, textureType_s> g_TextureTypeMap;

//Contains Step types
//		StepTypeName, Step Type
extern std::map<std::string, stepType_s> g_StepTypeMap;

//Contains texture names and corresponding types
//		 TextureName, TextureType
//extern std::map<std::string, std::string> g_TypedTextureMap;

//Contains texture names and corresponding types' pointers
//		 TextureName, *TextureType
extern std::map<std::string, textureType_s*> g_TypedTextureMapPtr;

//Contains special attributes and corresponding step types' pointers
//				SpecialType, *StepType
extern std::map<StepSpecialType, stepType_s*> g_SpecialStepMapPtr;

//Contains Texture impact types
extern std::vector<impactGroupType_s> g_texTypeImpactTypeVector;

#endif // !PM_MATERIALSH
