#pragma once

#include	"schedule.h"

#define GRUNTS_H

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_GRUNT_SUPPRESS = LAST_COMMON_SCHEDULE + 1,
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE,// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_GRUNT_COVER_AND_RELOAD,
	SCHED_GRUNT_SWEEP,
	SCHED_GRUNT_FOUND_ENEMY,
	SCHED_GRUNT_REPEL,
	SCHED_GRUNT_REPEL_ATTACK,
	SCHED_GRUNT_REPEL_LAND,
	SCHED_GRUNT_WAIT_FACE_ENEMY,
	SCHED_GRUNT_TAKECOVER_FAILED,// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_GRUNT_ELOF_FAIL,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_GRUNT_FACE_TOSS_DIR = LAST_COMMON_TASK + 1,
	TASK_GRUNT_SPEAK_SENTENCE,
	TASK_GRUNT_CHECK_FIRE,
};

//=========================================================
// monster-specific conditions
//=========================================================
#define bits_COND_GRUNT_NOFIRE	( bits_COND_SPECIAL1 )

class CSas : public CSquadMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int  Classify(void);
	int ISoundMask(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	BOOL FCanCheckAttacks(void);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack2(float flDot, float flDist);
	void CheckAmmo(void);
	void SetActivity(Activity NewActivity);
	void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);
	void DeathSound(void);
	void PainSound(void);
	void IdleSound(void);
	Vector GetGunPosition(void);
	void Shoot(void);
	void Shotgun(void);
	void PrescheduleThink(void);
	void GibMonster(void);
	void SpeakSentence(void);

	int	Save(CSave& save);
	int Restore(CRestore& restore);

	CBaseEntity* Kick(void);
	Schedule_t* GetSchedule(void);
	Schedule_t* GetScheduleOfType(int Type);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	int IRelationship(CBaseEntity* pTarget);

	BOOL FOkToSpeak(void);
	void JustSpoke(void);

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector	m_vecTossVelocity;

	BOOL	m_fThrowGrenade;
	BOOL	m_fStanding;
	BOOL	m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	int		m_cClipSize;
	BOOL	m_isFriendly;

	int m_voicePitch;

	int		m_iBrassShell;
	int		m_iShotgunShell;

	int		m_iSentence;

	static const char* pGruntSentences[];
};

class CHGrunt : public CSquadMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int  Classify(void);
	int ISoundMask(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	BOOL FCanCheckAttacks(void);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack2(float flDot, float flDist);
	void CheckAmmo(void);
	void SetActivity(Activity NewActivity);
	void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);
	void DeathSound(void);
	void PainSound(void);
	void IdleSound(void);
	Vector GetGunPosition(void);
	void Shoot(void);
	void Shotgun(void);
	void PrescheduleThink(void);
	void GibMonster(void);
	void SpeakSentence(void);

	int	Save(CSave& save);
	int Restore(CRestore& restore);

	CBaseEntity* Kick(void);
	Schedule_t* GetSchedule(void);
	Schedule_t* GetScheduleOfType(int Type);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	int IRelationship(CBaseEntity* pTarget);

	BOOL FOkToSpeak(void);
	void JustSpoke(void);

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector	m_vecTossVelocity;

	BOOL	m_fThrowGrenade;
	BOOL	m_fStanding;
	BOOL	m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	int		m_cClipSize;

	int m_voicePitch;

	int		m_iBrassShell;
	int		m_iShotgunShell;

	int		m_iSentence;

	static const char* pGruntSentences[];
};