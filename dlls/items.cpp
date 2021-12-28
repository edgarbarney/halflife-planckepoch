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

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"
#include "UserMessages.h"

void I_Precache(void)
{
	// common world objects (moved from W_Precache - weapons.cpp)
	UTIL_PrecacheOther("item_suit");
	UTIL_PrecacheOther("item_battery");
	UTIL_PrecacheOther("item_antidote");
	UTIL_PrecacheOther("item_security");
	UTIL_PrecacheOther("item_longjump");
	UTIL_PrecacheOther("item_healthkit");
	UTIL_PrecacheOther("item_camera");
	UTIL_PrecacheOther("item_flare");
	UTIL_PrecacheOther("item_antirad");
	UTIL_PrecacheOther("item_medicalkit");
}

class CWorldItem : public CBaseEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	int m_iType;
};

LINK_ENTITY_TO_CLASS(world_items, CWorldItem);

bool CWorldItem::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "type"))
	{
		m_iType = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CWorldItem::Spawn()
{
	CBaseEntity* pEntity = NULL;

	switch (m_iType)
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create("item_battery", pev->origin, pev->angles);
		break;
	case 42: // ITEM_ANTIDOTE:
		pEntity = CBaseEntity::Create("item_antidote", pev->origin, pev->angles);
		break;
	case 43: // ITEM_SECURITY:
		pEntity = CBaseEntity::Create("item_security", pev->origin, pev->angles);
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create("item_suit", pev->origin, pev->angles);
		break;
	}

	if (!pEntity)
	{
		ALERT(at_debug, "unable to create world_item %d\n", m_iType);
	}
	else
	{
		pEntity->pev->target = pev->target;
		pEntity->pev->targetname = pev->targetname;
		pEntity->pev->spawnflags = pev->spawnflags;
	}

	REMOVE_ENTITY(edict());
}


void CItem::Spawn()
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin(this, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItem::ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove(this);
		return;
	}
}

void CItem::ItemTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch(pPlayer))
	{
		SUB_UseTargets(pOther, USE_TOGGLE, 0);
		SetTouch(NULL);

		// player grabbed the item.
		g_pGameRules->PlayerGotItem(pPlayer, this);
		if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_YES)
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove(this);
	}
}

CBaseEntity* CItem::Respawn()
{
	SetTouch(NULL);
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin(this, g_pGameRules->VecItemRespawnSpot(this)); // blip to whereever you should respawn.

	SetThink(&CItem::Materialize);
	AbsoluteNextThink(g_pGameRules->FlItemRespawnTime(this));
	return this;
}

void CItem::Materialize()
{
	if ((pev->effects & EF_NODRAW) != 0)
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CItem::ItemTouch);
}

#define SF_SUIT_SHORTLOGON 0x0001

class CItemSuit : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_suit.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_suit.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return false;
		}

		if (pPlayer->HasSuit())
			return false;

		if ((pev->spawnflags & SF_SUIT_SHORTLOGON) != 0)
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0"); // short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx"); // long version of suit logon

		pPlayer->SetHasSuit(true);
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);



class CItemBattery : public CItem
{
	void Spawn() override
	{
		Precache();
		if (pev->model)
			SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
		else
			SET_MODEL(ENT(pev), "models/w_battery.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		if (pev->model)
			PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
		else
			PRECACHE_MODEL("models/w_battery.mdl");

		if (pev->noise)
			PRECACHE_SOUND((char*)STRING(pev->noise)); //LRC
		else
			PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			pPlayer->HasSuit())
		{
			int pct;
			char szcharge[64];

			if (pev->armorvalue)
				pPlayer->pev->armorvalue += pev->armorvalue;
			else
				pPlayer->pev->armorvalue += gSkillData.batteryCapacity;
			pPlayer->pev->armorvalue = V_min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			if (pev->noise)
				EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM); //LRC
			else
				EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();


			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)((float)(pPlayer->pev->armorvalue * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;

			sprintf(szcharge, "!HEV_%1dP", pct);

			//EMIT_SOUND_SUIT(ENT(pev), szcharge);
			pPlayer->SetSuitUpdate(szcharge, false, SUIT_NEXT_IN_30SEC);
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);


void CItemAntidote::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_antidote.mdl");
	CItem::Spawn();
}

void CItemAntidote::Precache()
{
	PRECACHE_MODEL("models/w_antidote.mdl");
}

bool CItemAntidote::MyTouch(CBasePlayer* pPlayer)
{
	pPlayer->SetSuitUpdate("!HEV_DET4", false, SUIT_NEXT_IN_1MIN);

	pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;

	MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); //AJH msg change inventory
	WRITE_SHORT((ITEM_ANTIDOTE));							   //which item to change
	WRITE_SHORT(pPlayer->m_rgItems[ITEM_ANTIDOTE]);			   //set counter to this ammount
	MESSAGE_END();

	if (pev->noise) //AJH
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
	else
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

	return true;
}

void CItemAntidote::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{

	if (!(pActivator->IsPlayer()))
	{
		ALERT(at_debug, "DEBUG: Antidote kit used by non-player\n");
		return;
	}

	CBasePlayer* m_hActivator = (CBasePlayer*)pActivator;
	ALERT(at_console, "HazardSuit: Antitoxin shots remaining: %i\n", m_hActivator->m_rgItems[ITEM_ANTIDOTE]);
}


LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);


class CItemSecurity : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_security.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_security.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		pPlayer->m_rgItems[ITEM_SECURITY] += 1; //AJH implement a new system with different cards instead of just MORE cards

		MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); //AJH msg change inventory
		WRITE_SHORT((ITEM_SECURITY));							   //which item to change
		WRITE_SHORT(pPlayer->m_rgItems[ITEM_SECURITY]);			   //set counter to this ammount
		MESSAGE_END();

		if (pev->noise) //AJH
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
		else
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);

class CItemLongJump : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_longjump.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_longjump.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_fLongJump)
		{
			return false;
		}

		if (pPlayer->HasSuit())
		{
			pPlayer->m_fLongJump = true; // player now has longjump module

			g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "1");

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); //AJH msg change inventory
			WRITE_SHORT((ITEM_LONGJUMP));							   //which item to change
			WRITE_SHORT(1);											   //set counter to this ammount
			MESSAGE_END();

			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1"); // Play the longjump sound UNDONE: Kelly? correct sound?
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);



LINK_ENTITY_TO_CLASS(item_medicalkit, CItemMedicalKit);

#define CHARGE_IN_MEDKIT 50 //Possibly implement skill system
#define MAX_MEDKIT 200		//possibly allow mapper to change, say in worldspawn


#define ITEM_NOTPICKEDUP 0
#define ITEM_PICKEDUP 1
#define ITEM_DRAINED 2 //The item has had some 'charge' removed but remains in existence

void CItemMedicalKit ::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_portablemed.mdl"); // create a new model and spawn it here
	pev->dmg = pev->health;							 //Store initial charge

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin(this, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItemMedicalKit::ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove(this);
		return;
	}
}

void CItemMedicalKit::Precache(void)
{
	PRECACHE_MODEL("models/w_portablemed.mdl"); // create a new model and precache it here
	PRECACHE_SOUND("items/smallmedkit1.wav");
}

bool CItemMedicalKit::MyTouch(CBasePlayer* pPlayer)
{
	if (pPlayer->pev->deadflag != DEAD_NO || pPlayer->m_rgItems[ITEM_HEALTHKIT] >= (int)CVAR_GET_FLOAT("max_medkit"))
	{
		return false;
	}
	pPlayer->m_rgItems[ITEM_HEALTHKIT] += (pev->health) ? pev->health : CHARGE_IN_MEDKIT; //increment/decrement counter by this ammount
	MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev);							  //msg change inventory
	WRITE_SHORT((ITEM_HEALTHKIT));														  //which item to change
	WRITE_SHORT(pPlayer->m_rgItems[ITEM_HEALTHKIT]);									  //set counter to this ammount
	MESSAGE_END();

	if (pPlayer->m_rgItems[ITEM_HEALTHKIT] > MAX_MEDKIT) // We have more 'charge' than the player is allowed to have
	{
		pev->health = pPlayer->m_rgItems[ITEM_HEALTHKIT] - MAX_MEDKIT; //set the amount of charge left in the kit to be the difference
		pPlayer->m_rgItems[ITEM_HEALTHKIT] = MAX_MEDKIT;			   //set players kit charge to max
																	   //	Respawn();// respawn the model (with the new decreased charge)
		return true;
	}
	else
	{
		if (g_pGameRules->ItemShouldRespawn(this))
		{
			pev->health = pev->dmg; //Reset initial health;
			Respawn();
		}
		else
		{
			SetTouch(NULL); //Is this necessary?
			UTIL_Remove(this);
		}

		return true;
	}
}

void CItemMedicalKit::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{

	if (!(pActivator->IsPlayer()))
	{
		ALERT(at_debug, "DEBUG: Medical kit used by non-player\n");
		return;
	}

	EMIT_SOUND(pActivator->edict(), CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM); //play sound to tell player it's been used

	CBasePlayer* m_hActivator = (CBasePlayer*)pActivator;
	int startHealth = m_hActivator->pev->health;
	m_hActivator->TakeHealth(m_hActivator->m_rgItems[ITEM_HEALTHKIT], DMG_GENERIC); //Actually give the health
	int m_fHealthUsed = startHealth - m_hActivator->pev->health;
	m_hActivator->m_rgItems[ITEM_HEALTHKIT] -= m_fHealthUsed;												 //increment/decrement counter by this ammount

	MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, m_hActivator->pev); //msg change inventory
	WRITE_SHORT((ITEM_HEALTHKIT));									//which item to change
	WRITE_SHORT(m_hActivator->m_rgItems[ITEM_HEALTHKIT]);			//set counter to this ammount
	MESSAGE_END();

	ALERT(at_console, "AutoMedicalKit: I have healed %i health\n", m_fHealthUsed);
	ALERT(at_console, "AutoMedicalKit: Charge remaining for healing: %i\n", m_hActivator->m_rgItems[ITEM_HEALTHKIT]);
}

void CItemMedicalKit::ItemTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch(pPlayer))
	{
		if (pev->noise)
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
		else
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		SUB_UseTargets(pOther, USE_TOGGLE, 0);

		// player grabbed the item.
		g_pGameRules->PlayerGotItem(pPlayer, this);
	}
}

void CItemAntiRad::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_rad.mdl");
	CItem::Spawn();
}

void CItemAntiRad::Precache(void)
{
	PRECACHE_MODEL("models/w_rad.mdl");
}

bool CItemAntiRad::MyTouch(CBasePlayer* pPlayer)
{
	pPlayer->SetSuitUpdate("!HEV_DET5", false, SUIT_NEXT_IN_1MIN); //TODO: find right suit notifcation

	pPlayer->m_rgItems[ITEM_ANTIRAD] += 1;
	MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); //AJH msg change inventory
	WRITE_SHORT((ITEM_ANTIRAD));							   //which item to change
	WRITE_SHORT(pPlayer->m_rgItems[ITEM_ANTIRAD]);			   //set counter to this ammount
	MESSAGE_END();
	if (pev->noise) //AJH
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
	else
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

	return true;
}

void CItemAntiRad::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{

	if (!(pActivator->IsPlayer()))
	{
		ALERT(at_debug, "DEBUG: AntiRad syringe used by non-player\n");
		return;
	}

	CBasePlayer* m_hActivator = (CBasePlayer*)pActivator;
	ALERT(at_console, "HazardSuit: AntiRad shots remaining: %i\n", m_hActivator->m_rgItems[ITEM_ANTIRAD]);
}
LINK_ENTITY_TO_CLASS(item_antirad, CItemAntiRad);



void CItemFlare::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_flare.mdl");
	CItem::Spawn();
}

void CItemFlare::Precache(void)
{
	PRECACHE_MODEL("models/w_flare.mdl");
}

bool CItemFlare::MyTouch(CBasePlayer* pPlayer)
{
	pPlayer->m_rgItems[ITEM_FLARE] += 1;
	MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); //AJH msg change inventory
	WRITE_SHORT((ITEM_FLARE));								   //which item to change
	WRITE_SHORT(pPlayer->m_rgItems[ITEM_FLARE]);			   //set counter to this ammount
	MESSAGE_END();

	if (pev->noise)
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
	else
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

	return true;
}

void CItemFlare::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{

	if (!(pActivator->IsPlayer()))
	{
		ALERT(at_debug, "DEBUG: FLARE used by non-player\n");
		return;
	}

	CBasePlayer* m_hActivator = (CBasePlayer*)pActivator;

	if (m_hActivator->m_rgItems[ITEM_FLARE] > 0)
	{
		m_hActivator->m_rgItems[ITEM_FLARE]--; //increment/decrement counter by one

		MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, m_hActivator->pev); //msg change inventory
		WRITE_SHORT((ITEM_FLARE));										//which item to change
		WRITE_SHORT(m_hActivator->m_rgItems[ITEM_FLARE]);				//set counter to this ammount
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY); //Activate Flare
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(m_hActivator->pev->origin.x); // origin
		WRITE_COORD(m_hActivator->pev->origin.y);
		WRITE_COORD(m_hActivator->pev->origin.z);
		WRITE_BYTE(32); // radius
		if (pev->rendercolor)
		{
			WRITE_BYTE(pev->rendercolor.x); // R
			WRITE_BYTE(pev->rendercolor.y); // G
			WRITE_BYTE(pev->rendercolor.z); // B
		}
		else
		{
			WRITE_BYTE(255); // R
			WRITE_BYTE(255); // G
			WRITE_BYTE(250); // B
		}
		if (pev->ltime)
		{
			WRITE_BYTE((int)(pev->ltime * 10)); // ltime = life time
		}
		else
		{
			WRITE_BYTE(600); // life * 10
		}
		WRITE_BYTE(0); // decay
		MESSAGE_END();

		ALERT(at_console, "HazardSuit: %i flares remaining.\n", m_hActivator->m_rgItems[ITEM_FLARE]);
	}
	else
		ALERT(at_console, "HazardSuit: No flares available for use!.\n");
}
LINK_ENTITY_TO_CLASS(item_flare, CItemFlare);




#define SF_CAMERA_PLAYER_POSITION 1
#define SF_CAMERA_PLAYER_TARGET 2
#define SF_CAMERA_PLAYER_TAKECONTROL 4
#define SF_CAMERA_DRAWHUD 16

//Dont Change these values, they are assumed in the client.
#define ITEM_CAMERA_ACTIVE 5
#define CAMERA_DRAWPLAYER 8

#define MAX_CAMERAS 4

LINK_ENTITY_TO_CLASS(item_camera, CItemCamera);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION CItemCamera::m_SaveData[] =
	{
		//DEFINE_FIELD( CItemCamera, m_hPlayer, FIELD_EHANDLE ),
		DEFINE_FIELD(CItemCamera, m_state, FIELD_INTEGER),
		DEFINE_FIELD(CItemCamera, m_iobjectcaps, FIELD_INTEGER),
		DEFINE_FIELD(CItemCamera, m_pNextCamera, FIELD_CLASSPTR),
		DEFINE_FIELD(CItemCamera, m_pLastCamera, FIELD_CLASSPTR),
};

IMPLEMENT_SAVERESTORE(CItemCamera, CItem);

void CItemCamera::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_camera.mdl");
	pev->movetype = MOVETYPE_NONE;
	pev->classname = MAKE_STRING("item_camera");
	m_iobjectcaps = 0;
	if (pev->targetname == NULL)
		pev->targetname = MAKE_STRING("item_camera");
	m_state = 0;
	m_pLastCamera = NULL;
	m_pNextCamera = NULL;

	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin(this, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItemCamera::ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove(this);
		return;
	}
	pev->oldorigin = pev->origin; //Remeber where we respawn (must be after DROP_TO_FLOOR)
}

void CItemCamera::Precache(void)
{
	PRECACHE_MODEL("models/w_camera.mdl");
}

void CItemCamera::ItemTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	// Don't touch me too often!!
	if (gpGlobals->time <= pev->dmgtime)
		return;
	pev->dmgtime = gpGlobals->time + 0.5;

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch(pPlayer))
	{
		if (pev->noise)
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
		else
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		SUB_UseTargets(pOther, USE_TOGGLE, 0);

		// player grabbed the item.
		g_pGameRules->PlayerGotItem(pPlayer, this);
	}
}

bool CItemCamera::MyTouch(CBasePlayer* pPlayer)
{
	if (pPlayer->m_rgItems[ITEM_CAMERA] < (int)CVAR_GET_FLOAT("max_cameras"))
	{

		if (pPlayer->m_pItemCamera == NULL)
		{
			pPlayer->m_pItemCamera = this;
			pPlayer->m_pItemCamera->m_pLastCamera = this;
			pPlayer->m_pItemCamera->m_pNextCamera = this;
		}
		else
		{
			if (pPlayer->m_pItemCamera->m_pLastCamera == NULL)
			{
				ALERT(at_debug, "MYTOUCH: Null pointer in camera list!! (Impossible?!)\n"); //Shouldn't be here!
				return false;
			}
			pPlayer->m_pItemCamera->m_pLastCamera->m_pNextCamera = this; //Set the current last camera to point to us
			pPlayer->m_pItemCamera->m_pLastCamera = this;				 //then set us as the last camera in the list.
		}
		pPlayer->m_rgItems[ITEM_CAMERA] += 1;
		MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); //AJH msg change inventory
		WRITE_SHORT((ITEM_CAMERA));								   //which item to change
		WRITE_SHORT(pPlayer->m_rgItems[ITEM_CAMERA]);			   //set counter to this ammount
		MESSAGE_END();
		SetTouch(NULL);

		//pev->solid = SOLID_NOT;		// Remove model & collisions
		//pev->renderamt = 0;								// The engine won't draw this model if this is set to 0 and blending is on
		//pev->rendermode = kRenderTransTexture;
		pev->effects |= EF_NODRAW; //Don't draw the model

		m_iobjectcaps |= FCAP_ACROSS_TRANSITION;
		pev->movetype = MOVETYPE_FOLLOW; //Follow the player (so that level transitions work)
		pev->aiment = pPlayer->edict();
		pev->owner = pPlayer->edict();

		return true;
	}
	return false;
}

void CItemCamera::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!(pActivator->IsPlayer()))
	{
		ALERT(at_debug, "DEBUG: Camera used by non-player\n");
		return;
	}
	CBasePlayer* pPlayer = (CBasePlayer*)pActivator;
	//m_hPlayer = pActivator;

	if (pPlayer->m_rgItems[ITEM_CAMERA] <= 0)
	{
		ALERT(at_debug, "DEBUG: Player attempted to use a camera but he has none!\n");
		return;
	}

	if (useType == USE_TOGGLE)
	{
		// next state
		m_state++;
		if (m_state > 2)
			m_state = 0;
	}
	else
	{
		m_state = (int)value;
	}

	if (m_state == 0) //We are exiting the camera view, and moving the camera pointers to the next camera.
	{
		//ALERT(at_debug,"DEBUG: Camera destroyed by user\n");
		if (pPlayer->m_rgItems[ITEM_CAMERA] > 0)
		{
			pPlayer->m_rgItems[ITEM_CAMERA]--;						   //decrement counter by one
			MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); //msg change inventory
			WRITE_SHORT((ITEM_CAMERA));								   //which item to change
			WRITE_SHORT(pPlayer->m_rgItems[ITEM_CAMERA]);			   //set counter to this ammount
			MESSAGE_END();

			pPlayer->viewEntity = 0;
			pPlayer->viewFlags = 0;
			pPlayer->viewNeedsUpdate = 1;
			pPlayer->EnableControl(true);

			CLIENT_COMMAND(pPlayer->edict(), "hideplayer\n");

			if (pPlayer->m_pItemCamera->m_pLastCamera == NULL || pPlayer->m_pItemCamera->m_pNextCamera == NULL || pPlayer->m_rgItems[ITEM_CAMERA] <= 0)
			{								   //the player is out of cameras
				pPlayer->m_pItemCamera = NULL; // Tell the player they don't have any more cameras!
				ALERT(at_debug, "USE: Player has no more cameras.\n");
			}
			else //Set the next camera the player can use
			{
				pPlayer->m_pItemCamera->m_pNextCamera->m_pLastCamera = pPlayer->m_pItemCamera->m_pLastCamera;
				pPlayer->m_pItemCamera = pPlayer->m_pItemCamera->m_pNextCamera;
			}

			if (g_pGameRules->ItemShouldRespawn(this))
			{
				pev->origin = pev->oldorigin; //Reset initial position;
				m_iobjectcaps &= ~FCAP_ACROSS_TRANSITION;
				pev->movetype = MOVETYPE_NONE;
				pev->aiment = NULL;
				pev->owner = NULL;
				pev->takedamage = DAMAGE_NO;
				pev->dmg = 0;
				Respawn();
			}
			else
			{
				SetTouch(NULL); //Is this necessary?
				UTIL_Remove(this);
			}
		}
		return;
	}
	else if (m_state == 1) //Drop the camera at the current location
	{
		if (pPlayer->m_rgItems[ITEM_CAMERA] > 0)
		{
			// copy over player information
			UTIL_SetOrigin(this, pPlayer->pev->origin + pPlayer->pev->view_ofs);
			pev->angles.x = -pPlayer->pev->v_angle.x;
			pev->angles.y = pPlayer->pev->v_angle.y;
			pev->angles.z = 0;

			//Get the engine to draw the model again
			pev->effects &= ~EF_NODRAW;

			m_iobjectcaps &= ~FCAP_ACROSS_TRANSITION;
			pev->movetype = MOVETYPE_NONE; // Stop following the player
			pev->aiment = NULL;
			pev->owner = NULL;
			pev->takedamage = DAMAGE_YES;
		}
	}
	else if (m_state == 2) //Look through the camera
	{
		if (pPlayer->m_rgItems[ITEM_CAMERA] > 0)
		{
			if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL))
			{
				pPlayer->EnableControl(false);
			}

			//	This is redundant 'viewFlags |= CAMERA_DRAWPLAYER' has the same effect.
			//	CLIENT_COMMAND(pPlayer->edict(),"drawplayer\n");

			int sendflags;
			sendflags = sendflags | ITEM_CAMERA_ACTIVE | CAMERA_DRAWPLAYER;

			pPlayer->viewEntity = pev->targetname;
			//	ALERT(at_debug,"Player looks through camera '%s'\n",(pev->targetname)?STRING(pev->targetname):"NULL");
			pPlayer->viewFlags = sendflags;
			pPlayer->viewNeedsUpdate = 1;

			m_iobjectcaps &= ~FCAP_ACROSS_TRANSITION;
			pev->movetype = MOVETYPE_NONE; // Stop following the player
			pev->aiment = NULL;
			pev->owner = NULL;

			pev->takedamage = DAMAGE_YES;

			//SetThink(&CItemCamera:: Think ); //allow the player to control the camera view?
			//SetNextThink( 0 );
		}
	}

	if (m_state == 3) //We are exiting the camera view (without losing the camera)
	{				  //We don't update the camera list as the player wants to come back to this one.

		//ALERT(at_debug,"DEBUG: Camera turns off (reusable)\n");
		if (pPlayer->m_rgItems[ITEM_CAMERA] > 0)
		{
			pPlayer->viewEntity = 0;
			pPlayer->viewFlags = 0;
			pPlayer->viewNeedsUpdate = 1;
			pPlayer->EnableControl(true);

			m_iobjectcaps &= ~FCAP_ACROSS_TRANSITION;
			pev->movetype = MOVETYPE_NONE; // Stop following the player
			pev->aiment = NULL;
			pev->owner = NULL;

			pev->takedamage = DAMAGE_YES;

			CLIENT_COMMAND(pPlayer->edict(), "hideplayer\n");
		}
		return;
	}
}
/*
CBaseEntity* CItemCamera::Respawn( void ) //Not needed
{
	SetTouch( NULL );
	pev->effects |= EF_NODRAW;
	pev->origin=pev->oldorigin; //Reset initial position;				
	UTIL_SetOrigin( this, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.

	SetThink(&CItemCamera:: Materialize );
	AbsoluteNextThink( g_pGameRules->FlItemRespawnTime( this ) );
	ALERT(at_debug,"CItemCamera::Respawn\n");
	return this;
}



void CItemCamera::Materialize( void ) //Not needed
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150 );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
		pev->solid = SOLID_TRIGGER;
	}
	
	ALERT(at_debug,"CItemCamera::Materialize\n");
	SetTouch(&CItemCamera:: ItemTouch );
}
*/

//Called when a player dies (or otherwise loses their inventory)
//Strips all cameras the player is carrying. (This does NOT reset the players inventory list)
void CItemCamera::StripFromPlayer()
{
	if (m_pNextCamera)
	{
		m_pNextCamera->StripFromPlayer();
	}
	m_pNextCamera = NULL;
	m_pLastCamera = NULL;

	if (g_pGameRules->ItemShouldRespawn(this))
	{
		Respawn();
	}
	else
	{
		SetTouch(NULL); //Is this necessary?
		UTIL_Remove(this);
	}
}

/*void CItemCamera::Think( )
{
	Do Pan/player control code?

	SetNextThink( 0.1 );
}*/

bool CItemCamera::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{

	if (pev->health <= 0)
	{
		ALERT(at_debug, "An invulnerable camera has been attacked\n");
		return false; //This camera is invulnerable
	}

	pev->dmg += flDamage;

	if (pev->health - pev->dmg <= 0)
	{
		CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);
		ALERT(at_console, "Your camera has been destroyed!!\n");
		Use(pAttacker, this, USE_SET, 0);
		return true;
	}

	ALERT(at_console, "Your camera is being attacked!!\n");
	return false;
}
