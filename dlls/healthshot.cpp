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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "decals.h"
#include "shake.h"

enum healthshot_e {
	HEALTHSHOT_IDLE = 0,
	HEALTHSHOT_FIDGET,
	HEALTHSHOT_USE,
	HEALTHSHOT_HOLSTER,
	HEALTHSHOT_DRAW
};

const char* healthshot_models [] =
{
	"models/v_healthshot.mdl",
	"models/w_medkit.mdl",
	"models/p_crowbar.mdl",
};

LINK_ENTITY_TO_CLASS(weapon_healthshot, CHealthShot);

void CHealthShot::Spawn()
{
	pev->classname = MAKE_STRING("weapon_healthshot"); // hack to allow for old names
	Precache();
	SET_MODEL(ENT(pev), healthshot_models[MDL_WORLD]);
	m_iId = WEAPON_HEALTHSHOT;
	
	m_isUsed = false;
	//m_healthToAdd = 50;

	m_iDefaultAmmo = HEALTHSHOT_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CHealthShot::Precache()
{
	PRECACHE_MODEL(healthshot_models[MDL_WORLD]);
	PRECACHE_MODEL(healthshot_models[MDL_VIEW]);
	PRECACHE_MODEL(healthshot_models[MDL_PLAYA]);
	PRECACHE_SOUND("items/smallmedkit1.wav");
}

BOOL CHealthShot::Deploy()
{
	return DefaultDeploy(healthshot_models[MDL_VIEW], healthshot_models[MDL_PLAYA], HEALTHSHOT_DRAW, "healthshot", 0.9);
}

void CHealthShot::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
	SendWeaponAnim(HEALTHSHOT_HOLSTER);
}

int CHealthShot::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Health";
	p->iMaxAmmo1 = HEALTHSHOT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = WEAPON_HEALTHSHOT;
	p->iFlags = ITEM_FLAG_EXHAUSTIBLE;
	p->iWeight = 16;
	return 1;
}

int CHealthShot::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

void CHealthShot::PrimaryAttack()
{
	//Start Throw is wait time that is predefined for grenades which we will reuse for this
	if (!m_isUsed && m_pPlayer->pev->health < 100) {
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		{
			m_flStartThrow = gpGlobals->time;
			m_pPlayer->m_hsBoostStartTime = gpGlobals->time;
			m_flReleaseThrow = 0;

			SendWeaponAnim(HEALTHSHOT_USE);
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
			// Time Before Heal
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;  // GlobalTime + Time Before Heal
			m_pPlayer->m_hsBoostIdleTime = gpGlobals->time + 4.5f; // GlobalTime + Time Before Heal + Wait Time
			m_pPlayer->m_hsIsBoosting = TRUE;
			m_isUsed = true;
		}
	}
}

void CHealthShot::ApplyHealth()
{
#ifndef CLIENT_DLL
	UTIL_ScreenFade(m_pPlayer, Vector(0, 225, 255), 2, 2, 128, FFADE_IN);
#endif
	m_pPlayer->pev->iuser1 = TRUE;
	m_pPlayer->pev->fuser1 = 600; //def 320. sv_maxspeed replacement

	if (m_pPlayer->pev->health + m_healthToAdd > 100)
		m_pPlayer->pev->health = 100;
	else
		m_pPlayer->pev->health += m_healthToAdd;
	m_isUsed = false;
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "player/pl_wade1.wav", 0.9, ATTN_NORM);
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 1)
		RetireWeapon();
}

void CHealthShot::WeaponIdle()
{
	ResetEmptySound();

	if (m_flReleaseThrow == 0 && m_flStartThrow)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow)
	{
		ApplyHealth();

		m_flReleaseThrow = 0;
		m_flStartThrow = 0;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5f);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		
		RetireWeapon();
#ifndef CLIENT_DLL
		m_pPlayer->SwitchWeapon(m_pPlayer->m_pLastItem);
#endif

		return;
	}
	else if (m_flReleaseThrow > 0)
	{
		// we've finished the throw, restart.
		RetireWeapon();
		m_flStartThrow = 0;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0f, 1.0f);
		if (flRand <= 0.75f)
		{
			iAnim = HEALTHSHOT_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10.0f, 15.0f);// how long till we do this again.
		}
		else
		{
			iAnim = HEALTHSHOT_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0f / 30.0f;
		}

		SendWeaponAnim(iAnim);
	}
}


