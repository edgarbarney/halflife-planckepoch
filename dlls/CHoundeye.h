#ifndef HOUNDEYE_H
#define HOUNDEYE_H


class CHoundeye : public CSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	int  Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void SetYawSpeed() override;
	void WarmUpSound();
	void AlertSound() override;
	void DeathSound() override;
	void WarnSound();
	void PainSound() override;
	void IdleSound() override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void SonicAttack();
	void PrescheduleThink() override;
	void SetActivity(Activity NewActivity) override;
	void WriteBeamColor();
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	BOOL FValidateHintType(short sHint) override;
	BOOL FCanActiveIdle() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;

	int	Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iSpriteTexture;
	BOOL m_fAsleep;// some houndeyes sleep in idle mode if this is set, the houndeye is lying down
	BOOL m_fDontBlink;// don't try to open/close eye if this bit is set!
	Vector	m_vecPackCenter; // the center of the pack. The leader maintains this by averaging the origins of all pack members.
};

#endif // HOUNDEYE_H