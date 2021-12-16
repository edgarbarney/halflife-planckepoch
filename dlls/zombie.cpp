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
// Zombie
//=========================================================

// UNDONE: Don't flinch every time you get hit

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	ZOMBIE_AE_ATTACK_RIGHT		0x01
#define	ZOMBIE_AE_ATTACK_LEFT		0x02
#define	ZOMBIE_AE_ATTACK_BOTH		0x03

// Headcrab jumps from zombie
#define     ZOMBIE_AE_CRAB1          0x04
#define     ZOMBIE_AE_CRAB2          0x05
#define     ZOMBIE_AE_CRAB3          0x06

#define ZOMBIE_FLINCH_DELAY			2		// at most one flinch every n secs

#define	TOTALZOMBIETYPES			4
enum class zombieTypeEnum { scientist, barney, hgrunt, blops };

#define ZOMBIE_CRAB          "monster_headcrab"

class CZombie : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify () override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	int IgnoreConditions () override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	void SpawnCrab();
	void RemoveKrabby();
	zombieTypeEnum m_zombieType;

	float m_flNextFlinch;

	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;
	void AttackSound();

	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pIdleSoundsGrunt[];
	static const char* pAlertSounds[];
	static const char* pAlertSoundsGrunt[];
	static const char* pPainSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

	// No range attacks
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override { return FALSE; }
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) override { return FALSE; }
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) override;
};

LINK_ENTITY_TO_CLASS(monster_zombie, CZombie);

const char* CZombie::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CZombie::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CZombie::pAttackSounds[] =
{
	"zombie/zo_attack1.wav",
	"zombie/zo_attack2.wav",
};

const char* CZombie::pIdleSounds[] =
{
	"zombie/zo_idle1.wav",
	"zombie/zo_idle2.wav",
	"zombie/zo_idle3.wav",
	"zombie/zo_idle4.wav",
};

const char* CZombie::pIdleSoundsGrunt[] =
{
	"zombie/zo_grunt_idle1.wav",
	"zombie/zo_grunt_idle2.wav",
	"zombie/zo_grunt_idle3.wav",
	"zombie/zo_grunt_idle4.wav",
};

const char* CZombie::pAlertSounds[] =
{
	"zombie/zo_alert10.wav",
	"zombie/zo_alert20.wav",
	"zombie/zo_alert30.wav",
};

const char* CZombie::pAlertSoundsGrunt[] =
{
	"zombie/zo_grunt_alert10.wav",
	"zombie/zo_grunt_alert20.wav",
	"zombie/zo_grunt_alert30.wav",
};

const char* CZombie::pPainSounds[] =
{
	"zombie/zo_pain1.wav",
	"zombie/zo_pain2.wav",
};

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CZombie :: Classify ()
{
	return m_iClass?m_iClass:CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CZombie :: SetYawSpeed ()
{
	int ys;

	ys = 120;

#if 0
	switch (m_Activity)
	{
	}
#endif

	pev->yaw_speed = ys;
}

int CZombie::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// Take 30% damage from bullets
	if (bitsDamageType == DMG_BULLET)
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();
		float flForce = DamageForce(flDamage);
		pev->velocity = pev->velocity + vecDir * flForce;
		flDamage *= 0.3;
	}

	// HACK HACK -- until we fix this.
	if (IsAlive())
		PainSound();
	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CZombie :: PainSound()
{
	int pitch = 0;
	if (RANDOM_LONG(0, 5) < 2) {
		switch (m_zombieType)
		{
		case zombieTypeEnum::hgrunt:
			pitch = 100 + RANDOM_LONG(-30, -15);
			break;
		case zombieTypeEnum::blops:
			pitch = 100 + RANDOM_LONG(-30, -15);
			break;
		default:
			pitch = 95 + RANDOM_LONG(0, 9);
			break;
		}
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
	}
}

void CZombie :: AlertSound()
{
	int pitch = 0;
	switch (m_zombieType)
	{
	case zombieTypeEnum::hgrunt:
		pitch = 100 + RANDOM_LONG(-30, +15);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSoundsGrunt[RANDOM_LONG(0, ARRAYSIZE(pAlertSoundsGrunt) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	case zombieTypeEnum::blops:
		//Black Ops Zombies are mostly silent
		//pitch = 100 + RANDOM_LONG(-30, +15);
		//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSoundsGrunt[RANDOM_LONG(0, ARRAYSIZE(pAlertSoundsGrunt) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	default:
		pitch = 95 + RANDOM_LONG(0, 9);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0, ARRAYSIZE(pAlertSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	}
}

void CZombie :: IdleSound()
{
	int pitch = 0;
	switch (m_zombieType)
	{
	case zombieTypeEnum::hgrunt:
		pitch = 100 + RANDOM_LONG(-30, +15);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pIdleSoundsGrunt[RANDOM_LONG(0, ARRAYSIZE(pIdleSoundsGrunt) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	case zombieTypeEnum::blops:
		//Black Ops Zombies are mostly silent
		//pitch = 100 + RANDOM_LONG(-30, +15);
		//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pIdleSoundsGrunt[RANDOM_LONG(0, ARRAYSIZE(pIdleSoundsGrunt) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	default:
		pitch = 100 + RANDOM_LONG(-5, 5);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pIdleSounds[RANDOM_LONG(0, ARRAYSIZE(pIdleSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	}
}

void CZombie :: AttackSound()
{
	int pitch = 0;
	switch (m_zombieType)
	{
	case zombieTypeEnum::hgrunt:
		pitch = 100 + RANDOM_LONG(-30, -15);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	case zombieTypeEnum::blops:
		//Black Ops Zombies are mostly silent
		//pitch = 100 + RANDOM_LONG(-30, -15);
		//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	default:
		pitch = 100 + RANDOM_LONG(-5, 5);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	}
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombie::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case ZOMBIE_AE_ATTACK_RIGHT:
	{
		// do stuff for this event.
//		ALERT( at_console, "Slash right!\n" );
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgOneSlash, DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
			}
			// Play a random attack hit sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackHitSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else // Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_LEFT:
	{
		// do stuff for this event.
//		ALERT( at_console, "Slash left!\n" );
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgOneSlash, DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.z = 18;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 100;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackHitSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_BOTH:
	{
		// do stuff for this event.
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgBothSlash, DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackHitSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	// Krusty Krabs
	case ZOMBIE_AE_CRAB1 :
	{
		RemoveKrabby();
		SpawnCrab(); // Spawn a Krabby
	}
	break;

	case ZOMBIE_AE_CRAB2:
	{
		RemoveKrabby();
		SpawnCrab(); // Spawn a Krabby
	}
	break;

	case ZOMBIE_AE_CRAB3:
	{
		RemoveKrabby();
		SpawnCrab(); // Spawn a Krabby
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// Crab Removal
//=========================================================
void CZombie::RemoveKrabby()
{
	m_zombieType = static_cast<zombieTypeEnum>(GetBodygroup(0));
	switch (m_zombieType)
		{
		case zombieTypeEnum::barney:
			SetBodygroup(1, 4); // Remove Krabby
			SetBodygroup(2, 1); // Attach Headdy
			break;
		case zombieTypeEnum::hgrunt:
			SetBodygroup(1, 4); // Remove Krabby
			SetBodygroup(2, 2); // Attach Headdy
			break;
		case zombieTypeEnum::blops:
			SetBodygroup(1, 4); // Remove Krabby
			SetBodygroup(2, 3); // Attach Headdy
			break;
		default:
			SetBodygroup(1, 4); // Remove Krabby
			SetBodygroup(2, 0); // Attach Headdy
			break;
		}
}

//=========================================================
// Spawn
//=========================================================
void CZombie::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/zombie.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	if (pev->body == -1)
	{
		pev->body = RANDOM_LONG(0, TOTALZOMBIETYPES - 1);// pick a random type
	}

	m_zombieType = static_cast<zombieTypeEnum>(pev->body);

	//ALERT(at_notice, "Zombie \"%s\" at (%f, %f, %f) has a body value of \"%d\" \n",
	//	STRING(pev->targetname), pev->origin.x, pev->origin.y, pev->origin.z, m_zombieType);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	if (pev->health == 0)
		switch (m_zombieType)
		{
		case zombieTypeEnum::barney:
			pev->health = gSkillData.zombieHealth * 1.2;
			SetBodygroup(2, 4); // Detach Headdy
			SetBodygroup(1, 1); // Attach Krabby
			break;
		case zombieTypeEnum::hgrunt:
			pev->health = gSkillData.zombieHealth * 1.5;
			SetBodygroup(2, 4); // Detach Headdy
			SetBodygroup(1, 2); // Attach Krabby
			break;
		case zombieTypeEnum::blops:
			pev->health = gSkillData.zombieHealth * 1.5;
			SetBodygroup(2, 4); // Detach Headdy
			SetBodygroup(1, 3); // Attach Krabby
			break;
		default:
			pev->health = gSkillData.zombieHealth;
			SetBodygroup(2, 4); // Detach Headdy
			SetBodygroup(1, 0); // Attach Krabby
			break;
		}
	pev->view_ofs = VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_DOORS_GROUP;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombie::Precache()
{
	UTIL_PrecacheOther(ZOMBIE_CRAB);

	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/zombie.mdl");

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pIdleSoundsGrunt);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pAlertSoundsGrunt);
	PRECACHE_SOUND_ARRAY(pPainSounds);
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================



int CZombie::IgnoreConditions ()
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
		else
#endif			
			if (m_flNextFlinch >= gpGlobals->time)
				iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;

	}

//=========================================================
// TraceAttack - checks to see if the zombie was shot in the head
//=========================================================
void CZombie::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	// Check to see if shot in the head
	if (ptr->iHitgroup == HITGROUP_HEAD)
	{
		m_bloodColor = BLOOD_COLOR_YELLOW; // If shot in the head, emit yellow blood
	}
	else
	{
		m_bloodColor = BLOOD_COLOR_RED; // If not, emit red blood
	}

	CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

//=========================================================
// Spawn Headcrab - headcrab jumps from zombie
//=========================================================
void CZombie::SpawnCrab()
{
	CBaseEntity* pCrab = CBaseEntity::Create(ZOMBIE_CRAB, pev->origin, pev->angles, edict()); // Spawn the crab

	pCrab->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND; // Let the crab fall, when body collapses 
}