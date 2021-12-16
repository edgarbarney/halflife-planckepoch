/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// monster template
//=========================================================
// UNDONE: Holster weapon?

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"talkmonster.h"
#include	"schedule.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"animation.h"
#include <FranUtils.hpp>

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?
#define		BARNEY_AE_DRAW		( 2 )
#define		BARNEY_AE_SHOOT		( 3 )
#define		BARNEY_AE_HOLSTER	( 4 )
#define		BARNEY_AE_RELOAD	( 31 )

enum
{
	SCHED_BARNEY_COVER_AND_RELOAD,
};

enum
{
	BRNWPN_GLOCK = 2,
	BRNWPN_MP5 = 3,
	BRNWPN_AR16 = 4,
};

enum
{
	BRNWPS_HOLSTER = 0,
	BRNWPS_EQUIP = 1,
	BRNWPS_DROP = 2,
};

namespace BarneySkin
{
	enum BarneySkin
	{
		Default = -1,
		White = 0,
		Black,
		Blond,
		SKINCOUNT = 2,
	};
}

#define	BARNEY_BODY_GUNHOLSTERED	0
#define	BARNEY_BODY_GUNDRAWN		1
#define BARNEY_BODY_GUNGONE			2

class CBarney : public CTalkMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  ISoundMask() override;
	void BarneyFirePistol();
	void AlertSound() override;
	int  Classify () override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	
	void RunTask( Task_t *pTask ) override;
	void StartTask( Task_t *pTask ) override;
    int	ObjectCaps() override { return CTalkMonster :: ObjectCaps() | FCAP_IMPULSE_USE; }
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;
	void CheckAmmo() override; // For Reload

	//For Multiple Weapons
	void SetWeaponBG(int weaponToWork, int gunstatus, bool forceglock);
	void SwitchWeapon(int weaponToWork, bool forcehand);
	void SetActivity(Activity NewActivity) override;
	void BarneyFireMP5();
	void BarneyFireAR16();

	void KeyValue(KeyValueData* pkvd) override;
	
	void DeclineFollowing() override;

	// Override these to set behavior
	Schedule_t *GetScheduleOfType ( int Type ) override;
	Schedule_t *GetSchedule () override;
	MONSTERSTATE GetIdealState () override;

	void DeathSound() override;
	void PainSound() override;
	
	void TalkInit();

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) override;
	void Killed( entvars_t *pevAttacker, int iGib ) override;

    int		Save( CSave &save ) override;
    int		Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];
	
	int		m_iBaseBody; //LRC - for barneys with different bodies
	BOOL	m_fGunDrawn;
	float	m_painTime;
	float	m_checkAttackTime;
	BOOL	m_lastAttackCheck;
	int		m_cClipSize;
	int		m_curWeapon;
	float	m_fireRate;

	// UNDONE: What is this for?  It isn't used?
	float	m_flPlayerDamage;// how much pain has the player inflicted on me?

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_barney, CBarney );

TYPEDESCRIPTION	CBarney::m_SaveData[] = 
{
	DEFINE_FIELD( CBarney, m_iBaseBody, FIELD_INTEGER ), //LRC
	DEFINE_FIELD( CBarney, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarney, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CBarney, m_checkAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CBarney, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarney, m_flPlayerDamage, FIELD_FLOAT ),
	DEFINE_FIELD( CBarney, m_cClipSize, FIELD_INTEGER),
	DEFINE_FIELD( CBarney, m_curWeapon, FIELD_INTEGER),
	DEFINE_FIELD( CBarney, m_voicePitch, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE( CBarney, CTalkMonster );

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlBaFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slBaFollow[] =
{
	{
		tlBaFollow,
		ARRAYSIZE ( tlBaFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"
	},
};

//=========================================================
// BarneyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlBarneyEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },
};

Schedule_t slBarneyEnemyDraw[] = 
{
	{
		tlBarneyEnemyDraw,
		ARRAYSIZE ( tlBarneyEnemyDraw ),
		0,
		0,
		"Barney Enemy Draw"
	}
};

Task_t	tlBarneyEnemyDrawRifle[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_ARM },
};

Schedule_t slBarneyEnemyDrawRifle[] =
{
	{
		tlBarneyEnemyDrawRifle,
		ARRAYSIZE(tlBarneyEnemyDrawRifle),
		0,
		0,
		"Barney Enemy Draw Rifle"
	}
};

Task_t	tlBaFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slBaFaceTarget[] =
{
	{
		tlBaFaceTarget,
		ARRAYSIZE ( tlBaFaceTarget ),
		bits_COND_CLIENT_PUSH	|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};


Task_t	tlIdleBaStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleBaStand[] =
{
	{ 
		tlIdleBaStand,
		ARRAYSIZE ( tlIdleBaStand ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};

Task_t	tlBarneyHideReload[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slBarneyHideReload[] =
{
	{
		tlBarneyHideReload,
		ARRAYSIZE(tlBarneyHideReload),
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,
		bits_SOUND_DANGER,
		"BarneyHideReload"
	}
};

Task_t	tlBarneyHideReloadMP5[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slBarneyHideReloadMP5[] =
{
	{
		tlBarneyHideReloadMP5,
		ARRAYSIZE(tlBarneyHideReloadMP5),
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,
		bits_SOUND_DANGER,
		"BarneyHideReloadMP5"
	}
};

Task_t	tlBarneyHideReloadAR16[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slBarneyHideReloadAR16[] =
{
	{
		tlBarneyHideReloadAR16,
		ARRAYSIZE(tlBarneyHideReloadAR16),
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,
		bits_SOUND_DANGER,
		"BarneyHideReloadAR16"
	}
};

DEFINE_CUSTOM_SCHEDULES( CBarney )
{
	slBaFollow,
	slBarneyEnemyDraw,
	slBaFaceTarget,
	slIdleBaStand,
	slBarneyHideReload,
	slBarneyHideReloadMP5,
	slBarneyHideReloadAR16,
	slBarneyEnemyDrawRifle,
};


IMPLEMENT_CUSTOM_SCHEDULES( CBarney, CTalkMonster );

void CBarney :: StartTask( Task_t *pTask )
{
	switch (pTask->iTask)
	{
	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;
	default:
		CTalkMonster::StartTask(pTask);
		break;
	}
}

void CBarney :: RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK1:
		if (m_hEnemy != nullptr && (m_hEnemy->IsPlayer()))
		{
			pev->framerate = 8;
		}
		CTalkMonster::RunTask( pTask );
		break;
	default:
		CTalkMonster::RunTask( pTask );
		break;
	}
}




//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CBarney :: ISoundMask () 
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_CARCASS	|
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER;
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CBarney :: Classify ()
{
	return m_iClass?m_iClass:CLASS_PLAYER_ALLY;
}

//=========================================================
// ALertSound - barney says "Freeze!"
//=========================================================
void CBarney :: AlertSound()
{
	if ( m_hEnemy != nullptr )
	{
		if ( FOkToSpeak() )
		{
			if (m_iszSpeakAs)
			{
				char szBuf[32];
				strcpy(szBuf,STRING(m_iszSpeakAs));
				strcat(szBuf,"_ATTACK");
				PlaySentence( szBuf, RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
			}
			else
			{
				PlaySentence( "BA_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
			}
		}
	}

}
//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CBarney :: SetYawSpeed ()
{
	int ys;

	ys = 0;

	switch ( m_Activity )
	{
	case ACT_IDLE:		
		ys = 70;
		break;
	case ACT_WALK:
		ys = 70;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	default:
		ys = 70;
		break;
	}

	pev->yaw_speed = ys;
}


//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CBarney :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( flDist <= 1024 && flDot >= 0.5 )
	{
		if ( gpGlobals->time > m_checkAttackTime )
		{
			TraceResult tr;
			
			Vector shootOrigin = pev->origin + Vector( 0, 0, 55 );
			CBaseEntity *pEnemy = m_hEnemy;
			Vector shootTarget = ( (pEnemy->BodyTarget( shootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP );
			UTIL_TraceLine( shootOrigin, shootTarget, dont_ignore_monsters, ENT(pev), &tr );
			//m_checkAttackTime = gpGlobals->time + m_fireRate;
			m_checkAttackTime = gpGlobals->time + 1;
			if ( tr.flFraction == 1.0 || (tr.pHit != nullptr && CBaseEntity::Instance(tr.pHit) == pEnemy) )
				m_lastAttackCheck = TRUE;
			else
				m_lastAttackCheck = FALSE;
			m_checkAttackTime = gpGlobals->time + m_fireRate;
		}
		return m_lastAttackCheck;
	}
	return FALSE;
}

#pragma region FiringFunctions

//=========================================================
// BarneyFireAR16 - shoots one round from the AR16 at
// the enemy barney is facing.
//=========================================================

void CBarney::BarneyFireAR16()
{
	if (m_hEnemy == nullptr && m_pCine == nullptr) //LRC - scripts may fire when you have no enemy
	{
		return;
	}

	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector(0, 0, 55);
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
	pev->effects = EF_MUZZLEFLASH;

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_556);

	int pitchShift = RANDOM_LONG(0, 20);

	// Only shift about half the time
	if (pitchShift > 10)
		pitchShift = 0;
	else
		pitchShift -= 5;

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/ar16_fire1.wav", 1, ATTN_NORM, 0, 100 + pitchShift);

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 384, 0.3);

	// UNDONE: Reload?
	m_cAmmoLoaded--;// take away a bullet!

	// Teh_Freak: World Lighting!
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgCreateDLight);
	//WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecShootOrigin.x); // origin
	WRITE_COORD(vecShootOrigin.y);
	WRITE_COORD(vecShootOrigin.z);
	WRITE_BYTE(16);     // radius
	WRITE_BYTE(255);     // R
	WRITE_BYTE(255);     // G
	WRITE_BYTE(128);     // B
	WRITE_BYTE(0);     // life * 10
	WRITE_BYTE(0); // decay
	MESSAGE_END();
	// Teh_Freak: World Lighting!

}

//=========================================================
// BarneyFireMP5 - shoots one round from the MP5 at
// the enemy barney is facing.
//=========================================================

void CBarney::BarneyFireMP5()
{
	if (m_hEnemy == nullptr && m_pCine == nullptr) //LRC - scripts may fire when you have no enemy
	{
		return;
	}

	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector(0, 0, 55);
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
	pev->effects = EF_MUZZLEFLASH;

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM);

	int pitchShift = RANDOM_LONG(0, 20);

	// Only shift about half the time
	if (pitchShift > 10)
		pitchShift = 0;
	else
		pitchShift -= 5;
	//weapons/ar16_fire1
	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM, 0, 100 + pitchShift);

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 384, 0.3);

	// UNDONE: Reload?
	m_cAmmoLoaded--;// take away a bullet!

	// Teh_Freak: World Lighting!
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgCreateDLight);
	//WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecShootOrigin.x); // origin
	WRITE_COORD(vecShootOrigin.y);
	WRITE_COORD(vecShootOrigin.z);
	WRITE_BYTE(16);     // radius
	WRITE_BYTE(255);     // R
	WRITE_BYTE(255);     // G
	WRITE_BYTE(128);     // B
	WRITE_BYTE(0);     // life * 10
	WRITE_BYTE(0); // decay
	MESSAGE_END();
	// Teh_Freak: World Lighting!

}

//=========================================================
// BarneyFirePistol - shoots one round from the pistol at
// the enemy barney is facing.
//=========================================================
void CBarney :: BarneyFirePistol ()
{
	if (m_hEnemy == nullptr && m_pCine == nullptr) //LRC - scripts may fire when you have no enemy
	{
		return;
	}

	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
	pev->effects = EF_MUZZLEFLASH;

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM );

	int pitchShift = RANDOM_LONG( 0, 20 );
	
	// Only shift about half the time
	if ( pitchShift > 10 )
		pitchShift = 0;
	else
		pitchShift -= 5;

	EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/pl_gun3.wav", 1, ATTN_NORM, 0, 100 + pitchShift );

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );

	// UNDONE: Reload?
	m_cAmmoLoaded--;// take away a bullet!

	// Teh_Freak: World Lighting!
		FranUtils::EmitDlight(pev->origin, 16, { 255, 255, 160 }, 0.05f, 0);
	// Teh_Freak: World Lighting!

}

#pragma endregion

//=========================================================
// CheckAmmo - overridden for the barney because like
// hgrunt, he actually uses ammo! (base class doesn't)
//=========================================================
void CBarney::CheckAmmo()
{
	if (m_cAmmoLoaded <= 0)
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CBarney :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BARNEY_AE_SHOOT:
		switch(m_curWeapon)
		{
			case BRNWPN_MP5:
				BarneyFireMP5();
				break;
			case BRNWPN_AR16:
				BarneyFireAR16();
				break;
			case BRNWPN_GLOCK:
			default:
				BarneyFirePistol();
				break;
		}
		break;

	case BARNEY_AE_RELOAD:
		m_cAmmoLoaded = m_cClipSize;
		ClearConditions(bits_COND_NO_AMMO_LOADED);
		break;

	case BARNEY_AE_DRAW:
		// barney's bodygroup switches here so he can pull gun from holster
		SetBodygroup(m_curWeapon, BRNWPS_EQUIP);
		m_fGunDrawn = TRUE;
		break;

	case BARNEY_AE_HOLSTER:
		// change bodygroup to replace gun in holster
		SetBodygroup(m_curWeapon, BRNWPS_HOLSTER);
		m_fGunDrawn = FALSE;
		break;
	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}

#pragma region MultipleWeapons

//=========================================================
// Set Weapon Bodygroup
//=========================================================
void CBarney::SetWeaponBG(int weaponToWork, int gunstatus, bool forceglock)
{
	int glokstatus = BRNWPS_HOLSTER;

	if (forceglock)
	{
		glokstatus = BRNWPS_DROP;
	}

	switch (weaponToWork)
	{
	default:
	case BRNWPN_GLOCK:
		SetBodygroup(BRNWPN_GLOCK, gunstatus);
		SetBodygroup(BRNWPN_MP5, BRNWPS_DROP);
		SetBodygroup(BRNWPN_AR16, BRNWPS_DROP);
		break;
	case BRNWPN_MP5:
		SetBodygroup(BRNWPN_GLOCK, glokstatus);
		SetBodygroup(BRNWPN_MP5, gunstatus);
		SetBodygroup(BRNWPN_AR16, BRNWPS_DROP);
		break;
	case BRNWPN_AR16:
		SetBodygroup(BRNWPN_GLOCK, glokstatus);
		SetBodygroup(BRNWPN_MP5, BRNWPS_DROP);
		SetBodygroup(BRNWPN_AR16, gunstatus);
		break;
	}
}

//=========================================================
// Switch Weapon
//=========================================================
void CBarney::SwitchWeapon(int weaponToWork, bool forcehand)
{
	int gunstatus = BRNWPS_HOLSTER;

	if (forcehand)
	{
		gunstatus = BRNWPS_EQUIP;
	}
	else
	{
		m_fGunDrawn = false;
	}

	switch (weaponToWork)
	{
	default:
	case BRNWPN_GLOCK:
		SetBodygroup(BRNWPN_GLOCK, gunstatus);
		SetBodygroup(BRNWPN_MP5, BRNWPS_DROP);
		SetBodygroup(BRNWPN_AR16, BRNWPS_DROP);
		m_curWeapon = BRNWPN_GLOCK;
		m_cClipSize = GLOCK_MAX_CLIP;
		break;
	case BRNWPN_MP5:
		SetBodygroup(BRNWPN_GLOCK, BRNWPS_HOLSTER);
		SetBodygroup(BRNWPN_MP5, gunstatus);
		SetBodygroup(BRNWPN_AR16, BRNWPS_DROP);
		m_curWeapon = BRNWPN_MP5;
		m_cClipSize = MP5_MAX_CLIP;
		break;
	case BRNWPN_AR16:
		SetBodygroup(BRNWPN_GLOCK, BRNWPS_HOLSTER);
		SetBodygroup(BRNWPN_MP5, BRNWPS_DROP);
		SetBodygroup(BRNWPN_AR16, gunstatus);
		m_curWeapon = BRNWPN_AR16;
		m_cClipSize = AR16_MAX_CLIP;
		break;
	}
}

#pragma endregion

//=========================================================
// Spawn
//=========================================================
void CBarney :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/barney.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;
	
	//m_fGunDrawn			= FALSE;	

	m_fireRate = 0.01;
	//m_curWeapon = BRNWPN_GLOCK;

	SwitchWeapon(m_curWeapon, false);
	//SetWeaponBG(m_curWeapon, BRNWPS_HOLSTER, false);

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_cAmmoLoaded = m_cClipSize;

	// Random skin for Barney
	if (pev->skin == -1) 
	{
		pev->skin = RANDOM_LONG(0, BarneySkin::SKINCOUNT);
	}

	// Get voice for head
	if (pev->skin == BarneySkin::Black)
	{
		m_voicePitch = 88 + RANDOM_LONG(0, 4);
	}
	else
	{
		m_voicePitch = 100;
	}
		
	MonsterInit();
	SetUse( &CBarney::FollowerUse );
}

void CBarney::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wpnkey"))
	{
		if (atoi(pkvd->szValue) == BRNWPN_AR16 || atoi(pkvd->szValue) == BRNWPN_MP5 || atoi(pkvd->szValue) == BRNWPN_GLOCK)
			m_curWeapon = atoi(pkvd->szValue);
		else
			m_curWeapon = BRNWPN_GLOCK;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBarney :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/barney.mdl");

	PRECACHE_SOUND("barney/ba_attack1.wav" );
	PRECACHE_SOUND("weapons/hks1.wav" );
	PRECACHE_SOUND("weapons/ar16_fire1.wav");
	PRECACHE_SOUND("weapons/pl_gun3.wav");

	PRECACHE_SOUND("barney/ba_pain1.wav");
	PRECACHE_SOUND("barney/ba_pain2.wav");
	PRECACHE_SOUND("barney/ba_pain3.wav");

	PRECACHE_SOUND("barney/ba_die1.wav");
	PRECACHE_SOUND("barney/ba_die2.wav");
	PRECACHE_SOUND("barney/ba_die3.wav");

	
	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}	

// Init talk data
void CBarney :: TalkInit()
{
	
	CTalkMonster::TalkInit();

	// barney speech group names (group names are in sentences.txt)

	if (!m_iszSpeakAs)
	{
		m_szGrp[TLK_ANSWER]		=	"BA_ANSWER";
		m_szGrp[TLK_QUESTION]	=	"BA_QUESTION";
		m_szGrp[TLK_IDLE]		=	"BA_IDLE";
		m_szGrp[TLK_STARE]		=	"BA_STARE";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER) //LRC
			m_szGrp[TLK_USE]	=	"BA_PFOLLOW";
		else
			m_szGrp[TLK_USE] =	"BA_OK";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER)
			m_szGrp[TLK_UNUSE] = "BA_PWAIT";
		else
			m_szGrp[TLK_UNUSE] = "BA_WAIT";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER)
			m_szGrp[TLK_DECLINE] =	"BA_POK";
		else
			m_szGrp[TLK_DECLINE] =	"BA_NOTOK";
		m_szGrp[TLK_STOP] =		"BA_STOP";

		m_szGrp[TLK_NOSHOOT] =	"BA_SCARED";
		m_szGrp[TLK_HELLO] =	"BA_HELLO";

		m_szGrp[TLK_PLHURT1] =	"!BA_CUREA";
		m_szGrp[TLK_PLHURT2] =	"!BA_CUREB"; 
		m_szGrp[TLK_PLHURT3] =	"!BA_CUREC";

		m_szGrp[TLK_PHELLO] =	nullptr;	//"BA_PHELLO";		// UNDONE
		m_szGrp[TLK_PIDLE] =	nullptr;	//"BA_PIDLE";			// UNDONE
		m_szGrp[TLK_PQUESTION] = "BA_PQUEST";		// UNDONE

		m_szGrp[TLK_SMELL] =	"BA_SMELL";
	
		m_szGrp[TLK_WOUND] =	"BA_WOUND";
		m_szGrp[TLK_MORTAL] =	"BA_MORTAL";
	}	
}


BOOL IsFacing(entvars_t* pevTest, const Vector& reference)
{
	Vector vecDir = (reference - pevTest->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector forward, angle;
	angle = pevTest->v_angle;
	angle.x = 0;
	UTIL_MakeVectorsPrivate( angle, forward, nullptr, nullptr );
	// He's facing me, he meant it
	if (DotProduct(forward, vecDir) > 0.96)	// +/- 15 degrees or so
	{
		return TRUE;
	}
	return FALSE;
}


int CBarney :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	if ( !IsAlive() || pev->deadflag == DEAD_DYING )
		return ret;

	// LRC - if my reaction to the player has been overridden, don't do this stuff
	if (m_iPlayerReact) return ret;

	if ( m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) )
	{
		m_flPlayerDamage += flDamage;

		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if ( m_hEnemy == nullptr )
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if ( (m_afMemory & bits_MEMORY_SUSPICIOUS) || IsFacing( pevAttacker, pev->origin ) )
			{
				// Alright, now I'm pissed!
				if (m_iszSpeakAs)
				{
					char szBuf[32];
					strcpy(szBuf,STRING(m_iszSpeakAs));
					strcat(szBuf,"_MAD");
					PlaySentence( szBuf, 4, VOL_NORM, ATTN_NORM );
				}
				else
				{
					PlaySentence( "BA_MAD", 4, VOL_NORM, ATTN_NORM );
				}

				Remember( bits_MEMORY_PROVOKED );
				StopFollowing( TRUE );
			}
			else
			{
				// Hey, be careful with that
				if (m_iszSpeakAs)
				{
					char szBuf[32];
					strcpy(szBuf,STRING(m_iszSpeakAs));
					strcat(szBuf,"_SHOT");
					PlaySentence( szBuf, 4, VOL_NORM, ATTN_NORM );
				}
				else
				{
					PlaySentence( "BA_SHOT", 4, VOL_NORM, ATTN_NORM );
				}
				Remember( bits_MEMORY_SUSPICIOUS );
			}
		}
		else if ( !(m_hEnemy->IsPlayer()) && pev->deadflag == DEAD_NO )
		{
			if (m_iszSpeakAs)
			{
				char szBuf[32];
				strcpy(szBuf,STRING(m_iszSpeakAs));
				strcat(szBuf,"_SHOT");
				PlaySentence( szBuf, 4, VOL_NORM, ATTN_NORM );
			}
			else
			{
				PlaySentence( "BA_SHOT", 4, VOL_NORM, ATTN_NORM );
			}
		}
	}

	return ret;
}

	
//=========================================================
// PainSound
//=========================================================
void CBarney :: PainSound ()
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	switch (RANDOM_LONG(0,2))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CBarney :: DeathSound ()
{
	switch (RANDOM_LONG(0,2))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}


void CBarney::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	switch( ptr->iHitgroup)
	{
	//case HITGROUP_CHEST:
	//case HITGROUP_STOMACH:
	//	if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST))
	//	{
	//		flDamage = flDamage / 2;
	//	}
	//	break;
	case 10:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
		{
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet(ptr->vecEndPos, 1.0);
				flDamage = 0.01;
			}
		}
		// always a head shot
		ptr->iHitgroup = HITGROUP_HEAD;
		break;
	case 11:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
		{
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet(ptr->vecEndPos, 1.0);
				flDamage = 0.01;
			}
		}
		ptr->iHitgroup = HITGROUP_CHEST;
		break;
	case 12:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
		{
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet(ptr->vecEndPos, 1.0);
				flDamage = 0.01;
			}
		}
		ptr->iHitgroup = HITGROUP_STOMACH;
		break;
	}

	CTalkMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}


void CBarney::Killed( entvars_t *pevAttacker, int iGib )
{
	if (GetBodygroup(2) <  BARNEY_BODY_GUNGONE && !(pev->spawnflags & SF_MONSTER_NO_WPN_DROP))
	{// drop the gun!
		Vector vecGunPos;
		Vector vecGunAngles;
		
		SetWeaponBG(m_curWeapon, BRNWPS_DROP, true);
		GetAttachment( 0, vecGunPos, vecGunAngles );

		CBaseEntity* pGun;
		CBaseEntity* pGun2;

		switch (m_curWeapon)
		{
		case BRNWPN_MP5:
			pGun = DropItem("weapon_9mmhandgun", vecGunPos, vecGunAngles);
			pGun2 = DropItem("weapon_9mmAR", Vector(vecGunPos.x, vecGunPos.y + 10, vecGunPos.z + 1), vecGunAngles);
			break;
		case BRNWPN_AR16:
			pGun = DropItem("weapon_9mmhandgun", vecGunPos, vecGunAngles);
			pGun2 = DropItem("weapon_556AR", Vector(vecGunPos.x, vecGunPos.y + 10, vecGunPos.z + 1), vecGunAngles);
			break;
		case BRNWPN_GLOCK:
		default:
			pGun = DropItem("weapon_9mmhandgun", vecGunPos, vecGunAngles);
			break;
		}
	}

	SetUse( nullptr );	
	CTalkMonster::Killed( pevAttacker, iGib );
}

//==========================================================
// SetActivity 
//=========================================================
void CBarney::SetActivity(Activity NewActivity)
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	switch (NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		switch (m_curWeapon)
		{
		case BRNWPN_MP5:
			iSequence = LookupSequence("shootmp5");
			break;
		case BRNWPN_AR16:
			iSequence = LookupSequence("shootar16");
			break;
		case BRNWPN_GLOCK:
		default:
			iSequence = LookupSequence("shootgun2");
			break;
		}
		break;
	case ACT_RELOAD:
		switch (m_curWeapon)
		{
		case BRNWPN_MP5:
			iSequence = LookupSequence("reloadmp5");
			break;
		case BRNWPN_AR16:
			iSequence = LookupSequence("reloadar16");
			break;
		case BRNWPN_GLOCK:
		default:
			iSequence = LookupSequence("reload");
			break;
		}
		break;
	case ACT_ARM:
		switch (m_curWeapon)
		{
		case BRNWPN_MP5:
		case BRNWPN_AR16:
			iSequence = LookupSequence("drawrifle");
			break;
		case BRNWPN_GLOCK:
		default:
			iSequence = LookupSequence("draw");
			break;
		}
		break;
	default:
		iSequence = LookupActivity(NewActivity);
		break;
	}

	m_Activity = NewActivity;

	if (iSequence > ACTIVITY_NOT_AVAILABLE)
	{
		if (pev->sequence != iSequence || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		pev->sequence = iSequence;
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		pev->sequence = 0;
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

Schedule_t* CBarney :: GetScheduleOfType ( int Type )
{
	Schedule_t *psched;

	switch( Type )
	{
	case SCHED_ARM_WEAPON:
		if ( m_hEnemy != nullptr )
		{
			// face enemy, then draw.
			return slBarneyEnemyDraw;
		}
		break;
	case SCHED_BARNEY_COVER_AND_RELOAD:
	{
		switch (m_curWeapon)
		{
		case BRNWPN_MP5:
			return &slBarneyHideReloadMP5[0];
			break;
		case BRNWPN_AR16:
			return &slBarneyHideReloadAR16[0];
			break;
		case BRNWPN_GLOCK:
		default:
			return &slBarneyHideReload[0];
			break;
		}
	}
	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that barney will talk
		// when 'used' 
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slBaFaceTarget;	// override this for different target face behavior
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slBaFollow;

	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
		{
			// just look straight ahead.
			return slIdleBaStand;
		}
		else
			return psched;	
	}

	return CTalkMonster::GetScheduleOfType( Type );
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CBarney :: GetSchedule ()
{
	if (HasConditions(bits_COND_NO_AMMO_LOADED))
	{
			//!!!KELLY - this individual just realized he's out of bullet ammo. 
			// He's going to try to find cover to run to and reload, but rarely, if 
			// none is available, he'll drop and reload in the open here. 
	return GetScheduleOfType(SCHED_BARNEY_COVER_AND_RELOAD);
	}

	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != nullptr );
		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
	}
	if ( HasConditions( bits_COND_ENEMY_DEAD ) && FOkToSpeak() )
	{
		// Hey, be careful with that
		if (m_iszSpeakAs)
		{
			char szBuf[32];
			strcpy(szBuf,STRING(m_iszSpeakAs));
			strcat(szBuf,"_KILL");
			PlaySentence( szBuf, 4, VOL_NORM, ATTN_NORM );
		}
		else
		{
			PlaySentence( "BA_KILL", 4, VOL_NORM, ATTN_NORM );
		}
	}

	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster :: GetSchedule();
			}

			// always act surprized with a new enemy
			if ( HasConditions( bits_COND_NEW_ENEMY ) && HasConditions( bits_COND_LIGHT_DAMAGE) )
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
				
			// wait for one schedule to draw gun
			if (!m_fGunDrawn )
				return GetScheduleOfType( SCHED_ARM_WEAPON );

			if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
		}
		break;

	case MONSTERSTATE_ALERT:	
	case MONSTERSTATE_IDLE:
		if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType( SCHED_SMALL_FLINCH );
		}

		if ( m_hEnemy == nullptr && IsFollowing() )
		{
			if ( !m_hTargetEnt->IsAlive() )
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing( FALSE );
				break;
			}
			else
			{
				if ( HasConditions( bits_COND_CLIENT_PUSH ) )
				{
					return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
				}
				return GetScheduleOfType( SCHED_TARGET_FACE );
			}
		}

		if ( HasConditions( bits_COND_CLIENT_PUSH ) )
		{
			return GetScheduleOfType( SCHED_MOVE_AWAY );
		}

		// try to say something about smells
		TrySmellTalk();
		break;
	}
	
	return CTalkMonster::GetSchedule();
}

MONSTERSTATE CBarney :: GetIdealState ()
{
	return CTalkMonster::GetIdealState();
}



void CBarney::DeclineFollowing()
{
	PlaySentence( m_szGrp[TLK_DECLINE], 2, VOL_NORM, ATTN_NORM ); //LRC
}





//=========================================================
// DEAD BARNEY PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iFirstPose properly!
//
//=========================================================
class CDeadBarney : public CBaseMonster
{
public:
	void Spawn() override;
	int	Classify () override { return	CLASS_PLAYER_ALLY; }

	void KeyValue( KeyValueData *pkvd ) override;

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static const char *m_szPoses[3];
};

const char *CDeadBarney::m_szPoses[] = { "lying_on_back", "lying_on_side", "lying_on_stomach" };

void CDeadBarney::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_barney_dead, CDeadBarney );

//=========================================================
// ********** DeadBarney SPAWN **********
//=========================================================
void CDeadBarney :: Spawn( )
{
	PRECACHE_MODEL("models/barney.mdl");
	SET_MODEL(ENT(pev), "models/barney.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor		= BLOOD_COLOR_RED;


	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if (pev->sequence == -1)
	{
		ALERT ( at_debug, "Dead barney with bad pose\n" );
	}
	// Corpses have less health
	pev->health			= 8;//gSkillData.barneyHealth;

	MonsterInitDead();
}


