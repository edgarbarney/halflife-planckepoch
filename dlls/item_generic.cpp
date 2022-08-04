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
#include "extdll.h"
#include "util.h"
#include "cbase.h"

const auto SF_ITEMGENERIC_DROP_TO_FLOOR = 1 << 0;


class CItemGeneric : public CBaseAnimating
{
public:
	void	Spawn() override;
	void	Precache() override;
	bool	KeyValue(KeyValueData* pkvd) override;

	int		ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	bool m_fDisableShadows;
	bool m_fDisableDrawing;
};

LINK_ENTITY_TO_CLASS(item_generic, CItemGeneric);
LINK_ENTITY_TO_CLASS(item_prop, CItemGeneric);

TYPEDESCRIPTION	CItemGeneric::m_SaveData[] =
{
	DEFINE_FIELD(CItemGeneric, m_fDisableShadows, FIELD_BOOLEAN),
	DEFINE_FIELD(CItemGeneric, m_fDisableDrawing, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CItemGeneric, CBaseAnimating);

bool CItemGeneric::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "DisableShadows"))
	{
		m_fDisableShadows = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "DisableDrawing"))
	{
		m_fDisableDrawing = atoi(pkvd->szValue);
		return true;
	}
	else
		return CBaseAnimating::KeyValue(pkvd);
}

void CItemGeneric::Precache()
{
	PRECACHE_MODEL((char*)STRING(pev->model));
}

void CItemGeneric::Spawn()
{
	if (pev->targetname)
	{
		Precache();
		SET_MODEL(ENT(pev), STRING(pev->model));
	}
	else
	{
		UTIL_Remove(this);
	}

	pev->movetype = MOVETYPE_NONE;

	if (m_fDisableShadows == true)
		pev->effects |= FL_NOSHADOW;

	if (m_fDisableDrawing == true)
		pev->effects |= FL_NOMODEL;

	pev->framerate = 1.0;
}
