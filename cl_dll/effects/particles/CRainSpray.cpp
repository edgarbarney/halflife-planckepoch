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
#include "CRainSpray.h"

void CRainSpray::Think(float time)
{
    if (m_flDieTime - time <= 3.0f)
    {
        if (m_flBrightness > 0.0f)
        {
            m_flBrightness -= static_cast<float>(time - m_flTimeCreated) * 0.4f;
        }

        if (m_flBrightness < 0.0f)
        {
            m_flBrightness = 0;
            time = m_flDieTime = gEngfuncs.GetClientTime();
        }
    }
    else
    {
        if (m_flBrightness < 105.0f)
        {
            m_flBrightness += static_cast<float>(time - m_flTimeCreated) * 5.0f + 4.0f;
        }
    }

    CBaseParticle::Think(time);
}
