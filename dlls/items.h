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


class CItem : public CBaseEntity
{
public:
	void	Spawn() override;
	CBaseEntity*	Respawn() override;
	void	EXPORT ItemTouch( CBaseEntity *pOther );
	virtual void EXPORT Materialize();
	virtual BOOL MyTouch( CBasePlayer *pPlayer ) { return FALSE; }
};

class CItemBattery : public CItem
{
	void Spawn() override;
	void Precache() override;

	void EXPORT GlowThink();

	virtual void EXPORT Materialize() override;

	BOOL MyTouch(CBasePlayer* pPlayer) override;
};

#endif // ITEMS_H
