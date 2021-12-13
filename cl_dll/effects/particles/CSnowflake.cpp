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
#include "com_model.h"

#include "../CWeather.h"
#include "CSnowflake.h"

void CSnowflake::Think(float time)
{
	if (m_flBrightness < 130.0f && !m_bTouched) m_flBrightness += 4.5f;

	Fade(time);
	Spin(time);

	if (m_flSpiralTime <= gEngfuncs.GetClientTime())
	{
		m_bSpiral = !m_bSpiral;

		m_flSpiralTime = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomLong(2, 4);
	}
	else
	{
	}

	if (m_bSpiral && !m_bTouched)
	{
		const auto flDelta = time - g_Weather.GetLastClientTime();
		const auto flSpin = sin(time * 5.0f + reinterpret_cast<int>(this));

		m_vOrigin = m_vOrigin + m_vVelocity * flDelta;
        m_vOrigin.x += (flSpin * flSpin) * 0.3f;
	}
	else
	{
		CalculateVelocity(time);
	}

	CheckCollision(time);
}

void CSnowflake::Touch(Vector pos, Vector normal, int index)
{
	if (m_bTouched)
	{
		return;
	}

	m_bTouched = true;

	SetRenderFlag(RENDER_FACEPLAYER);

	m_flOriginalBrightness = m_flBrightness;

	m_vVelocity = Vector(0, 0, 0);

	m_iRendermode = kRenderTransAdd;

	m_flFadeSpeed = 0;
	m_flScaleSpeed = 0;
	m_flDampingTime = 0;
	m_iFrame = 0;
	m_flMass = 1.0;
	m_flGravity = 0;

	m_vColor.x = m_vColor.y = m_vColor.z = 128.0f;

	m_flDieTime = gEngfuncs.GetClientTime() + 0.5f;

	m_flTimeCreated = gEngfuncs.GetClientTime();
}
