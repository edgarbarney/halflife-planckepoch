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
#include "mp3.h" //AJH - Killar MP3
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "rain.h"

#include "CGameStateManager.h"

//RENDERERS START
#include "bsprenderer.h"
#include "propmanager.h"
#include "watershader.h"

#include "studio.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

extern CGameStudioModelRenderer g_StudioRenderer;
//RENDERERS END

//LRC - the fogging fog
FogSettings g_fog;
FogSettings g_fogPreFade;
FogSettings g_fogPostFade;
float g_fFogFadeDuration;
float g_fFogFadeFraction;

#define MAX_CLIENTS 32

#if !defined( _TFC )
extern BEAM *pBeam;
extern BEAM *pBeam2;
extern TEMPENTITY* pFlare;	// Vit_amiN
#endif 

#if defined( _TFC )
void ClearEventList();
#endif

extern float g_clampMinYaw, g_clampMaxYaw, g_clampMinPitch, g_clampMaxPitch;
extern float g_clampTurnSpeed;

/// USER-DEFINED SERVER MESSAGE HANDLERS

int CHud :: MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf )
{
	ASSERT( iSize == 0 );

//RENDERERS START
	gHUD.m_pFogSettings.end = 0.0; 
	gHUD.m_pFogSettings.start = 0.0;
	gHUD.m_pFogSettings.active = false;
	gHUD.m_pSkyFogSettings.end = 0.0; 
	gHUD.m_pSkyFogSettings.start = 0.0;
	gHUD.m_pSkyFogSettings.active = false;
//RENDERERS END
	// clear all hud data
	HUDLIST *pList = m_pHudList;

	while ( pList )
	{
		if ( pList->p )
			pList->p->Reset();
		pList = pList->pNext;
	}
		
	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;

/*	//LRC - reset fog
	g_fStartDist = 0;
	g_fEndDist = 0;
	numMirrors = 0;
*/
	return 1;
}

//void CAM_ToFirstPerson(void);

void CHud :: MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf )
{
	//CAM_ToFirstPerson();
}

void CHud :: MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
//	CONPRINT("MSG:InitHUD");
	//LRC - clear the fog
	g_fog.startDist = -1;
	g_fog.endDist = -1;
	g_fog.fogColor[0] = -1;
	g_fog.fogColor[1] = -1;
	g_fog.fogColor[2] = -1;
	//LRC 1.8 - clear view clamps
	g_clampMinPitch = -90;
	g_clampMaxPitch = 90;
	g_clampMinYaw = 0;
	g_clampMaxYaw = 360;
	g_clampTurnSpeed = 1E6;
	numMirrors = 0;

	m_iSkyMode = SKY_OFF; //LRC
	
	//RENDERERS START
	gHUD.m_pFogSettings.end = 0.0; 
	gHUD.m_pFogSettings.start = 0.0;
	gHUD.m_pFogSettings.active = false;
	gHUD.m_pSkyFogSettings.end = 0.0; 
	gHUD.m_pSkyFogSettings.start = 0.0;
	gHUD.m_pSkyFogSettings.active = false;
	//RENDERERS END

	// prepare all hud data
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if ( pList->p )
			pList->p->InitHUDData();
		pList = pList->pNext;
	}

#if defined( _TFC )
	ClearEventList();

	// catch up on any building events that are going on
	gEngfuncs.pfnServerCmd("sendevents");
#endif

#if !defined( _TFC )
	//Probably not a good place to put this.
	pBeam = pBeam2 = nullptr;
	pFlare = nullptr;	// Vit_amiN: clear egon's beam flare
#endif

	g_gameStateManager.InitHud();
}

/*
//LRC
void CHud :: MsgFunc_KeyedDLight( const char *pszName, int iSize, void *pbuf )
{
//	CONPRINT("MSG:KeyedDLight");
	BEGIN_READ( pbuf, iSize );

// as-yet unused:
//	float	decay;				// drop this each second
//	float	minlight;			// don't add when contributing less
//	qboolean	dark;			// subtracts light instead of adding (doesn't seem to do anything?)

	int iKey = READ_BYTE();
	dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( iKey );

	int bActive = READ_BYTE();
	if (!bActive)
	{
		// die instantly
		dl->die = gEngfuncs.GetClientTime();
	}
	else
	{
		// never die
		dl->die = gEngfuncs.GetClientTime() + 1E6;

		dl->origin[0] = READ_COORD();
		dl->origin[1] = READ_COORD();
		dl->origin[2] = READ_COORD();
		dl->radius = READ_BYTE();
		dl->color.r = READ_BYTE();
		dl->color.g = READ_BYTE();
		dl->color.b = READ_BYTE();
	}
}
*/

//LRC
void CHud :: MsgFunc_SetSky( const char *pszName, int iSize, void *pbuf )
{
//	CONPRINT("MSG:SetSky");
	BEGIN_READ( pbuf, iSize );

	m_iSkyMode = READ_BYTE();
	m_vecSkyPos.x = READ_COORD();
	m_vecSkyPos.y = READ_COORD();
	m_vecSkyPos.z = READ_COORD();
	m_iSkyScale = READ_BYTE();
}

//LRC 1.8
void CHud :: MsgFunc_ClampView( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	g_clampMinYaw = READ_SHORT();
	g_clampMaxYaw = READ_SHORT();
	g_clampMinPitch = READ_BYTE() - 128;
	g_clampMaxPitch = READ_BYTE() - 128;
	*(long*)&g_clampTurnSpeed = READ_LONG();
}

int CHud :: MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_Teamplay = READ_BYTE();

	return 1;
}

int CHud :: MsgFunc_Damage(const char *pszName, int iSize, void *pbuf )
{
	int		armor, blood;
	Vector	from;
	int		i;
	float	count;
	
	BEGIN_READ( pbuf, iSize );
	armor = READ_BYTE();
	blood = READ_BYTE();

	for (i=0 ; i<3 ; i++)
		from[i] = READ_COORD();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return 1;
}

int CHud :: MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iConcussionEffect = READ_BYTE();
	if (m_iConcussionEffect)
	{
		int r, g, b;
		UnpackRGB(r, g, b, gHUD.m_iHUDColor);
		this->m_StatusIcons.EnableIcon("dmg_concuss", r, g, b);
	}
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return 1;
}

int CHud :: MsgFunc_PlayMP3( const char *pszName, int iSize, void *pbuf ) //AJH -Killar MP3
{
	BEGIN_READ( pbuf, iSize );

	gMP3.PlayMP3( READ_STRING() );

	return 1;
}
	// trigger_viewset message
int CHud :: MsgFunc_CamData( const char *pszName, int iSize, void *pbuf ) // rain stuff
{
	BEGIN_READ( pbuf, iSize );
		gHUD.viewEntityIndex = READ_SHORT();
		gHUD.viewFlags = READ_SHORT();
//	gEngfuncs.Con_Printf( "Got view entity with index %i\n", gHUD.viewEntityIndex );
	return 1;
}

int CHud::MsgFunc_RainData(const char* pszName, int iSize, void* pbuf)
{
	return 1;
}

void CHud::MsgFunc_ServerState(const char* pszName, int iSize, void* pBuf)
{
	BEGIN_READ(pBuf, iSize);
	const auto state = READ_BYTE();

	g_gameStateManager.UpdateServerState(state);
}

int CHud :: MsgFunc_Inventory( const char *pszName, int iSize, void *pbuf ) //AJH inventory system
{
	BEGIN_READ( pbuf, iSize );
		int i = READ_SHORT();

		if (i==0){ //We've died (or got told to lose all items) so remove inventory.
			for (i=0;i<MAX_ITEMS;i++){
				g_iInventory[i]=0;
			}
		}else
		{
			i-=1;	// subtract one so g_iInventory[0] can be used. (lowest ITEM_* is defined as '1')
			g_iInventory[i] = READ_SHORT();
		}
	return 1;
}

//RENDERERS START
int CHud ::MsgFunc_SetFog( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	gHUD.m_pFogSettings.color.x = (float)READ_SHORT()/255;
	gHUD.m_pFogSettings.color.y = (float)READ_SHORT()/255;
	gHUD.m_pFogSettings.color.z = (float)READ_SHORT()/255;
	gHUD.m_pFogSettings.start = READ_SHORT();
	gHUD.m_pFogSettings.end = READ_SHORT();
	gHUD.m_pFogSettings.affectsky = (READ_SHORT() == 1) ? false : true;

	if( gHUD.m_pFogSettings.end < 1 && gHUD.m_pFogSettings.start < 1 )
		gHUD.m_pFogSettings.active = false;
	else
		gHUD.m_pFogSettings.active = true;

	return 1;
}
int CHud :: MsgFunc_LightStyle(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int m_iStyleNum = READ_BYTE();
	char *szStyle = READ_STRING();
	gBSPRenderer.AddLightStyle(m_iStyleNum,szStyle);

	return 1;
}
int CHud ::MsgFunc_StudioDecal(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	Vector pos, normal;
	pos.x = READ_COORD();
	pos.y = READ_COORD();
	pos.z = READ_COORD();
	normal.x = READ_COORD();
	normal.y = READ_COORD();
	normal.z = READ_COORD();
	int entindex = READ_SHORT();
	
	if(!entindex)
		return 1;

	cl_entity_t *pEntity = gEngfuncs.GetEntityByIndex(entindex);
	
	if(!pEntity)
		return 1;

	g_StudioRenderer.StudioDecalForEntity(pos, normal, READ_STRING(), pEntity);

	return 1;
}
int CHud ::MsgFunc_FreeEnt(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int iEntIndex = READ_SHORT();
	
	if(!iEntIndex)
		return 1;
	

	cl_entity_t *pEntity = gEngfuncs.GetEntityByIndex(iEntIndex);

	if(!pEntity)
		return 1;

	pEntity->efrag = nullptr;
	return 1;
}
//RENDERERS END