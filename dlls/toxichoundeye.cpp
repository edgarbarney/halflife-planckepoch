#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"animation.h"
#include	"nodes.h"
#include	"squadmonster.h"
#include	"soundent.h"
#include	"game.h"

#include "CHoundeye.h"


class CToxicHoundeye : public CHoundeye
{
	void Spawn() override;
	void Precache() override;
	int Classify() override;
	
	void MonsterThink() override;

	float toxicDamageDelay = 1.0f; // Delay between damages
	float toxicDamageNextTime = 0.0f; // Next damage time

};
LINK_ENTITY_TO_CLASS(monster_toxichoundeye, CToxicHoundeye);

void CToxicHoundeye::MonsterThink()
{
	if (gpGlobals->time > toxicDamageNextTime)
	{
		RadiusDamage(pev, pev, 20, 250, CLASS_NONE, DMG_ACID, pev->classname, true);
		toxicDamageNextTime = gpGlobals->time + toxicDamageDelay;
	}

	Vector vecSrc = pev->origin;
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSrc);
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecSrc.x);	// X
		WRITE_COORD(vecSrc.y);	// Y
		WRITE_COORD(vecSrc.z);	// Z
		WRITE_BYTE(15);			// radius * 0.1
		WRITE_BYTE(57);		// r
		WRITE_BYTE(212);		// g
		WRITE_BYTE(173);			// b
		WRITE_BYTE(1 / pev->framerate);		// time * 10
		WRITE_BYTE(0);			// decay * 0.1
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_ELIGHT);
		WRITE_SHORT(entindex());     // entity, attachment
		WRITE_COORD(vecSrc.x);     // origin
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_COORD(15);     // radius
		WRITE_BYTE(57);		// r
		WRITE_BYTE(212);		// g
		WRITE_BYTE(173);			// b
		WRITE_BYTE(1 / pev->framerate);     // life * 10
		WRITE_COORD(0); // decay
	MESSAGE_END();

	CBaseMonster::MonsterThink();
}

void CToxicHoundeye::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/toxichoundeye.mdl");
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 36));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_YELLOW;
	pev->effects = 0;
	pev->health = gSkillData.houndeyeHealth;
	pev->yaw_speed = 5;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_fAsleep = FALSE; // everyone spawns awake
	m_fDontBlink = FALSE;
	m_afCapability |= bits_CAP_SQUAD;

	MonsterInit();
}

void CToxicHoundeye::Precache()
{
	PRECACHE_MODEL("models/toxichoundeye.mdl");

	PRECACHE_SOUND("houndeye/he_alert1.wav");
	PRECACHE_SOUND("houndeye/he_alert2.wav");
	PRECACHE_SOUND("houndeye/he_alert3.wav");

	PRECACHE_SOUND("houndeye/he_die1.wav");
	PRECACHE_SOUND("houndeye/he_die2.wav");
	PRECACHE_SOUND("houndeye/he_die3.wav");

	PRECACHE_SOUND("houndeye/he_idle1.wav");
	PRECACHE_SOUND("houndeye/he_idle2.wav");
	PRECACHE_SOUND("houndeye/he_idle3.wav");

	PRECACHE_SOUND("houndeye/he_hunt1.wav");
	PRECACHE_SOUND("houndeye/he_hunt2.wav");
	PRECACHE_SOUND("houndeye/he_hunt3.wav");

	PRECACHE_SOUND("houndeye/he_pain1.wav");
	PRECACHE_SOUND("houndeye/he_pain3.wav");
	PRECACHE_SOUND("houndeye/he_pain4.wav");
	PRECACHE_SOUND("houndeye/he_pain5.wav");

	PRECACHE_SOUND("houndeye/he_attack1.wav");
	PRECACHE_SOUND("houndeye/he_attack3.wav");

	PRECACHE_SOUND("houndeye/he_blast1.wav");
	PRECACHE_SOUND("houndeye/he_blast2.wav");
	PRECACHE_SOUND("houndeye/he_blast3.wav");

	m_iSpriteTexture = PRECACHE_MODEL("sprites/shockwave.spr");
}
int	CToxicHoundeye::Classify()
{
	return CLASS_PLAYER_ALLY;
}
