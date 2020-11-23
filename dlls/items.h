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
#ifndef ITEMS_H
#define ITEMS_H

// AJH Inventory items, some of which can be manually used, others automatically
// and are here just so the player knows they have it/them.
//This  has been moved from weapons.h
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_ANTIRAD		3
#define ITEM_SECURITY		4
#define ITEM_LONGJUMP		5
#define ITEM_FLARE			6
#define ITEM_CAMERA			7
//AJH extended inventory (rename me when I'm implemented)
#define ITEM_SLOT8			8
#define ITEM_SLOT9			9
#define ITEM_SLOT10			10

class CItem : public CBaseEntity
{
public:
	void	Spawn( void );
	CBaseEntity*	Respawn( void );
	void	EXPORT ItemTouch( CBaseEntity *pOther );
	void	EXPORT Materialize( void );
	virtual BOOL MyTouch( CBasePlayer *pPlayer ) { return FALSE; };
};

class CItemMedicalKit : public CItem	//AJH new inventory based manual use medkit
{
public:
	void Spawn( void );
	void Precache( void );
	int MyTouch( CBasePlayer *pPlayer );
	void EXPORT ItemTouch(CBaseEntity *pOther);
	void CItemMedicalKit::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

};

class CItemAntiRad : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MyTouch( CBasePlayer *pPlayer );
	void CItemAntiRad::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

class CItemAntidote : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MyTouch( CBasePlayer *pPlayer );
	void CItemAntidote::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

class CItemFlare : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MyTouch( CBasePlayer *pPlayer );
	void CItemFlare::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

class CItemCamera : public CItem //AJH new inventory camera (can be placed anywhere in a level by the player)
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	//void Think( void );
	void Precache( void );
	void EXPORT ItemTouch(CBaseEntity *pOther);
	int MyTouch( CBasePlayer *pPlayer );
	//CBaseEntity* Respawn(void);
	//void Materialize(void);
	void StripFromPlayer(void);
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );

	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	virtual int	ObjectCaps( void ) { return m_iobjectcaps; }
	static	TYPEDESCRIPTION m_SaveData[];

	//EHANDLE m_hPlayer;
	int	m_state;
	int	m_iobjectcaps;
	CItemCamera* m_pNextCamera;
	CItemCamera* m_pLastCamera;

};
#endif // ITEMS_H
