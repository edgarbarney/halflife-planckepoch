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
//=========================================================
// NyanCats
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"nyancat.h"
#include	"gamerules.h"


int iNyanCatTrail;
int iNyanCatPuff;

LINK_ENTITY_TO_CLASS( nyancat, CNyanCat );

//=========================================================
// Save/Restore
//=========================================================
TYPEDESCRIPTION	CNyanCat::m_SaveData[] = 
{
	DEFINE_FIELD( CNyanCat, m_flStopAttack, FIELD_TIME ),
	DEFINE_FIELD( CNyanCat, m_flFlySpeed, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CNyanCat, CBaseMonster );

//=========================================================
// don't let nyancats gib, ever.
//=========================================================
int CNyanCat :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// filter these bits a little.
	bitsDamageType &= ~ ( DMG_ALWAYSGIB );
	bitsDamageType |= DMG_NEVERGIB;

	return CBaseMonster :: TakeDamage ( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
//=========================================================
void CNyanCat :: Spawn()
{
	Precache();

	pev->movetype	= MOVETYPE_FLY;
	pev->solid		= SOLID_BBOX;
	pev->takedamage = DAMAGE_YES;
	pev->flags		|= FL_MONSTER;
	pev->health		= 1;// weak!
	
	if ( g_pGameRules->IsMultiplayer() )
	{
		// nyancats don't live as long in multiplayer
		m_flStopAttack = gpGlobals->time + 3.5;
	}
	else
	{
		m_flStopAttack	= gpGlobals->time + 5.0;
	}

	m_flFieldOfView = 0.9; // +- 25 degrees


	m_flFlySpeed = 300;

	SET_MODEL(ENT( pev ), "models/nyancat.mdl");
	UTIL_SetSize( pev, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );

	SetTouch( &CNyanCat::DieTouch );
	SetThink( &CNyanCat::StartTrack );

	edict_t *pSoundEnt = pev->owner;
	if ( !pSoundEnt )
		pSoundEnt = edict();

	if ( !FNullEnt(pev->owner) && (pev->owner->v.flags & FL_CLIENT) )
	{
		pev->dmg = gSkillData.plrDmgNyanCat;
	}
	else
	{
		// no real owner, or owner isn't a client. 
		pev->dmg = gSkillData.monDmgNyanCat;
	}
	
	pev->nextthink = gpGlobals->time + 0.1;
	ResetSequenceInfo( );
}


void CNyanCat :: Precache()
{
	PRECACHE_MODEL("models/nyancat.mdl");

	PRECACHE_SOUND( "nyancat/cathit.wav" );

	iNyanCatPuff = PRECACHE_MODEL( "sprites/muz1.spr" );
	iNyanCatTrail = PRECACHE_MODEL("sprites/laserbeam.spr");
}	

//=========================================================
// nyancats will never get mad at each other, no matter who the owner is.
//=========================================================
int CNyanCat::IRelationship ( CBaseEntity *pTarget )
{
	if ( pTarget->pev->modelindex == pev->modelindex )
	{
		return R_NO;
	}

	return CBaseMonster :: IRelationship( pTarget );
}

//=========================================================
// ID's NyanCat as their owner
//=========================================================
int CNyanCat::Classify ()
{

	if ( pev->owner && pev->owner->v.flags & FL_CLIENT)
	{
		return CLASS_PLAYER_BIOWEAPON;
	}

	return	CLASS_ALIEN_BIOWEAPON;
}

//=========================================================
// StartTrack - starts a nyancat out tracking its target
//=========================================================
void CNyanCat :: StartTrack ()
{
	IgniteTrail();

	SetTouch( &CNyanCat::TrackTouch );
	SetThink( &CNyanCat::TrackTarget );

	pev->nextthink = gpGlobals->time + 0.1;
}

//=========================================================
// StartDart - starts a nyancat out just flying straight.
//=========================================================
void CNyanCat :: StartDart ()
{
	IgniteTrail();

	SetTouch( &CNyanCat::DartTouch );

	SetThink( &CNyanCat::SUB_Remove );
	pev->nextthink = gpGlobals->time + 4;
}

void CNyanCat::IgniteTrail()
{
/*

  ted's suggested trail colors:

r161
g25
b97

r173
g39
b14

old colors
		case NYANCAT_TYPE_RED:
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 128 );   // r, g, b
			WRITE_BYTE( 0 );   // r, g, b
			break;
		case NYANCAT_TYPE_ORANGE:
			WRITE_BYTE( 0   );   // r, g, b
			WRITE_BYTE( 100 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			break;
	
*/

	// trail
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE(  TE_BEAMFOLLOW );
		WRITE_SHORT( entindex() );	// entity
		WRITE_SHORT( iNyanCatTrail );	// model
		WRITE_BYTE( 10 ); // life
		WRITE_BYTE( 2 );  // width

		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 255 );   // r, g, b

		WRITE_BYTE( 128 );	// brightness

	MESSAGE_END();
}

//=========================================================
// NyanCat is flying, gently tracking target
//=========================================================
void CNyanCat :: TrackTarget ()
{
	Vector	vecFlightDir;
	Vector	vecDirToEnemy;
	float	flDelta;

	StudioFrameAdvance( );

	if (gpGlobals->time > m_flStopAttack)
	{
		SetTouch( NULL );
		SetThink( &CNyanCat::SUB_Remove );
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	// UNDONE: The player pointer should come back after returning from another level
	if ( m_hEnemy == NULL )
	{// enemy is dead.
		Look( 512 );
		m_hEnemy = BestVisibleEnemy( );
	}
	
	if ( m_hEnemy != NULL && FVisible( m_hEnemy ))
	{
		m_vecEnemyLKP = m_hEnemy->BodyTarget( pev->origin );
	}
	else
	{
		m_vecEnemyLKP = m_vecEnemyLKP + pev->velocity * m_flFlySpeed * 0.1;
	}

	vecDirToEnemy = ( m_vecEnemyLKP - pev->origin ).Normalize();

	if (pev->velocity.Length() < 0.1)
		vecFlightDir = vecDirToEnemy;
	else 
		vecFlightDir = pev->velocity.Normalize();

	// measure how far the turn is, the wider the turn, the slow we'll go this time.
	flDelta = DotProduct ( vecFlightDir, vecDirToEnemy );
	
	if ( flDelta < 0.5 )
	{// hafta turn wide again. play sound
		switch (RANDOM_LONG(0,2))
		{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM, 100);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM,  80);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM, 120);	break;
		}
	}


	flDelta = 0.25;

	pev->velocity = ( vecFlightDir + vecDirToEnemy).Normalize();

	if ( pev->owner && (pev->owner->v.flags & FL_MONSTER) )
	{
		// random pattern only applies to nyancats fired by monsters, not players. 

		pev->velocity.x += RANDOM_FLOAT ( -0.10, 0.10 );// scramble the flight dir a bit.
		pev->velocity.y += RANDOM_FLOAT ( -0.10, 0.10 );
		pev->velocity.z += RANDOM_FLOAT ( -0.10, 0.10 );
	}
	

	pev->velocity = pev->velocity * ( m_flFlySpeed * flDelta );// scale the dir by the ( speed * width of turn )
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.3 );

	//pev->velocity = pev->velocity * m_flFlySpeed;// do not have to slow down to turn.
	//pev->nextthink = gpGlobals->time + 0.1;// fixed think time

	pev->angles = UTIL_VecToAngles (pev->velocity);

	pev->solid = SOLID_BBOX;

	// if nyancat is close to the enemy, jet in a straight line for a half second.
	// (only in the single player game)
	if ( m_hEnemy != NULL && !g_pGameRules->IsMultiplayer() )
	{
		if ( flDelta >= 0.4 && ( pev->origin - m_vecEnemyLKP ).Length() <= 300 )
		{
			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
				WRITE_BYTE( TE_SPRITE );
				WRITE_COORD( pev->origin.x);	// pos
				WRITE_COORD( pev->origin.y);
				WRITE_COORD( pev->origin.z);
				WRITE_SHORT( iNyanCatPuff );		// model
				// WRITE_BYTE( 0 );				// life * 10
				WRITE_BYTE( 2 );				// size * 10
				WRITE_BYTE( 128 );			// brightness
			MESSAGE_END();

			switch (RANDOM_LONG(0,2))
			{
			case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM, 100);	break;
			case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM, 80);	break;
			case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM, 120);	break;
			}
			pev->velocity = pev->velocity * 2;
			pev->nextthink = gpGlobals->time + 1.0;
			// don't attack again
			m_flStopAttack = gpGlobals->time;
		}
	}
}

//=========================================================
// Tracking NyanCat hit something
//=========================================================
void CNyanCat :: TrackTouch ( CBaseEntity *pOther )
{
	if ( pOther->edict() == pev->owner || pOther->pev->modelindex == pev->modelindex )
	{// bumped into the guy that shot it.
		pev->solid = SOLID_NOT;
		return;
	}

	if ( IRelationship( pOther ) <= R_NO )
	{
		// hit something we don't want to hurt, so turn around.

		pev->velocity = pev->velocity.Normalize();

		pev->velocity.x *= -1;
		pev->velocity.y *= -1;

		pev->origin = pev->origin + pev->velocity * 4; // bounce the nyancat off a bit.
		pev->velocity = pev->velocity * m_flFlySpeed;

		return;
	}

	DieTouch( pOther );
}

void CNyanCat::DartTouch( CBaseEntity *pOther )
{
	DieTouch( pOther );
}

void CNyanCat::DieTouch ( CBaseEntity *pOther )
{
	if ( pOther && pOther->pev->takedamage )
	{// do the damage

		switch (RANDOM_LONG(0,2))
		{// buzz when you plug someone
			case 0:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM, 100);	break;
			case 1:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM,  80);	break;
			case 2:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "nyancat/cathit.wav", 1, ATTN_NORM, 120);	break;
		}
			
		pOther->TakeDamage( pev, VARS( pev->owner ), pev->dmg, DMG_BULLET );
	}

	pev->modelindex = 0;// so will disappear for the 0.1 secs we wait until NEXTTHINK gets rid
	pev->solid = SOLID_NOT;

	SetThink ( &CNyanCat::SUB_Remove );
	pev->nextthink = gpGlobals->time + 1;// stick around long enough for the sound to finish!
}

