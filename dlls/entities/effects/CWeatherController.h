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
* Valve LLC.All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/
#pragma once

#include	"extdll.h"
#include	"cbase.h"

/// <summary>
/// This entity isn't placed by the mapper, it is
/// automatically spawned if needed by the other env_weather_*
/// entities. There should only ever be one instance of this
/// entity in the map. It's used to remember the current
/// rain settings after a save/restore.
/// </summary>
class CWeatherController : public CBaseEntity
{
public:
    void Spawn() override;
    void SetPrecipitation(int type, int intensity, int sprayDensity);
    void SetWind(float windYaw, float windSpeed, float yawVariance, float speedVariance, float changeFrequency, float changeSpeed);

    int ObjectCaps() override
    {
        return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_MUST_SPAWN;
    }

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    static TYPEDESCRIPTION m_SaveData[];

    DLLEXPORT void DelayAndSendWeather();
    DLLEXPORT void SendWeather();

    static CWeatherController* GetInstance();

private:
    int type = 0;
    int intensity = 0;
    int sprayDensity = 0;

    float windSpeed = 0;
    float windYaw = 0;
    float yawVariance = 0;
    float speedVariance = 0;
    float changeFrequency = 0;
    float changeSpeed = 0;
};
