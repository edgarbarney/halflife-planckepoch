/***
*
* Copyright (c) 1996-2001, Valve LLC. All rights reserved.
* 
* This product contains software technology licensed from Id
* Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
* All Rights Reserved.
*
* Use, distribution, and modification of this source code and/or resulting
* object code is restricted to non-commercial enhancements to products from
* Valve LLC.  All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/

#include "CWeatherPrecipitation.h"
#include "CWeatherController.h"

LINK_ENTITY_TO_CLASS(env_weather_precipitation, CWeatherPrecipitation);

/*
TYPEDESCRIPTION CWeatherPrecipitation::m_SaveData[] =
{
    DEFINE_FIELD(CWeatherPrecipitation, type, FIELD_INTEGER),
    DEFINE_FIELD(CWeatherPrecipitation, intensity, FIELD_INTEGER),
    DEFINE_FIELD(CWeatherPrecipitation, sprayDensity, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CWeatherPrecipitation, CBaseEntity);
*/

void CWeatherPrecipitation::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "type"))
    {
        this->type = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "intensity"))
    {
        this->intensity = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "spray_density"))
    {
        this->sprayDensity = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else
    {
        CBaseEntity::KeyValue(pkvd);
    }
}

void CWeatherPrecipitation::Spawn()
{
    pev->solid = SOLID_NOT;
    pev->effects = EF_NODRAW;
    SetThink(&CWeatherPrecipitation::SendWeather);
    if (pev->spawnflags & SF_WEATHER_START_ON) SetNextThink(0.1f);
}

void CWeatherPrecipitation::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    SetNextThink(0.1f);
}

void CWeatherPrecipitation::SendWeather()
{
    auto* controller = CWeatherController::GetInstance();
    controller->SetPrecipitation(this->type, this->intensity, this->sprayDensity);
    DontThink();
}
