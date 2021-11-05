#ifndef SQUADMONSTER_PINKSLAVE_H
#define SQUADMONSTER_PINKSLAVE_H

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"talkmonster.h"
#include	"schedule.h"
#include	"effects.h"
#include	"weapons.h"
#include	"soundent.h"

#define		PINKSLAVE_AE_CLAW		( 1 )
#define		PINKSLAVE_AE_CLAWRAKE	( 2 )
#define		PINKSLAVE_AE_ZAP_POWERUP	( 3 )
#define		PINKSLAVE_AE_ZAP_SHOOT		( 4 )
#define		PINKSLAVE_AE_ZAP_DONE		( 5 )

#define		PINKSLAVE_MAX_BEAMS	8

class CPinkSlave : public CTalkMonster
{
public:
	void Spawn() override;
	void DeclineFollowing() override;
	void TalkInit();
	void Precache() override;
	void SetYawSpeed() override;
	int	 ISoundMask() override;
	int  Classify() override;
	int  IRelationship(CBaseEntity* pTarget) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack2(float flDot, float flDist) override;
	void CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, Vector& vecLocation);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void DeathSound() override;
	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	void StartTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	CUSTOM_SCHEDULES;

	int	Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void ClearBeams();
	void ArmBeam(int side);
	void WackBeam(int side, CBaseEntity* pEntity);
	void ZapBeam(int side);
	void BeamGlow();

	int m_iBravery;

	CBeam* m_pBeam[PINKSLAVE_MAX_BEAMS];

	int m_iBeams;
	float m_flNextAttack;

	int	m_voicePitch;

	EHANDLE m_hDead;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
};



#endif
