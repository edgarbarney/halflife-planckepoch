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

#ifndef NYANCAT_H
#define NYANCAT_H

#include <map>
//=========================================================
// NyanCats
//=========================================================

//=========================================================
// NyanCat Defines
//=========================================================

//Pre-made colours
enum Colours
{
	Red = 0xff0000,
	Orange = 0xffa500,
	Yellow = 0xffff00,
	Green = 0x008000,
	Blue = 0x0000ff,
	Indigo = 0x4b0082,
	Violet = 0xee82ee,
    _TotalColours = 7
};

#define	NYANCAT_NYAN_VOLUME		(float)0.8

extern int iNyanCatPuff;

//=========================================================
// NyanCat - this is the projectile that the Alien Grunt fires.
//=========================================================
class CNyanCat : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	int	 Classify () override;
	int  IRelationship ( CBaseEntity *pTarget ) override;
	int		Save( CSave &save ) override;
	int		Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];

	
	void IgniteTrail();
	void EXPORT StartTrack ();
	void EXPORT StartDart ();
	void EXPORT TrackTarget ();
	void EXPORT TrackTouch ( CBaseEntity *pOther );
	void EXPORT DartTouch( CBaseEntity *pOther );
	void EXPORT DieTouch ( CBaseEntity *pOther );
	virtual void EXPORT DoDamage (CBaseEntity* pOther);
	virtual void EXPORT Glowing(unsigned long lightcolour);

	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) override;

	float			m_flStopAttack;

	float			m_flFlySpeed;

	std::map<int, unsigned long> premadeColours =
	{
		{0, Colours::Red},
		{1, Colours::Orange},
		{2, Colours::Yellow},
		{3, Colours::Green},
		{4, Colours::Blue},
		{5, Colours::Indigo},
		{6, Colours::Violet}
	};
};

class CBigNyan : public CNyanCat
{
	void Spawn() override;
	void Precache() override;
	virtual void EXPORT DoDamage(CBaseEntity* pOther) override;
	virtual void EXPORT Glowing(unsigned long lightcolour) override;

};

#endif //NYANCAT_H