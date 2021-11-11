#include "CBullSquad.h"

int	iWaterSquidSpitSprite;

int CWaterSquid::SpitModelInt()
{
	return iWaterSquidSpitSprite;
}

int CWaterSpit::SpitModelInt()
{
	return iWaterSquidSpitSprite;
}



LINK_ENTITY_TO_CLASS(waterspit, CWaterSpit);

LINK_ENTITY_TO_CLASS(monster_WaterChick, CWaterSquid);

void CWaterSquid::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/watersquid.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	pev->health = gSkillData.bullsquidHealth;
	m_flFieldOfView = 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	m_fCanThreatDisplay = TRUE;
	m_flNextSpitTime = gpGlobals->time;

	MonsterInit();
}
void CWaterSquid::Precache()
{
	PRECACHE_MODEL("models/watersquid.mdl");

	PRECACHE_MODEL("sprites/waterbigspit.spr");// spit projectile.

	iWaterSquidSpitSprite = PRECACHE_MODEL("sprites/watertinyspit.spr");// client side spittle.

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	PRECACHE_SOUND("bullchicken/bc_attack2.wav");
	PRECACHE_SOUND("bullchicken/bc_attack3.wav");

	PRECACHE_SOUND("bullchicken/bc_die1.wav");
	PRECACHE_SOUND("bullchicken/bc_die2.wav");
	PRECACHE_SOUND("bullchicken/bc_die3.wav");

	PRECACHE_SOUND("bullchicken/bc_idle1.wav");
	PRECACHE_SOUND("bullchicken/bc_idle2.wav");
	PRECACHE_SOUND("bullchicken/bc_idle3.wav");
	PRECACHE_SOUND("bullchicken/bc_idle4.wav");
	PRECACHE_SOUND("bullchicken/bc_idle5.wav");

	PRECACHE_SOUND("bullchicken/bc_pain1.wav");
	PRECACHE_SOUND("bullchicken/bc_pain2.wav");
	PRECACHE_SOUND("bullchicken/bc_pain3.wav");
	PRECACHE_SOUND("bullchicken/bc_pain4.wav");

	PRECACHE_SOUND("bullchicken/bc_attackgrowl.wav");
	PRECACHE_SOUND("bullchicken/bc_attackgrowl2.wav");
	PRECACHE_SOUND("bullchicken/bc_attackgrowl3.wav");

	PRECACHE_SOUND("bullchicken/bc_acid1.wav");

	PRECACHE_SOUND("bullchicken/bc_bite2.wav");
	PRECACHE_SOUND("bullchicken/bc_bite3.wav");

	PRECACHE_SOUND("bullchicken/bc_spithit1.wav");
	PRECACHE_SOUND("bullchicken/bc_spithit2.wav");

}

void CWaterSquid::ShootSpit(Vector v_offset, Vector v_dir)
{
	CSquidSpit::Shoot(pev, v_offset, v_dir * 900);
}

int CWaterSpit::SquidDecal()
{
	return WATERDECAL_SPIT1;
}
