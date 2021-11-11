#ifndef BULLSQUAD_H
#define BULLSQUAD_H

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"nodes.h"
#include	"effects.h"
#include	"decals.h"
#include	"soundent.h"
#include	"game.h"

class CBullsquid : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  ISoundMask() override;
	int  Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void IdleSound() override;
	void PainSound() override;
	void DeathSound() override;
	void AlertSound() override;
	void AttackSound();
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	BOOL CheckMeleeAttack1(float flDot, float flDist) override;
	BOOL CheckMeleeAttack2(float flDot, float flDist) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	void RunAI() override;
	BOOL FValidateHintType(short sHint) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int IRelationship(CBaseEntity* pTarget) override;
	int IgnoreConditions() override;
	MONSTERSTATE GetIdealState() override;

	virtual void ShootSpit(Vector v_offset, Vector v_dir);
	virtual int SpitModelInt();
	
	int	Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_fCanThreatDisplay;// this is so the squid only does the "I see a headcrab!" dance one time. 

	float m_flLastHurtTime;// we keep track of this, because if something hurts a squid, it will forget about its love of headcrabs for a while.
	float m_flNextSpitTime;// last time the bullsquid used the spit attack.

};

class CWaterSquid : public CBullsquid
{
	void Spawn() override;
	void Precache() override;
	int SpitModelInt() override;
	void ShootSpit(Vector v_offset, Vector v_dir) override;
};

//=========================================================
// Bullsquid's spit projectile
//=========================================================
class CSquidSpit : public CBaseEntity
{
public:
	void Spawn() override;

	static void Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity* pOther) override;
	void EXPORT Animate();

	int		Save(CSave& save) override;
	int		Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
	
	virtual int SpitModelInt();
	virtual int SquidDecal();

	int  m_maxFrame;

};

class CWaterSpit : public CSquidSpit
{
	int SpitModelInt() override;
	int SquidDecal();
};
 
#endif
