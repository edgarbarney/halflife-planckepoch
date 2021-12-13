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

#include "particleman.h"

class CSnowflake : public CBaseParticle
{
public:
    CSnowflake() = default;
    void Think(float time) override;
    void Touch(Vector pos, Vector normal, int index) override;

    bool m_bSpiral;
    float m_flSpiralTime;

private:
    bool m_bTouched = false;
};