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
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

//#define WEAPON_TIMEBASE UTIL_WeaponTimeBase()
#define WEAPON_TIMEBASE gpGlobals->time

#define	CROWBAR_BODYHIT_VOLUME 128
#define	CROWBAR_WALLHIT_VOLUME 512

#ifndef CLIENT_DLL
TYPEDESCRIPTION	CCrowbar::m_SaveData[] =
{
	DEFINE_FIELD(CCrowbar, m_flBigSwingStart, FIELD_TIME),
	DEFINE_FIELD(CCrowbar, m_iSwing, FIELD_INTEGER),
	DEFINE_FIELD(CCrowbar, m_iSwingMode, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CCrowbar, CCrowbar::BaseClass);
#endif

LINK_ENTITY_TO_CLASS(weapon_crowbar, CCrowbar);

enum crowbar_e
{
	CROWBAR_IDLE1 = 0,
	CROWBAR_IDLE2,
	CROWBAR_IDLE3,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK1MISS,
	CROWBAR_ATTACK2HIT,
	CROWBAR_ATTACK2MISS,
	CROWBAR_ATTACK3HIT,
	CROWBAR_ATTACK3MISS,
	CROWBAR_BIG_SWING_START,
	CROWBAR_BIG_SWING_HIT,
	CROWBAR_BIG_SWING_MISS,
	CROWBAR_BIG_SWING_IDLE
};

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int			i, j, k;
	float		distance;
	float		*minmaxs[2] = {mins, maxs};
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );
	if ( tmpTrace.flFraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );
				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

void CCrowbar::Spawn()
{
	pev->classname = MAKE_STRING("weapon_crowbar");
	Precache();
	m_iId = WEAPON_CROWBAR;
	SET_MODEL(edict(), "models/w_crowbar.mdl");
	m_iClip = WEAPON_NOCLIP;
	m_iSwingMode = SWING_NONE;

	FallInit(); // get ready to fall down.
}

void CCrowbar::Precache()
{
	PRECACHE_MODEL("models/v_crowbar.mdl");
	PRECACHE_MODEL("models/w_crowbar.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	// Shepard - The commented sounds below are unused
	// in Opposing Force, if you wish to use them,
	// uncomment all the appropriate lines.
	/*PRECACHE_SOUND("weapons/fireaxe_big_hit1.wav");
	PRECACHE_SOUND("weapons/fireaxe_big_hit2.wav");*/
	PRECACHE_SOUND("weapons/fireaxe_big_hitbod1.wav");
	PRECACHE_SOUND("weapons/fireaxe_big_hitbod2.wav");
	PRECACHE_SOUND("weapons/fireaxe_big_miss.wav");
	PRECACHE_SOUND("weapons/fireaxe_hit1.wav");
	PRECACHE_SOUND("weapons/fireaxe_hit2.wav");
	PRECACHE_SOUND("weapons/fireaxe_hitbod1.wav");
	PRECACHE_SOUND("weapons/fireaxe_hitbod2.wav");
	PRECACHE_SOUND("weapons/fireaxe_hitbod3.wav");
	PRECACHE_SOUND("weapons/fireaxe_miss1.wav");
	PRECACHE_SOUND("weapons/fireaxe_miss2.wav");

	m_usCrowbar = PRECACHE_EVENT(1, "events/crowbar.sc");
}

BOOL CCrowbar::Deploy()
{
	return CbarDeploy("models/v_crowbar.mdl", "models/p_crowbar.mdl", CROWBAR_DRAW, "crowbar");
}

void CCrowbar::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = WEAPON_TIMEBASE + 1.35;
	SendWeaponAnim(CROWBAR_HOLSTER);
}

void CCrowbar::PrimaryAttack()
{
	if (m_iSwingMode == SWING_NONE && !Swing(true))
	{
#ifndef CLIENT_DLL
		SetThink(&CCrowbar::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.1;
#endif
	}
}

void CCrowbar::SecondaryAttack()
{
	if (m_iSwingMode != SWING_START_BIG)
	{
		SendWeaponAnim(CROWBAR_BIG_SWING_START);
		m_flBigSwingStart = gpGlobals->time;
	}

	m_iSwingMode = SWING_START_BIG;

	m_flNextPrimaryAttack = GetNextAttackDelayGlobal(1.0);
	m_flNextSecondaryAttack = GetNextAttackDelayGlobal(0.1);
	m_flTimeWeaponIdle = m_flNextSecondaryAttack + 0.2;
}

void CCrowbar::Smack()
{
	// Trinity Removed
	//DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}

void CCrowbar::SwingAgain()
{
	Swing(false);
}

bool CCrowbar::Swing(const bool bFirst)
{
	bool bDidHit = false;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	if (bFirst)
	{
#if defined( CLIENT_WEAPONS )
		m_eventFlags = FEV_NOTHOST;
#else
		m_eventFlags = 0;
#endif
		PLAYBACK_EVENT_FULL(m_eventFlags, m_pPlayer->edict(), m_usCrowbar,
			0.0, (float*)&g_vecZero, (float*)&g_vecZero, 3.0, RANDOM_FLOAT(-3.0, 3.0), 0,
			0.0, 0, tr.flFraction < 1);
	}


	if (tr.flFraction >= 1.0)
	{
		if (bFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelayGlobal(0.75);
			m_flNextSecondaryAttack = GetNextAttackDelayGlobal(0.75);
			m_flTimeWeaponIdle = WEAPON_TIMEBASE + 1.0;

			// Shepard - In Opposing Force, the "miss" sound is
			// played twice (maybe it's a mistake from Gearbox or
			// an intended feature), if you only want a single
			// sound, comment this "switch" or the one in the
			// event (EV_Crowbar)
			/*
			switch ( ((m_iSwing++) % 1) )
			{
			case 0: EMIT_SOUND( m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_miss1.wav", 1, ATTN_NORM); break;
			case 1: EMIT_SOUND( m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_miss2.wav", 1, ATTN_NORM); break;
			}
			*/

			switch (RANDOM_LONG(0,1))
			{
			case 0: 
				EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_miss1.wav", 1, ATTN_NORM); 
				break;
			case 1: 
				EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_miss2.wav", 1, ATTN_NORM); 
				break;
			}

			switch ((m_iSwing++) % 3)
			{
			case 0:
				SendWeaponAnim(CROWBAR_ATTACK1MISS); 
				break;
			case 1:
				SendWeaponAnim(CROWBAR_ATTACK2MISS); 
				break;
			case 2:
				SendWeaponAnim(CROWBAR_ATTACK3MISS); 
				break;
			}

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}
	}
	else
	{
		switch ((m_iSwing++) % 3)
		{
		case 0:
			SendWeaponAnim(CROWBAR_ATTACK1HIT); 
			break;
		case 1:
			SendWeaponAnim(CROWBAR_ATTACK2HIT); 
			break;
		case 2:
			SendWeaponAnim(CROWBAR_ATTACK3HIT); 
			break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL

		// hit
		bDidHit = true;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (pEntity)
		{
			ClearMultiDamage();

			if ((m_flNextPrimaryAttack + 1 < WEAPON_TIMEBASE) || g_pGameRules->IsMultiplayer())
			{
				// first swing does full damage
				pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgCrowbar, gpGlobals->v_forward, &tr, DMG_CLUB);
			}
			else
			{
				// subsequent swings do half
				pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgCrowbar / 2, gpGlobals->v_forward, &tr, DMG_CLUB);
			}

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		}

#endif

		m_flNextPrimaryAttack = GetNextAttackDelayGlobal(0.5);
		m_flNextSecondaryAttack = GetNextAttackDelayGlobal(0.5);
		m_flTimeWeaponIdle = WEAPON_TIMEBASE + 1.0;

#ifndef CLIENT_DLL

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool bHitWorld = true;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 2))
				{
				case 0:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hitbod1.wav", 1, ATTN_NORM); break;
				case 1:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hitbod2.wav", 1, ATTN_NORM); break;
				case 2:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hitbod3.wav", 1, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return true;
				else
					flVol = 0.1;

				bHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (bHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play pipe wrench strike
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;

		SetThink(&CCrowbar::Smack);
		pev->nextthink = gpGlobals->time + 0.2;

		//RENDERERS START
		DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR, vecSrc, vecEnd);
		//RENDERERS END

#endif
	}
	return bDidHit;
}

void CCrowbar::BigSwing()
{
	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

#if defined( CLIENT_WEAPONS )
	m_eventFlags = FEV_NOTHOST;
#else
	m_eventFlags = 0;
#endif

	PLAYBACK_EVENT_FULL(m_eventFlags, m_pPlayer->edict(), m_usCrowbar,
		0.0, (float*)&g_vecZero, (float*)&g_vecZero, 3.0, RANDOM_FLOAT(-3.0, 3.0), 0,
		0.0, 1, tr.flFraction < 1);

	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/fireaxe_big_miss.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));

	if (tr.flFraction >= 1.0)
	{
		// miss
		m_flNextPrimaryAttack = GetNextAttackDelayGlobal(1.0);
		m_flNextSecondaryAttack = GetNextAttackDelayGlobal(1.0);
		m_flTimeWeaponIdle = WEAPON_TIMEBASE + 1.0;

		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_miss1.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_miss2.wav", 1, ATTN_NORM);
			break;
		}

		SendWeaponAnim(CROWBAR_BIG_SWING_MISS);

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	}
	else
	{
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hit1.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hit2.wav", 1, ATTN_NORM);
			break;
		}
		SendWeaponAnim(CROWBAR_BIG_SWING_HIT);

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL

		// hit
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (pEntity)
		{
			ClearMultiDamage();

			float flDamage = (gpGlobals->time - m_flBigSwingStart) * gSkillData.plrDmgCrowbar + 25.0f;
			if ((m_flNextPrimaryAttack + 1 < WEAPON_TIMEBASE) || g_pGameRules->IsMultiplayer())
			{
				// first swing does full damage
				pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_CLUB);
			}
			else
			{
				// subsequent swings do half
				pEntity->TraceAttack(m_pPlayer->pev, flDamage / 2, gpGlobals->v_forward, &tr, DMG_CLUB);
			}
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		}

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool bHitWorld = true;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_big_hitbod1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_big_hitbod2.wav", 1, ATTN_NORM);
					break;
				}
				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return;
				else
					flVol = 0.1;

				bHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (bHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play pipe wrench strike
			// Shepard - The commented sounds below are unused
			// in Opposing Force, if you wish to use them,
			// uncomment all the appropriate lines.
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				//EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/fireaxe_big_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			case 1:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/fireaxe_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				//EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/fireaxe_big_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;

		// Shepard - The original Opposing Force's pipe wrench
		// doesn't make a bullet hole decal when making a big
		// swing. If you want that decal, just uncomment the
		// 2 lines below.
		SetThink( &CCrowbar::Smack );
		SetNextThink( WEAPON_TIMEBASE + 0.2 );

		//RENDERERS START
		DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR, vecSrc, vecEnd);
		//RENDERERS END

#endif
		m_flNextPrimaryAttack = GetNextAttackDelayGlobal(1.0);
		m_flNextSecondaryAttack = GetNextAttackDelayGlobal(1.0);
		m_flTimeWeaponIdle = WEAPON_TIMEBASE + 1.0;
	}
}

void CCrowbar::WeaponIdle()
{
	if (m_flTimeWeaponIdle > WEAPON_TIMEBASE)
		return;

	if (m_iSwingMode == SWING_START_BIG)
	{
		if (gpGlobals->time > m_flBigSwingStart + 1)
		{
			m_iSwingMode = SWING_DOING_BIG;
			m_flTimeWeaponIdle = WEAPON_TIMEBASE + 1.2;
			SetThink(&CCrowbar::BigSwing);
			pev->nextthink = gpGlobals->time + 0.1;
		}
	}
	else
	{
		m_iSwingMode = SWING_NONE;
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		if (flRand <= 0.5)
		{
			iAnim = CROWBAR_IDLE3;
			m_flTimeWeaponIdle = WEAPON_TIMEBASE + 3.0;
		}
		else if (flRand <= 0.9)
		{
			iAnim = CROWBAR_IDLE1;
			m_flTimeWeaponIdle = WEAPON_TIMEBASE + 2.0;
		}
		else
		{
			iAnim = CROWBAR_IDLE2;
			m_flTimeWeaponIdle = WEAPON_TIMEBASE + 3.0;
		}

		SendWeaponAnim(iAnim);
	}
}

void CCrowbar::GetWeaponData(weapon_data_t& data)
{
	BaseClass::GetWeaponData(data);

	data.m_fInSpecialReload = static_cast<int>(m_iSwingMode);
}

void CCrowbar::SetWeaponData(const weapon_data_t& data)
{
	BaseClass::SetWeaponData(data);

	m_iSwingMode = data.m_fInSpecialReload;
}

int CCrowbar::iItemSlot()
{
	return 1;
}

int CCrowbar::GetItemInfo(ItemInfo* p)
{
	p->pszAmmo1 = nullptr;
	p->iMaxAmmo1 = WEAPON_NOCLIP;
	p->pszName = STRING(pev->classname);
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = WEAPON_NOCLIP;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_CROWBAR;
	p->iWeight = CROWBAR_WEIGHT;

	return true;
}

