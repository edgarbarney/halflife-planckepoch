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

//Shared Structs

#ifndef PM_STRUCTSH
#define PM_STRUCTSH
#pragma once

#ifndef _STRING_
#include <string>
#endif

#ifndef _VECTOR_
#include <vector>
#endif

#define NOCHECK INT32_MAX

enum class StepSpecialType
{
	Default,
	Ladder,
	LiquidKnee,
	LiquidFeet,
	Flesh,
};

struct textureType_s
{
	int texTypeID;				// Tex Type Number (Identifier)
	std::string texType;		// Material Type Alias. Used in "materials.txt"
	std::string texStep;		// Sound Type that material uses. Used for bullet/melee impact and Footsteps.
	float impactVolume;			// Volume of impact sound.
	float impactAttenuation;	// Volume of weapon when impact happens. Used for melee.
	float weaponVolume;			// Attenuetion of impact sound. Default is 0.8 (ATTN_NORM)

	/// <summary>
	/// Material Type Constructor
	/// </summary>
	/// <param name="tType">Material Type Alias</param>
	/// <param name="tStep">Material Step Type</param>
	/// <param name="impVol">Impact Volume</param>
	/// <param name="impAtten">Impact Attenuation</param>
	/// <param name="weapVol">Weapon Volume</param>

	textureType_s
	(	int tTypeNum, 
		std::string tType = "C", 
		std::string tStep = "STEP_CONCRETE", 
		float impVol = 0.6f, 
		float impAtten = 0.8f, 
		float weapVol = 0.3f 
	)
	{
		texTypeID = tTypeNum;
		texType = tType;
		texStep = tStep;
		impactVolume = impVol;
		impactAttenuation = impAtten;
		weaponVolume = weapVol;
	}

	textureType_s()
	{
		texTypeID = 0;
		texType = "C";
		texStep = "STEP_CONCRETE";
		impactVolume = 0.6f;
		impactAttenuation = 0.8f;
		weaponVolume = 0.3f;
	}
};

struct stepType_s
{
	int stepNum;							// Step Number (Identifier)
	bool stepSkip;							// Will skip sound?
	std::vector <std::string> stepSounds;	// Sound list

	float walkingVolume;					// "Walking" (slow) sound volume
	float normalVolume;						// "Running" (default) sound volume
	float walkingStepTime;					// Next "Walking" (slow) step delay in ms - Should be int???
	float normalStepTime;					// Next "Running" (default) step delay in ms - Should be int???
	float crouchMultiplier;					// Volume multiplier when crouching

	
	/// <summary>
	/// Step Type Constructor
	/// </summary>
	/// <param name="stNum">Step order</param>
	/// <param name="stSkip">Will skip sound?</param>
	/// <param name="stSound">A sound to add into the soundlist</param>
	/// <param name="walkingVol">"Walking" (slow) sound volume</param>
	/// <param name="normalVol">"Running" (default) sound volume</param>
	/// <param name="walkingStTime">Next "Walking" (slow) step delay in ms</param>
	/// <param name="normalStTime">Next "Running" (default) step delay in ms</param>
	/// <param name="crouchVolMulti">Volume multiplier when crouching</param>
	stepType_s
	(
		int stNum = 0, 
		bool stSkip = false, 
		std::string stSound = "ambience/_comma.wav",
		float walkingVol = 0.2f,
		float normalVol = 0.5f,
		float walkingStTime = 400,
		float normalStTime = 300,
		float crouchVolMulti= 0.35f
	)
	{
		stepNum = stNum;
		stepSkip = stSkip;
		stepSounds.push_back(stSound);
		walkingVolume = walkingVol;
		normalVolume = normalVol;
		walkingStepTime = walkingStTime;
		normalStepTime = normalStTime;
		crouchMultiplier = crouchVolMulti;
	}
};

struct impactType_s
{
	std::string materialTypeAlias;	// Which material is going to produce this impact.
	std::string decalGroupName;		// Decal Group to Trace
	std::string scriptFile;			// Particle Script to Create

	impactType_s
	(
		std::string mType = "DEFAULT",
		std::string dgName = "shot",
		std::string scFile = "concrete_impact_cluster.txt"
	)
	{
		materialTypeAlias = mType;
		decalGroupName = dgName;
		scriptFile = scFile;
	}
};

struct impactGroupType_s
{
	int renderMode;		//PhysEnt Cheks
	int renderAmt;		//PhysEnt Cheks
	int classnumber;	//PhysEnt Cheks

	std::vector<impactType_s> impactTypes;

	impactGroupType_s
	(
		int rMode = NOCHECK,
		int rAmt = NOCHECK,
		int cNumber = NOCHECK
	)
	{
		renderMode = rMode;
		renderAmt = rAmt;
		classnumber = cNumber;
	}
};


#endif // PM_STRUCTSH