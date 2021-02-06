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

#include	"cbase.h"
#include    "util.h"

#define SF_WEATHER_START_ON 1

/// <summary>
/// Triggerable entity to alter the current rain/snow parameters.
/// </summary>
class CWeatherPrecipitation : public CBaseEntity
{
public:
    void Spawn() override;
    void KeyValue(KeyValueData* pkvd) override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

    int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

    //int Save(CSave& save) override;
    //int Restore(CRestore& restore) override;
    //static TYPEDESCRIPTION m_SaveData[];

    DLLEXPORT void SendWeather();

private:
    int type = 1;
    int intensity = 100;
    int sprayDensity = 2;
};
