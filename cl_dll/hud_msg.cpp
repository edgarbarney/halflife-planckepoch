/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//
//  hud_msg.cpp
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"

#include "particleman.h"
extern IParticleMan* g_pParticleMan;

//LRC - the fogging fog
float g_fFogColor[3];
float g_fStartDist;
float g_fEndDist;
//int g_iFinalStartDist; //for fading
int g_iFinalEndDist;   //for fading
float g_fFadeDuration; //negative = fading out

#if !defined(_TFC)
extern BEAM* pBeam;
extern BEAM* pBeam2;
extern TEMPENTITY* pFlare; // Vit_amiN
#endif

#if defined(_TFC)
void ClearEventList();
#endif

/// USER-DEFINED SERVER MESSAGE HANDLERS

bool CHud::MsgFunc_ResetHUD(const char* pszName, int iSize, void* pbuf)
{
	//	CONPRINT("MSG:ResetHUD\n");

	ASSERT(iSize == 0);

	// clear all hud data
	HUDLIST* pList = m_pHudList;

	while (pList)
	{
		if (pList->p)
			pList->p->Reset();
		pList = pList->pNext;
	}

	//Reset weapon bits.
	m_iWeaponBits = 0ULL;

	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;

	//LRC - reset fog
	g_fStartDist = 0;
	g_fEndDist = 0;

	return true;
}

void CAM_ToFirstPerson();

void CHud::MsgFunc_ViewMode(const char* pszName, int iSize, void* pbuf)
{
	CAM_ToFirstPerson();
}

void CHud::MsgFunc_InitHUD(const char* pszName, int iSize, void* pbuf)
{
	//	CONPRINT("MSG:InitHUD");
	//LRC - clear the fog
	g_fStartDist = 0;
	g_fEndDist = 0;

	// prepare all hud data
	HUDLIST* pList = m_pHudList;

	while (pList)
	{
		if (pList->p)
			pList->p->InitHUDData();
		pList = pList->pNext;
	}

#if defined(_TFC)
	ClearEventList();

	// catch up on any building events that are going on
	gEngfuncs.pfnServerCmd("sendevents");
#endif

	if (g_pParticleMan)
		g_pParticleMan->ResetParticles();

#if !defined(_TFC)
	//Probably not a good place to put this.
	pBeam = pBeam2 = NULL;
	pFlare = NULL; // Vit_amiN: clear egon's beam flare
#endif
}

//LRC
void CHud::MsgFunc_SetFog(const char* pszName, int iSize, void* pbuf)
{
	//	CONPRINT("MSG:SetFog");
	BEGIN_READ(pbuf, iSize);

	for (int i = 0; i < 3; i++)
		g_fFogColor[i] = READ_BYTE();

	g_fFadeDuration = READ_SHORT();
	g_fStartDist = READ_SHORT();

	if (g_fFadeDuration > 0)
	{
		//		// fading in
		//		g_fStartDist = READ_SHORT();
		g_iFinalEndDist = READ_SHORT();
		//		g_fStartDist = FOG_LIMIT;
		g_fEndDist = FOG_LIMIT;
	}
	else if (g_fFadeDuration < 0)
	{
		//		// fading out
		//		g_iFinalStartDist =
		g_iFinalEndDist = g_fEndDist = READ_SHORT();
	}
	else
	{
		//		g_fStartDist = READ_SHORT();
		g_fEndDist = READ_SHORT();
	}
}


bool CHud::MsgFunc_GameMode(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	//Note: this user message could be updated to include multiple gamemodes, so make sure this checks for game mode 1
	//See CHalfLifeTeamplay::UpdateGameMode
	//TODO: define game mode constants
	m_Teamplay = READ_BYTE() == 1;

	return true;
}


bool CHud::MsgFunc_Damage(const char* pszName, int iSize, void* pbuf)
{
	int armor, blood;
	Vector from;
	int i;
	float count;

	BEGIN_READ(pbuf, iSize);
	armor = READ_BYTE();
	blood = READ_BYTE();

	for (i = 0; i < 3; i++)
		from[i] = READ_COORD();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return true;
}

bool CHud::MsgFunc_Concuss(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_iConcussionEffect = READ_BYTE();
	if (0 != m_iConcussionEffect)
	{
		int r, g, b;
		UnpackRGB(r, g, b, gHUD.m_iHUDColor);
		this->m_StatusIcons.EnableIcon("dmg_concuss", r, g, b);
	}
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return true;
}

bool CHud::MsgFunc_Weapons(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	const std::uint64_t lowerBits = READ_LONG();
	const std::uint64_t upperBits = READ_LONG();

	m_iWeaponBits = lowerBits | (upperBits << 32ULL);

	return true;
}
