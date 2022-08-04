/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012
Spirinity Rendering Engine - Copyright FranticDreamer 2020-2021

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Blood Puddle Entity
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "customentity.h"
#include "effects.h"

TYPEDESCRIPTION CBloodPuddle::m_SaveData[] =
{
	DEFINE_FIELD(CBloodPuddle, m_fBleedStartTime, FIELD_TIME),
	DEFINE_FIELD(CBloodPuddle, m_fStartTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CBloodPuddle, CBaseAnimating);
LINK_ENTITY_TO_CLASS(env_bloodpuddle, CBloodPuddle);

bool CBloodPuddle::KeyValue(KeyValueData* pkvd)
{
	/*
	if (FStrEq(pkvd->szKeyName, "m_iszSequence_On"))
	{
		m_iszSequence_On = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSequence_Off"))
	{
		m_iszSequence_Off = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else
	{
		CBaseAnimating::KeyValue(pkvd);
	}
	*/
	return CBaseAnimating::KeyValue(pkvd);
}

CBloodPuddle* CBloodPuddle::CreatePuddle(Vector vecOrigin, Vector vecAngles, CBaseEntity* pOwner, int bloodColor, float bloodScale)
{
	CBloodPuddle* pPuddle = GetClassPtr((CBloodPuddle*)nullptr);

	UTIL_SetOrigin(pPuddle, vecOrigin);
	pPuddle->pev->angles = vecAngles;
	switch (bloodColor)
	{
	//case BLOOD_COLOR_GREEN:
	case BLOOD_COLOR_YELLOW:
		pPuddle->pev->skin = 1;
		break;
	case BLOOD_COLOR_RED:
	default:
		pPuddle->pev->skin = 0;
		break;
	}
	//pPuddle->SetTouch(&CBloodPuddle::RocketTouch);
	
	pPuddle->pev->scale = bloodScale;

	pPuddle->pev->owner = pOwner->edict();

	pPuddle->Spawn();

	return pPuddle;
}

void CBloodPuddle::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/bloodpuddle.mdl");
	UTIL_SetOrigin(this, pev->origin);

	//	UTIL_AssignOrigin(this, pev->oldorigin); //AJH - WTF is this here for?

	//if (pev->spawnflags & SF_BLOODPUDDLE_SOLID)
	//{
		//pev->solid = SOLID_SLIDEBOX;
		//UTIL_SetSize(pev, Vector(-10, -10, -10), Vector(10, 10, 10));	//LRCT
	//}

	//if (pev->spawnflags & SF_BLOODPUDDLE_DROPTOFLOOR)
	//{
		pev->origin.z += 1;
		DROP_TO_FLOOR(ENT(pev));
	//}

	//m_fStartTime = gpGlobals->time;
	CBaseMonster* pOwner = dynamic_cast<CBaseMonster*>(CBaseEntity::Instance(pev->owner));

	if (pOwner != nullptr)
	{
		m_fStartTime = gpGlobals->time += pOwner->m_fBleedTime;
	}
	else
	{
		m_fStartTime = gpGlobals->time;
	}

	SetBoneController(0, 0);
	SetBoneController(1, 0);

	SetSequence("smallidle", 1);

	SetNextThink(0.1);
}

void CBloodPuddle::Precache()
{
	PRECACHE_MODEL("models/bloodpuddle.mdl");
}

void CBloodPuddle::Think()
{
	//	ALERT(at_console, "env_bloodpuddle Think fr=%f\n", pev->framerate);

	StudioFrameAdvance(); // set m_fSequenceFinished if necessary

	if (gpGlobals->time >= m_fBleedStartTime && pev->sequence == LookupSequence("getbiggur") && m_fSequenceFinished)
		SetSequence("idle", false);

	else if (gpGlobals->time >= m_fBleedStartTime && pev->sequence != LookupSequence("idle"))
	{
		SetSequence("getbiggur", false);
	}
		
		
	
	SetNextThink(0.1);
}

void CBloodPuddle::SetSequence(const char* sequenceName, bool isLoopin)
{
	if (pev->sequence != LookupSequence(sequenceName))
	{
		pev->sequence = LookupSequence(sequenceName);
		pev->frame = 0;
		ResetSequenceInfo();
	}

	if (pev->sequence == -1)
	{
		if (pev->targetname)
			ALERT(at_error, "env_bloodpuddle %s: unknown sequence \"%s\"\n", STRING(pev->targetname), sequenceName);
		else
			ALERT(at_error, "env_bloodpuddle: unknown sequence \"%s\"\n", STRING(pev->targetname), sequenceName);
		pev->sequence = 0;
	}

	m_fSequenceLoops = isLoopin;
}