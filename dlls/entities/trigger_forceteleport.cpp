#include <string>
#include "basetrigger.h"
#include "player.h"
#include "locus.h" //LRC
#include "FranUtils.hpp"

class CTriggerForceTeleport : public CBaseTrigger
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void EXPORT TeleportTouch ( CBaseEntity *pOther );
	
	BOOL m_bOnce; //Only work once?
	BOOL m_bDidOnce; //Did work once?

	BOOL m_bTeleportX; //Will teleporter change position's X axis?
	BOOL m_bTeleportY; //Will teleporter change position's Y axis?
	BOOL m_bTeleportZ; //Will teleporter change position's Z axis?

	BOOL m_bRotateX; //Will teleporter change rotation's X axis?
	BOOL m_bRotateY; //Will teleporter change rotation's Y axis?
	BOOL m_bRotateZ; //Will teleporter change rotation's Z axis?

	BOOL m_bVelocity; //Will teleporter change velocity?

	BOOL m_bForceView; //Will teleporter change view angles?
};

LINK_ENTITY_TO_CLASS( trigger_forceteleport, CTriggerForceTeleport );

void CTriggerForceTeleport :: Spawn()
{
	InitTrigger();

	SetTouch( &CTriggerForceTeleport::TeleportTouch );
}

void CTriggerForceTeleport :: KeyValue( KeyValueData* pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_bTeleportX"))
	{
		m_bTeleportX = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	} 
	else if (FStrEq(pkvd->szKeyName, "m_bTeleportY"))
	{
		m_bTeleportY = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bTeleportZ"))
	{
		m_bTeleportZ = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bRotateX"))
	{
		m_bRotateX = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bRotateY"))
	{
		m_bRotateY = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bRotateZ"))
	{
		m_bRotateZ = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bVelocity"))
	{
		m_bVelocity = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bPlrView"))
	{
		m_bForceView = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bOnce"))
	{
		m_bOnce = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else CBaseEntity::KeyValue(pkvd);
}

void CTriggerForceTeleport :: TeleportTouch( CBaseEntity *pOther )
{
	if (m_bOnce && m_bDidOnce)
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	//if (FBitSet(m_ttTeleportType, TELEPORT_XAXIS))
	entvars_t* pevToucher = pOther->pev;
	CBaseEntity *pTarget = nullptr;

	// Only teleport monsters or clients
	if ( !FBitSet( pevToucher->flags, FL_CLIENT|FL_MONSTER ) )
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	if (!CanTouch(pevToucher))
		return;

	pTarget = UTIL_FindEntityByTargetname( pTarget, STRING(pev->target) );
	if ( !pTarget )
	   return;

	Vector tmp;

	if (m_bTeleportX)
		tmp.x = pTarget->pev->origin.x;
	else
		tmp.x = pOther->pev->origin.x;

	if (m_bTeleportY)
		tmp.y = pTarget->pev->origin.y;
	else
		tmp.y = pOther->pev->origin.y;

	if (m_bTeleportZ)
		tmp.z = pTarget->pev->origin.z;
	else
		tmp.z = pOther->pev->origin.z;

	if ( pOther->IsPlayer() )
	{
		//tmp.z -= pOther->pev->mins.z;// make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
	}
	//tmp.z++;
	//UTIL_SetOrigin( pOther, tmp );
	pOther->pev->origin = tmp;

	Vector tmpRot;

	if (m_bRotateX)
		tmpRot.x = pTarget->pev->angles.x;
	else
		tmpRot.x = pOther->pev->angles.x;

	if (m_bRotateY)
		tmpRot.y = pTarget->pev->angles.y;
	else
		tmpRot.y = pOther->pev->angles.y;

	if (m_bRotateZ)
		tmpRot.z = pTarget->pev->angles.z;
	else
		tmpRot.z = pOther->pev->angles.z;

	pOther->pev->angles = tmpRot;
	
	if (m_bVelocity)
		pOther->pev->velocity = pOther->pev->basevelocity = g_vecZero;

	if (m_bForceView)
	{
		if ( pOther->IsPlayer() )
		{
			pOther->pev->v_angle = tmpRot; //LRC
			pevToucher->fixangle = TRUE;
		}

		pevToucher->fixangle = TRUE;
	}

	pevToucher->flags &= ~FL_ONGROUND;

	FireTargets(STRING(pev->noise), pOther, this, USE_TOGGLE, 0);

	m_bDidOnce = TRUE;
}