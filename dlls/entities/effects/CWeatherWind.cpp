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

#include "CWeatherWind.h"
#include "CWeatherController.h"

LINK_ENTITY_TO_CLASS(env_weather_wind, CWeatherWind);

TYPEDESCRIPTION CWeatherWind::m_SaveData[] =
{
    DEFINE_FIELD(CWeatherWind, windSpeed, FIELD_FLOAT),
    DEFINE_FIELD(CWeatherWind, yawVariance, FIELD_FLOAT),
    DEFINE_FIELD(CWeatherWind, speedVariance, FIELD_FLOAT),
    DEFINE_FIELD(CWeatherWind, changeFrequency, FIELD_FLOAT),
    DEFINE_FIELD(CWeatherWind, changeSpeed, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CWeatherWind, CBaseEntity);

void CWeatherWind::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "wind_speed"))
    {
        this->windSpeed = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "yaw_variance"))
    {
        this->yawVariance = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "speed_variance"))
    {
        this->speedVariance = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "change_frequency"))
    {
        this->changeFrequency = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "change_speed"))
    {
        this->changeSpeed = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else
    {
        CBaseEntity::KeyValue(pkvd);
    }
}

void CWeatherWind::Spawn()
{
    pev->solid = SOLID_NOT;
    pev->effects = EF_NODRAW;
    SetThink(&CWeatherWind::SendWeather);
    if (pev->spawnflags & SF_WEATHER_START_ON) SetNextThink(0.1f);
}

void CWeatherWind::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    SetNextThink(0.1f);
}

void CWeatherWind::SendWeather()
{
    auto* controller = CWeatherController::GetInstance();
    controller->SetWind(this->pev->angles.y, this->windSpeed, this->yawVariance, this->speedVariance, this->changeFrequency, this->changeSpeed);
    DontThink();
}
