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
#include "hud.h"
#include "cl_util.h"
#include "event_api.h"
#include "triangleapi.h"

#include "pm_shared.h"
#include "pm_defs.h"
#include "com_model.h"

#include "r_studioint.h"

#include "particleman.h"

#include "CWeather.h"
#include "particles/CRaindrop.h"
#include "particles/CRainSpray.h"
#include "particles/CSnowflake.h"

//TODO: move - Solokiller
extern engine_studio_api_t IEngineStudio;
extern Vector g_vPlayerVelocity;

CWeather g_Weather;


CWeather::CWeather()
{
    this->precipitationType = PrecipitationType::None;
    this->numParticles = 0;
    this->nextPrecipitationUpdate = 0;

    this->baseWindYaw = 0;
    this->actualWindYaw = this->baseWindYaw;
    this->desiredWindYaw = this->baseWindYaw;

    this->baseWindSpeed = 0;
    this->actualWindSpeed = this->baseWindSpeed;
    this->desiredWindSpeed = this->baseWindSpeed;

    this->nextWindChangeTime = -1;
    this->windChangeYawVariance = 0;
    this->windChangeSpeedVariance = 0;
    this->windChangeFrequency = 0;
    this->windChangeSpeed = 0;
}

void CWeather::Initialise()
{
    this->lastClientTime = gEngfuncs.GetClientTime();
    this->nextPrecipitationUpdate = 0;

    this->rainSprite = const_cast<model_t*>(gEngfuncs.GetSpritePointer(gEngfuncs.pfnSPR_Load("sprites/effects/rain.spr")));
    this->snowSprite = const_cast<model_t*>(gEngfuncs.GetSpritePointer(gEngfuncs.pfnSPR_Load("sprites/effects/snowflake.spr")));
    this->rippleSprite = const_cast<model_t*>(gEngfuncs.GetSpritePointer(gEngfuncs.pfnSPR_Load("sprites/effects/ripple.spr")));
    this->rainSplashSprite = const_cast<model_t*>(gEngfuncs.GetSpritePointer(gEngfuncs.pfnSPR_Load("sprites/wsplash3.spr")));
    this->rainSpraySprite = const_cast<model_t*>(gEngfuncs.GetSpritePointer(gEngfuncs.pfnSPR_Load("sprites/effects/rainspray.spr")));
}

void CWeather::SetPrecipitation(PrecipitationType type, int numParticles, int sprayDensity)
{
    this->precipitationType = type;
    this->numParticles = numParticles;
    this->sprayDensity = sprayDensity;
}

void CWeather::SetWind(
    float yaw, float speed,
    float changeYawVariance, float changeSpeedVariance,
    float changeFrequency, float changeSpeed)
{
    this->baseWindYaw = yaw;
    this->baseWindSpeed = speed;
    this->nextWindChangeTime = -1;
    this->windChangeYawVariance = changeYawVariance;
    this->windChangeSpeedVariance = changeSpeedVariance;
    this->windChangeFrequency = changeFrequency;
    this->windChangeSpeed = changeSpeed;
}

void CWeather::RegisterParticleClasses()
{
    g_pParticleMan->AddCustomParticleClassSize(sizeof(CRaindrop));
    g_pParticleMan->AddCustomParticleClassSize(sizeof(CSnowflake));
}

void CWeather::Update()
{
    if (!IEngineStudio.IsHardware()) return; // not in software mode

    // Get the client's origin
    auto vecOrigin = gHUD.m_vecOrigin;

    // If we're spectating, use that origin instead
    if (g_iUser1 > 0 && g_iUser1 != OBS_ROAMING)
    {
        if (cl_entity_t* pFollowing = gEngfuncs.GetEntityByIndex(g_iUser2))
        {
            vecOrigin = pFollowing->origin;
        }
    }

    // The weather starts above the player
    vecOrigin.z += 36.0f;

    this->weatherOrigin = vecOrigin;

    // See if we need to update the weather
    const auto clientTime = gEngfuncs.GetClientTime();

    UpdateWind(clientTime);

    if (nextPrecipitationUpdate <= clientTime)
    {
        switch (precipitationType)
        {
            case PrecipitationType::None:
                break;
            case PrecipitationType::Rain:
                UpdateRain(clientTime);
                break;
            case PrecipitationType::Snow:
                UpdateSnow(clientTime);
                break;
        }
    }

    this->lastClientTime = clientTime;
}

// todo: move all the util stuff to shared code
#define M_PI 3.14159265358979323846	// matches value in gcc v2 math.h
float UTIL_Approach(float target, float value, float speed)
{
    float delta = target - value;
    if (speed < 0) speed = -speed;

    if (delta > speed) value += speed; 
    else if (delta < -speed) value -= speed;
    else value = target;

    return value;
}
float UTIL_ApproachAngle(float target, float value, float speed)
{
    float delta = target - value;
    if (speed < 0) speed = -speed;

    if (delta < -180) delta += 360;
    else if (delta > 180) delta -= 360;

    if (delta > speed) value += speed;
    else if (delta < -speed) value -= speed;
    else value = target;

    return value;
}

void CWeather::UpdateWind(float clientTime)
{
    if (this->windChangeFrequency > 0 && this->nextWindChangeTime <= clientTime)
    {
        // Change the wind if it's time
        const auto yawVariance = this->windChangeYawVariance;
        const auto speedVariance = this->windChangeSpeedVariance;
        this->desiredWindYaw = this->baseWindYaw + gEngfuncs.pfnRandomFloat(-yawVariance, yawVariance);
        this->desiredWindSpeed = this->baseWindSpeed + gEngfuncs.pfnRandomFloat(-speedVariance, speedVariance);

        if (this->desiredWindYaw < 0 || this->desiredWindYaw >= 360)
        {
            this->desiredWindYaw = static_cast<float>(fmod(this->desiredWindYaw, 360));
            if (this->desiredWindYaw < 0) this->desiredWindYaw += 360;
        }

        const auto changeTime = this->windChangeFrequency;
        this->nextWindChangeTime = gEngfuncs.pfnRandomFloat(changeTime * 0.5f, changeTime * 1.5f) + this->windChangeSpeed;

        if (this->windChangeSpeed < 0.01f)
        {
            // change instantly
            this->yawPerSecond = 360;
            this->speedPerSecond = 10000; // if your wind is faster than this you can wait
        }
        else
        {
            // change over time
            this->yawPerSecond = (this->desiredWindYaw - this->actualWindYaw) / this->windChangeSpeed;
            this->speedPerSecond = (this->desiredWindSpeed - this->actualWindSpeed) / this->windChangeSpeed;
        }
    }

    const auto delta = clientTime - this->lastClientTime;

    if (this->desiredWindYaw != this->actualWindYaw)
    {
        this->actualWindYaw = UTIL_ApproachAngle(this->desiredWindYaw, this->actualWindYaw, this->yawPerSecond * delta);
    }
    if (this->desiredWindSpeed != this->actualWindSpeed)
    {
        this->actualWindSpeed = UTIL_Approach(this->desiredWindSpeed, this->actualWindSpeed, this->speedPerSecond * delta);
    }
}

void CWeather::UpdateRain(float clientTime)
{
    if (!rainSprite) return;

    nextPrecipitationUpdate = clientTime + 0.3f;

    auto sprayIndex = 0;
    switch (this->sprayDensity)
    {
        case 1: sprayIndex = 20; break;
        case 2: sprayIndex = 15; break;
        case 3: sprayIndex = 10; break;
        case 4: sprayIndex =  7; break;
    }
    if (!this->rainSpraySprite) sprayIndex = 0;

    Vector endPos;
    pmtrace_t trace;

    const auto radians = this->actualWindYaw * M_PI / 180.0f;
    const auto windX = static_cast<float>(cos(radians)) * this->actualWindSpeed;
    const auto windY = static_cast<float>(sin(radians)) * this->actualWindSpeed;

    for (auto i = 0; i < numParticles; i++)
    {
        auto origin = this->weatherOrigin;

        origin.x += gEngfuncs.pfnRandomFloat(-400.0f, 400.0f);
        origin.y += gEngfuncs.pfnRandomFloat(-400.0f, 400.0f);
        origin.z += gEngfuncs.pfnRandomFloat(100.0f, 300.0f);

        endPos.x = origin.x + (gEngfuncs.pfnRandomLong(0, 5) > 2 ? g_vPlayerVelocity.x : -g_vPlayerVelocity.x);
        endPos.y = origin.y + g_vPlayerVelocity.y;
        endPos.z = 8000.0f;

        // Look up from the raindrop's origin and make sure there's sky above it
        gEngfuncs.pEventAPI->EV_SetTraceHull(2); // large hull
        gEngfuncs.pEventAPI->EV_PlayerTrace(origin, endPos, PM_WORLD_ONLY, -1, &trace);
        const char* pszTexture = gEngfuncs.pEventAPI->EV_TraceTexture(trace.ent, origin, trace.endpos);

        if (pszTexture && strncmp(pszTexture, "sky", 3) == 0)
        {
            CreateRaindrop(origin, windX, windY);

            if (sprayIndex > 0 && i % sprayIndex == 0)
            {
                // Create a rain spray particle
                auto sprayOrigin = Vector(origin.x, origin.y, gHUD.m_vecOrigin.z);
                if (gEngfuncs.pTriAPI->BoxInPVS(sprayOrigin, sprayOrigin))
                {
                    // Look down to find the ground
                    endPos.z = -8000.0f;
                    gEngfuncs.pEventAPI->EV_SetTraceHull(2); // large hull
                    gEngfuncs.pEventAPI->EV_PlayerTrace(sprayOrigin, endPos, PM_WORLD_ONLY, -1, &trace);

                    CreateRainSpray(trace.endpos, windX, windY);
                }
            }
        }
    }
}

void CWeather::UpdateSnow(float clientTime)
{
    nextPrecipitationUpdate = gEngfuncs.GetClientTime() + 0.7f;

    Vector endPos;
    pmtrace_t trace;

    const auto radians = this->actualWindYaw * M_PI / 180.0f;
    const auto windX = static_cast<float>(cos(radians)) * this->actualWindSpeed;
    const auto windY = static_cast<float>(sin(radians)) * this->actualWindSpeed;

    for (auto i = 0; i < numParticles; i++)
    {
        auto origin = this->weatherOrigin;

        origin.x += gEngfuncs.pfnRandomFloat(-300.0f, 300.0f);
        origin.y += gEngfuncs.pfnRandomFloat(-300.0f, 300.0f);
        origin.z += gEngfuncs.pfnRandomFloat(100.0f, 300.0f);

        endPos.x = origin.x + (gEngfuncs.pfnRandomLong(0, 5) > 2 ? g_vPlayerVelocity.x : -g_vPlayerVelocity.x);
        endPos.y = origin.y + g_vPlayerVelocity.y;
        endPos.z = 8000.0f;

        // Look up from the snowflake's origin and make sure there's sky above it
        gEngfuncs.pEventAPI->EV_SetTraceHull(2); // large hull
        gEngfuncs.pEventAPI->EV_PlayerTrace(origin, endPos, PM_WORLD_ONLY, -1, &trace);
        const char* pszTexture = gEngfuncs.pEventAPI->EV_TraceTexture(trace.ent, origin, trace.endpos);

        if (pszTexture && strncmp(pszTexture, "sky", 3) == 0)
        {
            CreateSnowflake(origin, windX, windY);
        }
    }
}

void CWeather::CreateRaindrop(const Vector& origin, const float windX, const float windY) const
{
    if (!this->rainSprite) return;

    auto* particle = new CRaindrop();
    if (!particle) return; // allocation failed

    particle->InitializeSprite(origin, Vector(0, 0, 0), rainSprite, 2.0f, 1.0f);
    strcpy(particle->m_szClassname, "particle_rain");

    particle->m_flStretchY = 40.0f;

    particle->m_vVelocity.x = windX * gEngfuncs.pfnRandomFloat(1.0f, 2.0f);
    particle->m_vVelocity.y = windY * gEngfuncs.pfnRandomFloat(1.0f, 2.0f);
    particle->m_vVelocity.z = gEngfuncs.pfnRandomFloat(-500.0f, -1800.0f);

    particle->m_flGravity = 0;

    particle->SetCollisionFlags(TRI_COLLIDEWORLD | TRI_COLLIDEKILL | TRI_WATERTRACE);
    particle->SetCullFlag(CULL_PVS | CULL_FRUSTUM_PLANE);
    particle->SetLightFlag(LIGHT_NONE);

    particle->m_iRendermode = kRenderTransAlpha;

    particle->m_vColor.x = particle->m_vColor.y = particle->m_vColor.z = 255.0f;

    particle->m_flDieTime = gEngfuncs.GetClientTime() + 1.0f;
}

void CWeather::CreateRainSpray(const Vector& origin, const float windX, const float windY) const
{
    if (!this->rainSpraySprite) return;

    auto* particle = new CRainSpray();
    if (!particle) return; // allocation failed

    auto vecPartOrigin = origin;
    vecPartOrigin.z += 10.0f;

    particle->InitializeSprite(vecPartOrigin, Vector(0, 0, 0), rainSpraySprite, gEngfuncs.pfnRandomFloat(50.0, 75.0), 1.0);
    strcpy(particle->m_szClassname, "wind_particle");

    particle->m_iNumFrames = rainSpraySprite->numframes;

    particle->m_vVelocity.x = windX / gEngfuncs.pfnRandomFloat(1.0f, 2.0f);
    particle->m_vVelocity.y = windY / gEngfuncs.pfnRandomFloat(1.0f, 2.0f);

    if (gEngfuncs.pfnRandomFloat(0.0, 1.0) < 0.1f)
    {
        particle->m_vVelocity.x *= 0.5f;
        particle->m_vVelocity.y *= 0.5f;
    }

    particle->m_flGravity = 0;

    particle->SetCollisionFlags(TRI_COLLIDEWORLD);
    particle->SetCullFlag(RENDER_FACEPLAYER | LIGHT_NONE | CULL_PVS | CULL_FRUSTUM_SPHERE);

    particle->m_iRendermode = kRenderTransAlpha;

    particle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat(-1.0, 1.0);

    particle->m_flScaleSpeed = 0.4f;
    particle->m_flDampingTime = 0;

    particle->m_iFrame = 0;

    particle->m_flMass = 1.0f;
    particle->m_flBounceFactor = 0;
    particle->m_vColor.x = particle->m_vColor.y = particle->m_vColor.z = 128.0f;

    particle->m_flFadeSpeed = -1.0f;

    particle->m_flDieTime = gEngfuncs.GetClientTime() + 6.0f;
}

void CWeather::CreateSnowflake(const Vector& origin, float windX, float windY) const
{
    if (!this->snowSprite) return;

    auto* pParticle = new CSnowflake();
    if (!pParticle) return; // allocation failed

    pParticle->InitializeSprite(origin, Vector(0, 0, 0), snowSprite, gEngfuncs.pfnRandomFloat(2.0, 2.5), 1.0);
    strcpy(pParticle->m_szClassname, "snow_particle");

    pParticle->m_iNumFrames = snowSprite->numframes;

    pParticle->m_vVelocity.x = windX / gEngfuncs.pfnRandomFloat(1.0, 2.0);
    pParticle->m_vVelocity.y = windY / gEngfuncs.pfnRandomFloat(1.0, 2.0);
    pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat(-100.0, -200.0);

    const auto flFrac = gEngfuncs.pfnRandomFloat(0.0, 1.0);

    if (flFrac >= 0.1f && flFrac < 0.2f)
    {
        pParticle->m_vVelocity.z = -65.0f;
    }
    else if (flFrac >= 0.1f && flFrac < 0.3f)
    {
        pParticle->m_vVelocity.z = -75.0f;
    }
    else
    {
        pParticle->m_vVelocity.x *= 0.5f;
        pParticle->m_vVelocity.y *= 0.5f;
    }

    pParticle->SetCollisionFlags(TRI_COLLIDEWORLD);
    pParticle->SetCullFlag(RENDER_FACEPLAYER | LIGHT_NONE | CULL_PVS | CULL_FRUSTUM_SPHERE);

    pParticle->m_iRendermode = kRenderTransAdd;

    pParticle->m_flScaleSpeed = 0;
    pParticle->m_flDampingTime = 0;
    pParticle->m_iFrame = 0;
    pParticle->m_flMass = 1.0;

    pParticle->m_flGravity = 0;
    pParticle->m_flBounceFactor = 0;

    pParticle->m_vColor.x = pParticle->m_vColor.y = pParticle->m_vColor.z = 128.0f;

    pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 3.0f;

    pParticle->m_bSpiral = gEngfuncs.pfnRandomLong(0, 1) != 0;

    pParticle->m_flSpiralTime = gEngfuncs.GetClientTime() + static_cast<float>(gEngfuncs.pfnRandomLong(2, 4));
}
