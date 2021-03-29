//FranticDreamer AR16 Code
//Mostly based on MP5's code

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

LINK_ENTITY_TO_CLASS(weapon_ar16, CAR16);
LINK_ENTITY_TO_CLASS(weapon_556AR, CAR16);


//=========================================================
//=========================================================
int CAR16::SecondaryAmmoIndex(void)
{
	return m_iSecondaryAmmoType;
}

void CAR16::Spawn()
{
	pev->classname = MAKE_STRING("weapon_ar16"); // hack to allow for old names
	Precache();
	SET_MODEL(ENT(pev), "models/w_ar16.mdl");
	m_iId = WEAPON_AR16;

	m_iDefaultAmmo = AR16_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CAR16::Precache(void)
{
	PRECACHE_MODEL("models/v_ar16.mdl");
	PRECACHE_MODEL("models/w_ar16.mdl");
	PRECACHE_MODEL("models/p_ar16.mdl");// not needed actually since this mod is not multiplayer

	m_iShell = PRECACHE_MODEL("models/556shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/w_ar16_magpale.mdl");
	PRECACHE_MODEL("models/w_ar16clip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/ar16_magin.wav");
	PRECACHE_SOUND("weapons/ar16_magrel.wav");

	PRECACHE_SOUND("weapons/ar16_fire1.wav");
	PRECACHE_SOUND("weapons/ar16_fire2.wav");
	PRECACHE_SOUND("weapons/ar16_fire3.wav");

	PRECACHE_SOUND("weapons/ar16_m203_1.wav");
	PRECACHE_SOUND("weapons/ar16_m203_2.wav");

	PRECACHE_SOUND("weapons/357_cock1.wav");

	m_usAR16 = PRECACHE_EVENT(1, "events/ar16.sc");
	m_usAR162 = PRECACHE_EVENT(1, "events/ar162.sc");
	m_usAR163 = PRECACHE_EVENT(1, "events/ar163.sc");
}

int CAR16::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556";
	p->iMaxAmmo1 = _556_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = AR16_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_AR16;
	p->iWeight = AR16_WEIGHT;

	return 1;
}

int CAR16::AddToPlayer(CBasePlayer* pPlayer)
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

BOOL CAR16::Deploy()
{
	return DefaultDeploy("models/v_ar16.mdl", "models/p_ar16.mdl", AR16_DEPLOY, "ar16");
}

void CAR16::Holster(int skiplocal /* = 0 */)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(AR16_HOLSTER);
}

void CAR16::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL 
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(pev->origin.x); // origin
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE(16);     // radius
	WRITE_BYTE(255);    // R
	WRITE_BYTE(255);    // G
	WRITE_BYTE(160);    // B
	WRITE_BYTE(0);      // life * 10
	WRITE_BYTE(0);      // decay
	MESSAGE_END();
#endif 

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector vecDir;

#ifdef CLIENT_DLL
	if (!bIsMultiplayer())
#else
	if (!g_pGameRules->IsMultiplayer())
#endif
	{
		// optimized multiplayer. Widened to make it easier to hit a moving player
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_556, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	}
	else
	{
		// single player spread
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_556, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	//	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usAR16, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usAR16, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, RANDOM_LONG(AR16_FIRE1, AR16_FIRE3), m_iShell, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}



void CAR16::SecondaryAttack(void)
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	// we don't add in player velocity anymore.
	CGrenade::ShootContact(m_pPlayer->pev,
		m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16,
		gpGlobals->v_forward * 1000); // Speed up the grenade

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usAR163);
	}
	else 
	{
		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usAR162);
	}
	m_flNextPrimaryAttack = GetNextAttackDelay(2); // Modified M203 delay
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;// idle pretty soon after shooting.

	if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_pPlayer->pev->punchangle.x -= 10;
}

void CAR16::Reload(void)
{
	if (m_pPlayer->ammo_556 <= 0)
		return;

	DefaultReload(AR16_MAX_CLIP, AR16_RELOAD, 1.5);
}


void CAR16::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = AR16_LONGIDLE;
		break;

	default:
	case 1:
		iAnim = AR16_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
}



class CAR16AmmoClip : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_ar16clip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_ar16clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_AR16CLIP_GIVE, "556", _556_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_556mag, CAR16AmmoClip);
LINK_ENTITY_TO_CLASS(ammo_556, CAR16AmmoClip);



class CAR16Chainammo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_ar16_magpale.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_ar16_magpale.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_CHAINBOX_GIVE, "556", _556_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_ar16box, CAR16Chainammo);
LINK_ENTITY_TO_CLASS(ammo_556box, CAR16Chainammo);


class CAR16AmmoGrenade : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_ARgrenade.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_M203BOX_GIVE, "ARgrenades", M203_GRENADE_MAX_CARRY) != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5grenades, CAR16AmmoGrenade); // For legacy MP5 grenade support. Spawners etc.
LINK_ENTITY_TO_CLASS(ammo_ar16grenades, CAR16AmmoGrenade);
LINK_ENTITY_TO_CLASS(ammo_ARgrenades, CAR16AmmoGrenade);


















