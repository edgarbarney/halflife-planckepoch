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

#include "CWeatherController.h"

extern int gmsgWeather;

LINK_ENTITY_TO_CLASS(env_weather_controller, CWeatherController);

TYPEDESCRIPTION CWeatherController::m_SaveData[] =
{
    DEFINE_FIELD(CWeatherController, type, FIELD_INTEGER),
    DEFINE_FIELD(CWeatherController, intensity, FIELD_INTEGER),
    DEFINE_FIELD(CWeatherController, sprayDensity, FIELD_INTEGER),
    DEFINE_FIELD(CWeatherController, windSpeed, FIELD_FLOAT),
    DEFINE_FIELD(CWeatherController, yawVariance, FIELD_FLOAT),
    DEFINE_FIELD(CWeatherController, speedVariance, FIELD_FLOAT),
    DEFINE_FIELD(CWeatherController, changeFrequency, FIELD_FLOAT),
    DEFINE_FIELD(CWeatherController, changeSpeed, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CWeatherController, CBaseEntity);

void CWeatherController::Spawn()
{
    pev->solid = SOLID_NOT;
    pev->effects = EF_NODRAW;
    SetThink(&CWeatherController::DelayAndSendWeather);
    SetNextThink(0);
}

void CWeatherController::SetPrecipitation(int type, int intensity, int sprayDensity)
{
    this->type = type;
    this->intensity = intensity;
    this->sprayDensity = sprayDensity;
    SendWeather();
}

void CWeatherController::SetWind(float windYaw, float windSpeed, float yawVariance, float speedVariance, float changeFrequency, float changeSpeed)
{
    this->windYaw = windYaw;
    this->windSpeed = windSpeed;
    this->yawVariance = yawVariance;
    this->speedVariance = speedVariance;
    this->changeFrequency = changeFrequency;
    this->changeSpeed = changeSpeed;
    SendWeather();
}


void CWeatherController::DelayAndSendWeather()
{
    // This is just here when loading save games, the NextThink
    // can't correctly delay to allow for the client to load in
    // so we delay a bit more on spawn before sending the weather
    SetNextThink(0.2f);
    SetThink(&CWeatherController::SendWeather);
}

void CWeatherController::SendWeather()
{
    MESSAGE_BEGIN(MSG_ALL, gmsgWeather);
    {
        WRITE_BYTE(1); // precipitation
        WRITE_BYTE(this->type);
        WRITE_SHORT(this->intensity);
        WRITE_BYTE(this->sprayDensity);
    }
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_ALL, gmsgWeather);
    {
        WRITE_BYTE(2); // wind
        WRITE_COORD(this->windYaw);
        WRITE_COORD(this->windSpeed);
        WRITE_COORD(this->yawVariance);
        WRITE_COORD(this->speedVariance);
        WRITE_COORD(this->changeFrequency);
        WRITE_COORD(this->changeSpeed);
    }
    MESSAGE_END();
    DontThink();
}

CWeatherController* CWeatherController::GetInstance()
{
    auto* find = FIND_ENTITY_BY_CLASSNAME(nullptr, "env_weather_controller");
    if (FNullEnt(find))
    {
        auto* instance = GetClassPtr(static_cast<CWeatherController*>(nullptr));
        instance->pev->classname = MAKE_STRING("env_weather_controller");
        instance->Spawn();
        return instance;
    }
    else
    {
        return reinterpret_cast<CWeatherController*>(Instance(find));
    }
}
