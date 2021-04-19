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
	void	KeyValue(KeyValueData* pkvd) override;

	int		ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	int		Save(CSave& save) override;
	int		Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL m_fDisableShadows;
	BOOL m_fDisableDrawing;
};

LINK_ENTITY_TO_CLASS(item_generic, CItemGeneric);
LINK_ENTITY_TO_CLASS(item_prop, CItemGeneric);

TYPEDESCRIPTION	CItemGeneric::m_SaveData[] =
{
	DEFINE_FIELD(CItemGeneric, m_fDisableShadows, FIELD_BOOLEAN),
	DEFINE_FIELD(CItemGeneric, m_fDisableDrawing, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CItemGeneric, CBaseAnimating);

void CItemGeneric::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "DisableShadows"))
	{
		m_fDisableShadows = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "DisableDrawing"))
	{
		m_fDisableDrawing = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseAnimating::KeyValue(pkvd);
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

	if (m_fDisableShadows == TRUE)
		pev->effects |= FL_NOSHADOW;

	if (m_fDisableDrawing == TRUE)
		pev->effects |= FL_NOMODEL;

	pev->framerate = 1.0;
}
