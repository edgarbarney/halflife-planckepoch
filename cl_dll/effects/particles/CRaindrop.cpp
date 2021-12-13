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
#include "pm_defs.h"
#include "com_model.h"
#include "studio_util.h"

#include "../CWeather.h"
#include "CRaindrop.h"

void CRaindrop::Think(float time)
{
	Vector vecViewAngles;
	Vector vecForward, vecRight, vecUp;

	gEngfuncs.GetViewAngles(vecViewAngles);

	AngleVectors(vecViewAngles, vecForward, vecRight, vecUp);

	m_vAngles.y = vecViewAngles.y;
	m_vAngles.z = atan(DotProduct(m_vVelocity, vecRight) / m_vVelocity.z) * (180.0 / M_PI);

	if (m_flBrightness < 155.0f)
		m_flBrightness += 6.5f;

	CBaseParticle::Think(time);
}

void CRaindrop::Touch(Vector pos, Vector normal, int index)
{

	if (m_bTouched)
	{
		return;
	}

	m_bTouched = true;

	Vector vecStart = m_vOrigin;
	vecStart.z += 32.0f;

	pmtrace_t trace;

	{
		Vector vecEnd = m_vOrigin;
		vecEnd.z -= 16.0f;

		gEngfuncs.pEventAPI->EV_PlayerTrace(vecStart, vecEnd, PM_WORLD_ONLY, -1, &trace);
	}

	Vector vecNormal;

	vecNormal.x = normal.x;
	vecNormal.y = normal.y;
	vecNormal.z = -normal.z;

	Vector vecAngles;

	VectorAngles(vecNormal, vecAngles);

	if (gEngfuncs.PM_PointContents(trace.endpos, nullptr) == gEngfuncs.PM_PointContents(vecStart, nullptr))
	{
		CBaseParticle* pParticle = new CBaseParticle();
		if (!pParticle) return;

		model_t* pRainSplash = g_Weather.GetRainSplashModel();

		pParticle->InitializeSprite(m_vOrigin + normal, Vector(90.0f, 0.0f, 0.0f), pRainSplash, gEngfuncs.pfnRandomLong(20, 25), 125.0f);

		pParticle->m_iRendermode = kRenderTransAdd;

		pParticle->m_flMass = 1.0f;
		pParticle->m_flGravity = 0.1f;

		pParticle->SetCullFlag(LIGHT_INTENSITY | CULL_PVS | CULL_FRUSTUM_SPHERE);

		pParticle->m_vColor.x = pParticle->m_vColor.y = pParticle->m_vColor.z = 255.0f;

		pParticle->m_iNumFrames = pRainSplash->numframes - 1;
		pParticle->m_iFramerate = gEngfuncs.pfnRandomLong(30, 45);
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 0.3f;
		pParticle->SetCollisionFlags(TRI_ANIMATEDIE);
		pParticle->SetRenderFlag(RENDER_FACEPLAYER);
	}
	else
	{
		Vector vecBegin = vecStart;
		Vector vecEnd = trace.endpos;

		Vector vecDist = vecEnd - vecBegin;

		Vector vecHalf;

		Vector vecNewBegin;

		while (vecDist.Length() > 4.0)
		{
			vecDist = vecDist * 0.5;

			vecHalf = vecBegin + vecDist;

			if (gEngfuncs.PM_PointContents(vecBegin, nullptr) == gEngfuncs.PM_PointContents(vecHalf, nullptr))
			{
				vecBegin = vecHalf;
				vecNewBegin = vecHalf;
			}
			else
			{
				vecEnd = vecHalf;
				vecNewBegin = vecBegin;
			}

			vecDist = vecEnd - vecNewBegin;
		}

		CBaseParticle* pParticle = new CBaseParticle();
		if (!pParticle) return;

		pParticle->InitializeSprite(vecBegin, vecAngles, g_Weather.GetRippleModel(), 15.0f, 110.0f);

		pParticle->m_iRendermode = kRenderTransAdd;
		pParticle->m_flScaleSpeed = 1.0f;
		pParticle->m_vColor.x = pParticle->m_vColor.y = pParticle->m_vColor.z = 255.0f;
		pParticle->SetCullFlag(LIGHT_INTENSITY | CULL_PVS | CULL_FRUSTUM_SPHERE);
		pParticle->m_flFadeSpeed = 2.0f;
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
	}
}
