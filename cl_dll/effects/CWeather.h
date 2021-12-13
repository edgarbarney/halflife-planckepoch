/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#pragma once

#include <cstdint>
#include "com_model.h"

enum class PrecipitationType : uint8_t
{
    None = 0,
    Rain = 1,
    Snow = 2
};

class CWeather final
{
public:
    CWeather();
    void SetPrecipitation(PrecipitationType type, int numParticles, int sprayDensity);
    void SetWind(float yaw, float speed, float changeYawVariance, float changeSpeedVariance, float changeFrequency, float changeSpeed);
    float GetLastClientTime() const { return this->lastClientTime; }
    static void RegisterParticleClasses();
    void Initialise();
    void Update();

    model_t* GetRainModel() const { return rainSprite; }
    model_t* GetRainSplashModel() const { return rainSplashSprite; }
    model_t* GetRippleModel() const { return rippleSprite; }

private:
    void UpdateWind(float clientTime);
    void UpdateRain(float clientTime);
    void UpdateSnow(float clientTime);

    void CreateRaindrop(const Vector& origin, float windX, float windY) const;
    void CreateRainSpray(const Vector& origin, const float windX, const float windY) const;
    void CreateSnowflake(const Vector& origin, float windX, float windY) const;

    Vector weatherOrigin;
    PrecipitationType precipitationType;
    int numParticles;
    int sprayDensity;

    float nextWindChangeTime;
    float nextPrecipitationUpdate;

    float baseWindYaw;
    float actualWindYaw;
    float desiredWindYaw;
    float yawPerSecond;

    float baseWindSpeed;
    float actualWindSpeed;
    float desiredWindSpeed;
    float speedPerSecond;

    float windChangeYawVariance;
    float windChangeSpeedVariance;
    float windChangeFrequency;
    float windChangeSpeed;

    float lastClientTime;

	model_t* snowSprite;
	model_t* rainSprite;
	model_t* rippleSprite;
	model_t* rainSplashSprite;
	model_t* rainSpraySprite;
};

extern CWeather g_Weather;
