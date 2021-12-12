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
#include "soundent.h"
#include "gamerules.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS( weapon_nyangun, CNyanGun );
LINK_ENTITY_TO_CLASS( weapon_egon,    CNyanGun );

//=========================================================
//=========================================================
void CNyanGun::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_nyangun"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_9mmAR.mdl");
	m_iId = WEAPON_NYANGUN;

	m_iDefaultAmmo = NYANGUN_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CNyanGun::Precache()
{
	PRECACHE_MODEL("models/v_nyangun.mdl");
	PRECACHE_MODEL("models/w_9mmAR.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND ("nyancat/nyan_start.wav");
	PRECACHE_SOUND ("nyancat/nyan_fireloop.wav");
	PRECACHE_SOUND ("nyancat/nyan_idleloop.wav");

	PRECACHE_SOUND( "weapons/glauncher.wav" );
	PRECACHE_SOUND( "weapons/glauncher2.wav" );

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	UTIL_PrecacheOther("nyancat");
	UTIL_PrecacheOther("bignyancat");

	m_usNYANGUN = PRECACHE_EVENT( 1, "events/nyangun.sc" );
	m_usNYANGUN2 = PRECACHE_EVENT( 1, "events/nyangun2.sc" );
}

int CNyanGun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "nyanammo";
	p->iMaxAmmo1 = NYANAMMO_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_NYANGUN;
	p->iWeight = NYANGUN_WEIGHT;

	return 1;
}

int CNyanGun::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CNyanGun::Deploy( )
{
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_start.wav", 1, ATTN_IDLE, 0, 100);
	return DefaultDeploy( "models/v_nyangun.mdl", "models/p_9mmAR.mdl", NYANGUN_DEPLOY, "nyangun" );
}

void CNyanGun::Holster()
{
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_start.wav",	 0, ATTN_IDLE, SND_STOP, 100);
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_idleloop.wav", 0, ATTN_IDLE, SND_STOP, 100);
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_fireloop.wav", 0, ATTN_IDLE, SND_STOP, 100);
}


void CNyanGun::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	CBaseEntity* pNyanCat = CBaseEntity::Create("nyancat", m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12, m_pPlayer->pev->v_angle, m_pPlayer->edict());
	pNyanCat->pev->velocity = gpGlobals->v_forward * 800;
	pNyanCat->pev->fuser1 = 800;

#endif

  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usNYANGUN, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
}



void CNyanGun::SecondaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound( );
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;
			
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/glauncher.wav", 1, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 0xf));
		break;
	case 1:
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/glauncher2.wav", 1, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 0xf));
		break;
	}

#ifndef CLIENT_DLL
 	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	// we don't add in player velocity anymore.
	//m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16

	CBaseEntity* pNyanCat = CBaseEntity::Create("bignyancat", m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16, m_pPlayer->pev->v_angle, m_pPlayer->edict());
	pNyanCat->pev->velocity = gpGlobals->v_forward * 300;
	pNyanCat->pev->fuser1 = 300;
#endif

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usNYANGUN2 );
	
	m_flNextPrimaryAttack = GetNextAttackDelay(1);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;

	if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
}

void CNyanGun::Reload()
{
	if ( m_pPlayer->ammo_nyanammo <= 0 )
		return;

	DefaultReload( NYANGUN_MAX_CLIP, NYANGUN_RELOAD, 1.5 );
}


void CNyanGun::WeaponIdle()
{
	ResetEmptySound( );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;

	/*
	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = NYANGUN_LONGIDLE;	
		break;
	
	default:
	case 1:
		iAnim = NYANGUN_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
	*/
}

BOOL CNyanGun::PlayEmptySound()
{
	if (m_iPlayEmptySound)
	{
		EMIT_SOUND(edict(), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
		m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

void CNyanGun::KeyPressed_PrimaryAttack()
{
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_start.wav",    0, ATTN_IDLE, SND_STOP, 100);
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_idleloop.wav", 0, ATTN_IDLE, SND_STOP, 100);

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_fireloop.wav", 1, ATTN_IDLE, 0, 100);

	CBasePlayerWeapon::KeyPressed_PrimaryAttack();
}

void CNyanGun::KeyReleased_PrimaryAttack()
{
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_start.wav",    0, ATTN_IDLE, SND_STOP, 100);
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_fireloop.wav", 0, ATTN_IDLE, SND_STOP, 100);

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_idleloop.wav", 1, ATTN_IDLE, 0, 100);

	CBasePlayerWeapon::KeyReleased_PrimaryAttack();
}

/*
void CNyanGun::KeyPressed_SecondaryAttack()
{
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_start.wav", 0, ATTN_IDLE, SND_STOP, 100);
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_idleloop.wav", 0, ATTN_IDLE, SND_STOP, 100);

	CBasePlayerWeapon::KeyPressed_SecondaryAttack();
}

void CNyanGun::KeyReleased_SecondaryAttack()
{
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_start.wav", 0, ATTN_IDLE, SND_STOP, 100);
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "nyancat/nyan_idleloop.wav", 1, ATTN_IDLE, 0, 100);

	CBasePlayerWeapon::KeyReleased_PrimaryAttack();
}
*/

class CNyanGunAmmoClip : public CBasePlayerAmmo
{
	void Spawn() override
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache() override
	{
		PRECACHE_MODEL ("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) override
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_NYANGUNCLIP_GIVE, "nyanammo", NYANAMMO_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_nyangunclip, CNyanGunAmmoClip );


class CNyanGunAmmoGrenade : public CBasePlayerAmmo
{
	void Spawn() override
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache() override
	{
		PRECACHE_MODEL ("models/w_ARgrenade.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) override
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_M203BOX_GIVE, "ARgrenades", M203_GRENADE_MAX_CARRY ) != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_nyangungrenades, CNyanGunAmmoGrenade );



//special thanks to FranticDreamer














