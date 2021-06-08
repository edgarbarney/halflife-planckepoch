#include "basetrigger.h"
#include "player.h"
#include "locus.h" //LRC

#define TELEPORT_XAXIS		( 1 << 0)
#define TELEPORT_YAXIS		( 1 << 1)
#define TELEPORT_ZAXIS		( 1 << 2)

//enum class teleportTypeEnum { xyz, xAxis, yAxis, zAxis };

class CTriggerForceTeleport : public CBaseTrigger
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void EXPORT TeleportTouch ( CBaseEntity *pOther );

	BOOL m_bTeleportX; //Will teleporter change X axis?
	BOOL m_bTeleportY; //Will teleporter change Y axis?
	BOOL m_bTeleportZ; //Will teleporter change Z axis?
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
	else CBaseEntity::KeyValue(pkvd);
}

void CTriggerForceTeleport :: TeleportTouch( CBaseEntity *pOther )
{
	
}