/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#pragma once

inline int gmsgShake = 0;
inline int gmsgFade = 0;
inline int gmsgFlashlight = 0;
inline int gmsgFlashBattery = 0;
inline int gmsgResetHUD = 0;
inline int gmsgInitHUD = 0;
inline int gmsgSetFog = 0;		// LRC
inline int gmsgKeyedDLight = 0; // LRC
inline int gmsgKeyedELight = 0; // LRC
inline int gmsgSetSky = 0;		// LRC
inline int gmsgHUDColor = 0;	// LRC
inline int gmsgAddShine = 0;	// LRC
inline int gmsgParticle = 0;	// LRC
inline int gmsgClampView = 0;	// LRC 1.8
inline int gmsgPlayMP3 = 0;		// Killar
inline int gmsgShowGameTitle = 0;
inline int gmsgCurWeapon = 0;
inline int gmsgHealth = 0;
inline int gmsgDamage = 0;
inline int gmsgBattery = 0;
inline int gmsgTrain = 0;
inline int gmsgLogo = 0;
inline int gmsgWeaponList = 0;
inline int gmsgAmmoX = 0;
inline int gmsgHudText = 0;
inline int gmsgDeathMsg = 0;
inline int gmsgScoreInfo = 0;
inline int gmsgTeamInfo = 0;
inline int gmsgTeamScore = 0;
inline int gmsgGameMode = 0;
inline int gmsgMOTD = 0;
inline int gmsgServerName = 0;
inline int gmsgAmmoPickup = 0;
inline int gmsgWeapPickup = 0;
inline int gmsgItemPickup = 0;
inline int gmsgHideWeapon = 0;
inline int gmsgSetCurWeap = 0;
inline int gmsgSayText = 0;
inline int gmsgTextMsg = 0;
inline int gmsgSetFOV = 0;
inline int gmsgShowMenu = 0;
inline int gmsgGeigerRange = 0;
inline int gmsgTeamNames = 0;
inline int gmsgStatusIcon = 0; //LRC
inline int gmsgStatusText = 0;
inline int gmsgStatusValue = 0;
inline int gmsgCamData = 0; // for trigger_viewset
inline int gmsgRainData = 0;
inline int gmsgInventory = 0; //AJH Inventory system

inline int gmsgWeapons = 0;

void LinkUserMessages();
