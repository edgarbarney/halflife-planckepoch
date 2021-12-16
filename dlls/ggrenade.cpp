/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== generic grenade.cpp ========================================================

*/

#include <cmath>
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "shake.h"
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"

#define NadeVectorSubtract(a,b,c) {(c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];(c)[2]=(a)[2]-(b)[2];}

float Distance(const float* v1, const float* v2);

enum nadedir
{
	NADEDIR_WTF = -1,
	NADEDIR_NOPE = 0,
	NADEDIR_FRONT = 1,
	NADEDIR_RIGHT,
	NADEDIR_REAR,
	NADEDIR_LEFT,
};

enum nadetype
{
	NADETYPE_TIMED = 0,
	NADETYPE_CONTACT,
	NADETYPE_SATCHEL,
	NADETYPE_STUN,
};

LINK_ENTITY_TO_CLASS( grenade, CGrenade );

// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
#define SF_DETONATE		0x0001

void CGrenade::ShootShrapnel()
{
	/*
	int i;
	TraceResult tr;

	Vector forward = { gpGlobals->v_forward.x, gpGlobals->v_forward.y * m_iLineCountNextRot, gpGlobals->v_forward.z};

	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -32), ignore_monsters, ENT(pev), &tr);

	// do damage, paint decals
	if (tr.flFraction != 1.0)
	{

	}
	*/
}

//
// Grenade Explode
//
void CGrenade::Explode( Vector vecSrc, Vector vecAim )
{
	TraceResult tr;
	UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ),  ignore_monsters, ENT(pev), & tr);

	if (m_nadeType != NADETYPE_STUN)
		Explode(&tr, DMG_BLAST);
	else
		StunExplode(&tr, DMG_SONIC);
}

// UNDONE: temporary scorching for PreAlpha - find a less sleazy permenant solution.
void CGrenade::Explode( TraceResult *pTrace, int bitsDamageType )
{
	float		flRndSound;// sound randomizer

	CBasePlayer* thePlayerPtr = nullptr;

	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible

	pev->takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->flFraction != 1.0 )
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
	}

	int iContents = UTIL_PointContents ( pev->origin );
	
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		if (iContents != CONTENTS_WATER)
		{
			WRITE_SHORT( g_sModelIndexFireball );
		}
		else
		{
			WRITE_SHORT( g_sModelIndexWExplosion );
		}
		WRITE_BYTE( (pev->dmg - 50) * .60  ); // scale * 10
		WRITE_BYTE( 15  ); // framerate
		WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();

	//RENDERERS START
	if(iContents != CONTENTS_WATER)
		UTIL_Particle("explosion_cluster.txt", pev->origin, g_vecZero, 1);
	//RENDERERS END
	
	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );
	
	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = nullptr;

	pev->owner = nullptr; // can't traceline attack owner if this is set

	if (!thePlayerPtr) 
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
			if (!pPlayer) // Failed to retrieve a player at this index, skip and move on to the next one
				continue;

			thePlayerPtr = pPlayer;
		}
	}

	if (thePlayerPtr)
	{
		float plrHeathOld = plrHeathOld = thePlayerPtr->pev->health;
		RadiusDamage ( pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType );
		float plrHeathDif = plrHeathDif = (plrHeathOld - thePlayerPtr->pev->health) * 8.0f;
		//UTIL_ScreenShake(pev->origin, plrHeathDif, 255.0f, plrHeathDif / 80.0f, plrHeathDif * 8.0f);
		//												  (plrHeathDif / 8) / 10
		if (Distance(thePlayerPtr->pev->origin, pev->origin) < 250.0f)
		{
			float volu = (150 - Distance(thePlayerPtr->pev->origin, pev->origin)) / 125; if (volu < 0) volu = 0;
			switch (CalcDamageDirection(pev->origin, thePlayerPtr))
			{
			case NADEDIR_FRONT:
				EMIT_SOUND_DYN(ENT(thePlayerPtr->pev), CHAN_AUTO, "player/earringing.wav", volu, ATTN_NORM, 0, 100);
				break;
			case NADEDIR_RIGHT:
				EMIT_SOUND_DYN(ENT(thePlayerPtr->pev), CHAN_AUTO, "player/earringing_right.wav", volu, ATTN_NORM, 0, 100);
				break;
			case NADEDIR_REAR:
				EMIT_SOUND_DYN(ENT(thePlayerPtr->pev), CHAN_AUTO, "player/earringing.wav", volu, ATTN_NORM, 0, 100);
				break;
			case NADEDIR_LEFT:
				EMIT_SOUND_DYN(ENT(thePlayerPtr->pev), CHAN_AUTO, "player/earringing_left.wav", volu, ATTN_NORM, 0, 100);
				break;
			case NADEDIR_WTF:
			case NADEDIR_NOPE:
			default:
				break;
			}
		}
	}
	else
	{
		RadiusDamage(pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType);
	}

	if (m_bSimulateShrapnel)
		ShootShrapnel();

	/*
	if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );
	}
	else
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH2 );
	}
	*/

//RENDERERS START
	UTIL_CustomDecal(pTrace, "expscorch");
//RENDERERS END

	flRndSound = RANDOM_FLOAT( 0 , 1 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);	break;
	}

	pev->effects |= EF_NODRAW;
	SetThink( &CGrenade::Smoke );
	pev->velocity = g_vecZero;
	SetNextThink( 0.3 );

	if (iContents != CONTENTS_WATER)
	{
		int sparkCount = RANDOM_LONG(0,3);
		for ( int i = 0; i < sparkCount; i++ )
			Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, nullptr );
	}
}

void CGrenade::StunExplode(TraceResult* pTrace, int bitsDamageType)
{
	float		flRndSound;// sound randomizer

	CBasePlayer* thePlayerPtr = nullptr;

	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible

	pev->takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if (pTrace->flFraction != 1.0)
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 1.2);
	}

	int iContents = UTIL_PointContents(pev->origin);

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgCreateDLight);
		//WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(pev->origin.x); // origin
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_BYTE(500);     // radius
		WRITE_BYTE(255);     // R
		WRITE_BYTE(255);     // G
		WRITE_BYTE(255);     // B
		WRITE_BYTE(0.1f);     // life * 10
		WRITE_BYTE(0); // decay
	MESSAGE_END();

	//RENDERERS START
	if (iContents != CONTENTS_WATER)
		UTIL_Particle("explosion_stun.txt", Vector(pev->origin.x, pev->origin.y, pev->origin.z - 50.0f), g_vecZero, 1);
	//RENDERERS END

	//if (m_nadeType != NADETYPE_STUN)
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0);

	entvars_t* pevOwner;
	if (pev->owner)
		pevOwner = VARS(pev->owner);
	else
		pevOwner = nullptr;

	pev->owner = nullptr; // can't traceline attack owner if this is set

	CBaseEntity* entPointa = nullptr;

	while ((entPointa = UTIL_FindEntityInSphere(entPointa, pev->origin, 1500)) != nullptr)
	{
		CBaseMonster* monstaPointa = entPointa->MyMonsterPointer();
		if (monstaPointa != nullptr && monstaPointa != this)
		{
			TraceResult tr2;
			UTIL_TraceLine(pev->origin, monstaPointa->pev->origin, ignore_monsters, ENT(pev), &tr2 );
			if (tr2.flFraction != 1.0)
			{
				pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
				// Something's wrong. Couldn't find monster. Try again for the last time.
				UTIL_TraceLine(pev->origin, monstaPointa->pev->origin, ignore_monsters, ENT(pev), &tr2);
			}
			//ALERT(at_console, "\nMonster %s at %f,%f,%f is now BEING paralized!\n", STRING(monstaPointa->pev->classname), monstaPointa->pev->origin.x, monstaPointa->pev->origin.y, monstaPointa->pev->origin.z);
			if (tr2.flFraction == 1)
				monstaPointa->BeStunned(4.0f);
		}
	}

	if (!thePlayerPtr)
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
			if (!pPlayer) // Failed to retrieve a player at this index, skip and move on to the next one
				continue;

			thePlayerPtr = pPlayer;
		}
	}

	if (thePlayerPtr)
	{
		TraceResult tr3;
		UTIL_TraceLine(pev->origin, thePlayerPtr->pev->origin, ignore_monsters, ENT(pev), &tr3);
		/*
		if (tr3.flFraction != 1.0)
		{
			pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
			UTIL_TraceLine(pev->origin, thePlayerPtr->pev->origin, ignore_monsters, ENT(pev), &tr3);
		}
		*/

		//ALERT(at_console, "\nMonster %s at %f,%f,%f is now BEING paralized!\n", STRING(monstaPointa->pev->classname), monstaPointa->pev->origin.x, monstaPointa->pev->origin.y, monstaPointa->pev->origin.z);
		if (tr3.flFraction == 1) 
		{
			float plrDist = Distance(thePlayerPtr->pev->origin, pev->origin);
			if (Distance(thePlayerPtr->pev->origin, pev->origin) < 1000.0f)
			{
				float volu = (1000.0f - plrDist) / 1500.0f; if (volu < 0) volu = 0;
				switch (CalcDamageDirection(pev->origin, thePlayerPtr))
				{
				case NADEDIR_FRONT:
					UTIL_ScreenFade(thePlayerPtr, Vector(255, 225, 255), (1500.0f - plrDist) / 1000.0f, (1500.0f - plrDist) / 300.0f, 255, FFADE_IN);
					EMIT_SOUND_DYN(ENT(thePlayerPtr->pev), CHAN_AUTO, "player/earringing.wav", volu, ATTN_NORM, 0, 100);
					break;
				case NADEDIR_RIGHT:
					UTIL_ScreenFade(thePlayerPtr, Vector(255, 225, 255), (1500.0f - plrDist) / 1000.0f, (1500.0f - plrDist) / 500.0f, 150, FFADE_IN);
					EMIT_SOUND_DYN(ENT(thePlayerPtr->pev), CHAN_AUTO, "player/earringing_right.wav", volu, ATTN_NORM, 0, 100);
					break;
				case NADEDIR_REAR:
					UTIL_ScreenFade(thePlayerPtr, Vector(255, 225, 255), (1500.0f - plrDist) / 1000.0f, (1500.0f - plrDist) / 1000.0f, 100, FFADE_IN);
					EMIT_SOUND_DYN(ENT(thePlayerPtr->pev), CHAN_AUTO, "player/earringing.wav", volu, ATTN_NORM, 0, 100);
					break;
				case NADEDIR_LEFT:
					UTIL_ScreenFade(thePlayerPtr, Vector(255, 225, 255), (1500.0f - plrDist) / 1000.0f, (1500.0f - plrDist) / 500.0f, 150, FFADE_IN);
					EMIT_SOUND_DYN(ENT(thePlayerPtr->pev), CHAN_AUTO, "player/earringing_left.wav", volu, ATTN_NORM, 0, 100);
					break;
				case NADEDIR_WTF:
				case NADEDIR_NOPE:
				default:
					break;
				}
			}
		}
	}

	if (RANDOM_FLOAT(0, 1) < 0.5)
	{
		UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
	}
	else
	{
		UTIL_DecalTrace(pTrace, DECAL_SCORCH2);
	}

	flRndSound = RANDOM_FLOAT(0, 1);

	/*
	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);	break;
	}
	*/
	switch (RANDOM_LONG(0, 1))
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/fbang_explode1.wav", 1.5f, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/fbang_explode2.wav", 1.5f, ATTN_NORM);	break;
	}
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);

	pev->effects |= EF_NODRAW;
	SetThink(&CGrenade::Smoke);
	pev->velocity = g_vecZero;
	SetNextThink(0.3);

	if (iContents != CONTENTS_WATER)
	{
		int sparkCount = RANDOM_LONG(0, 3);
		for (int i = 0; i < sparkCount; i++)
			Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, nullptr);
	}
}

int CGrenade::CalcDamageDirection(Vector vecFrom, CBasePlayer* playaPtr)
{
	Vector	forward, right, up;
	float	side, front;
	Vector vecOrigin, vecAngles;

	if (!vecFrom[0] && !vecFrom[1] && !vecFrom[2])
	{
		m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;
		return -1;
	}


	memcpy(vecOrigin, playaPtr->pev->origin, sizeof(Vector));
	memcpy(vecAngles, playaPtr->pev->angles, sizeof(Vector));


	NadeVectorSubtract(vecFrom, vecOrigin, vecFrom);

	float flDistToTarget = vecFrom.Length();

	vecFrom = vecFrom.Normalize();
	g_engfuncs.pfnAngleVectors(vecAngles, forward, right, up);

	front = DotProduct(vecFrom, right);
	side = DotProduct(vecFrom, forward);

	if (flDistToTarget <= 50)
	{
		m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 1;
	}
	else
	{
		if (side > 0)
		{
			if (side > 0.3)
				m_fAttackFront = V_max(m_fAttackFront, side);
		}
		else
		{
			float f = fabs(side);
			if (f > 0.3)
				m_fAttackRear = V_max(m_fAttackRear, f);
		}

		if (front > 0)
		{
			if (front > 0.3)
				m_fAttackRight = V_max(m_fAttackRight, front);
		}
		else
		{
			float f = fabs(front);
			if (f > 0.3)
				m_fAttackLeft = V_max(m_fAttackLeft, f);
		}
	}
	return EmitDir();
}

int CGrenade::EmitDir()
{
	if (!(m_fAttackFront || m_fAttackRear || m_fAttackLeft || m_fAttackRight))
		return 0;

	if	(m_fAttackFront > 0.4)	return NADEDIR_FRONT;
	else m_fAttackFront = 0;

	if	(m_fAttackRight > 0.4)	return NADEDIR_RIGHT;
	else m_fAttackRight = 0;

	if	(m_fAttackRear > 0.4)	return NADEDIR_REAR;
	else m_fAttackRear = 0;

	if	(m_fAttackLeft > 0.4)	return NADEDIR_LEFT;
	else m_fAttackLeft = 0;

	return 0;
}

void CGrenade::Smoke()
{
	if (UTIL_PointContents ( pev->origin ) == CONTENTS_WATER)
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else if (m_nadeType != NADETYPE_STUN)
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( (pev->dmg - 50) * 0.80 ); // scale * 10
			WRITE_BYTE( 12  ); // framerate
		MESSAGE_END();
	}
	UTIL_Remove( this );
}


void CGrenade::Killed( entvars_t *pevAttacker, int iGib )
{
	Detonate( );
}


// Timed grenade, this think is called when time runs out.
void CGrenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CGrenade::Detonate );
	SetNextThink( 0 );
}

void CGrenade::PreDetonate()
{
	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, 400, 0.3 );

	SetThink( &CGrenade::Detonate );
	SetNextThink( 1 );
}


void CGrenade::Detonate()
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);

	if (m_nadeType != NADETYPE_STUN)
		Explode(&tr, DMG_BLAST);
	else
		StunExplode(&tr, DMG_SONIC);
}


//
// Contact grenade, explode when it touches something
// 
void CGrenade::ExplodeTouch( CBaseEntity *pOther )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	pev->enemy = pOther->edict();

	vecSpot = pev->origin - pev->velocity.Normalize() * 32;
	UTIL_TraceLine( vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr );

	Explode( &tr, DMG_BLAST );
}


void CGrenade::DangerSoundThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	//if (m_nadeType != NADETYPE_STUN)
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length( ), 0.2 );
	SetNextThink( 0.2 );

	if (pev->waterlevel != 0 && pev->watertype > CONTENT_FLYFIELD)
	{
		pev->velocity = pev->velocity * 0.5;
	}
}


void CGrenade::BounceTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// only do damage if we're moving fairly fast
	if (m_flNextAttack < gpGlobals->time && pev->velocity.Length() > 100)
	{
		entvars_t *pevOwner = VARS( pev->owner );
		if (pevOwner)
		{
			TraceResult tr = UTIL_GetGlobalTrace( );
			ClearMultiDamage( );
			pOther->TraceAttack(pevOwner, 1, gpGlobals->v_forward, &tr, DMG_CLUB ); 
			ApplyMultiDamage( pev, pevOwner);
		}
		m_flNextAttack = gpGlobals->time + 1.0; // debounce
	}

	Vector vecTestVelocity;
	// pev->avelocity = Vector (300, 300, 300);

	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity. 
	// trimming the Z velocity a bit seems to help quite a bit.
	vecTestVelocity = pev->velocity; 
	vecTestVelocity.z *= 0.45;

	if ( !m_fRegisteredSound && vecTestVelocity.Length() <= 60 )
	{
		//ALERT( at_console, "Grenade Registered!: %f\n", vecTestVelocity.Length() );

		// grenade is moving really slow. It's probably very close to where it will ultimately stop moving. 
		// go ahead and emit the danger sound.
		
		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		//if (m_nadeType != NADETYPE_STUN)
			CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4, 0.3 );
		m_fRegisteredSound = TRUE;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// Static friction
		pev->velocity = pev->velocity * 0.8;

		pev->sequence = RANDOM_LONG( 1, 1 );
	}
	else
	{
		// play bounce sound
		BounceSound();
	}
	pev->framerate = pev->velocity.Length() / 200.0;
	if (pev->framerate > 1.0)
		pev->framerate = 1;
	else if (pev->framerate < 0.5)
		pev->framerate = 0;

}



void CGrenade::SlideTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// pev->avelocity = Vector (300, 300, 300);

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;

		if (pev->velocity.x != 0 || pev->velocity.y != 0)
		{
			// maintain sliding sound
		}
	}
	else
	{
		BounceSound();
	}
}

void CGrenade :: BounceSound()
{	
	switch (m_nadeType)
	{
	case NADETYPE_STUN:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/fbang_hit1.wav", 0.25, ATTN_NORM);	
		break;
	case NADETYPE_CONTACT:
	case NADETYPE_SATCHEL:
	case NADETYPE_TIMED:
	default:
		switch (RANDOM_LONG(0, 2))
		{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.25, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.25, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.25, ATTN_NORM);	break;
		}
		break;
	}
	
}

void CGrenade :: TumbleThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance( );
	SetNextThink( 0.1 );

	if (pev->dmgtime - 1 < gpGlobals->time && m_nadeType != NADETYPE_STUN)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( &CGrenade::Detonate );
	}
	if (pev->waterlevel != 0 && pev->watertype > CONTENT_FLYFIELD)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}
}


void CGrenade:: Spawn()
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->classname = MAKE_STRING( "grenade" );
	
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/grenade.mdl");
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	pev->dmg = 100;
	m_fRegisteredSound = FALSE;
}


CGrenade *CGrenade::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)nullptr );
	pGrenade->Spawn();
	// contact grenades arc lower
	pGrenade->pev->gravity = 0.5;// lower gravity since grenade is aerodynamic and engine doesn't know it.
	UTIL_SetOrigin( pGrenade, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles (pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	pGrenade->m_nadeType = NADETYPE_CONTACT;
	
	// make monsters afaid of it while in the air
	pGrenade->SetThink( &CGrenade::DangerSoundThink );
	pGrenade->SetNextThink( 0 );
	
	// Tumble in air
	//pGrenade->pev->avelocity.x = RANDOM_FLOAT ( -100, -500 );
	
	// Explode on contact
	pGrenade->SetTouch( &CGrenade::ExplodeTouch );

	pGrenade->pev->dmg = gSkillData.plrDmgM203Grenade;

	return pGrenade;
}


CGrenade * CGrenade:: ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)nullptr );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade, vecStart );
	pGrenade->pev->velocity = vecVelocity * 1.5;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	pGrenade->m_nadeType = NADETYPE_TIMED;
	
	pGrenade->SetTouch( &CGrenade::BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( &CGrenade::TumbleThink );
	pGrenade->SetNextThink( 0.1 );
	if (time < 0.1)
	{
		pGrenade->SetNextThink( 0 );
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 1;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/W_grenade_thrown.mdl");
	pGrenade->pev->dmg = 100;

	return pGrenade;
}


CGrenade* CGrenade::ShootStun(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float time)
{
	CGrenade* pGrenade = GetClassPtr((CGrenade*)nullptr);
	pGrenade->Spawn();
	UTIL_SetOrigin(pGrenade, vecStart);
	pGrenade->pev->velocity = vecVelocity * 1.5;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	pGrenade->m_nadeType = NADETYPE_STUN;

	pGrenade->SetTouch(&CGrenade::BounceTouch);	// Bounce if touched

	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink(&CGrenade::TumbleThink);
	pGrenade->SetNextThink(0.1);
	if (time < 0.1)
	{
		pGrenade->SetNextThink(0);
		pGrenade->pev->velocity = Vector(0, 0, 0);
	}

	pGrenade->pev->sequence = RANDOM_LONG(3, 6);
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 1;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/w_stungrenade_thrown.mdl");
	pGrenade->pev->dmg = 100;

	return pGrenade;
}


CGrenade * CGrenade :: ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)nullptr );
	pGrenade->pev->movetype = MOVETYPE_BOUNCE;
	pGrenade->pev->classname = MAKE_STRING( "grenade" );
	
	pGrenade->pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pGrenade->pev), "models/grenade.mdl");	// Change this to satchel charge model

	UTIL_SetSize(pGrenade->pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	pGrenade->pev->dmg = 200;
	UTIL_SetOrigin( pGrenade, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = g_vecZero;
	pGrenade->pev->owner = ENT(pevOwner);
	pGrenade->m_nadeType = NADETYPE_SATCHEL;
	
	// Detonate in "time" seconds
	pGrenade->SetThink( &CGrenade::SUB_DoNothing );
	pGrenade->SetUse( &CGrenade::DetonateUse );
	pGrenade->SetTouch( &CGrenade::SlideTouch );
	pGrenade->pev->spawnflags = SF_DETONATE;

	pGrenade->pev->friction = 0.9;

	return pGrenade;
}



void CGrenade :: UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code )
{
	edict_t *pentOwner;

	if ( !pevOwner )
		return;

	CBaseEntity	*pOwner = CBaseEntity::Instance( pevOwner );

	pentOwner = pOwner->edict();

	CBaseEntity *pEnt = UTIL_FindEntityByClassname( nullptr, "grenade" );
	while ( pEnt )
	{
		if ( FBitSet( pEnt->pev->spawnflags, SF_DETONATE ) && pEnt->pev->owner == pentOwner )
		{
			if ( code == SATCHEL_DETONATE )
				pEnt->Use( pOwner, pOwner, USE_ON, 0 );
			else	// SATCHEL_RELEASE
				pEnt->pev->owner = nullptr;
		}
		pEnt = UTIL_FindEntityByClassname( pEnt, "grenade" );
	}
}
