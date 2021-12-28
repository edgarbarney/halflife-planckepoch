//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
// New clipping style camera - original idea by Xwider
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "in_defs.h"
#include "Exports.h"

bool iMouseInUse = false;

void DLLEXPORT CAM_Think()
{
}

void CAM_Init()
{
}

int DLLEXPORT CL_IsThirdPerson()
{
	return (gHUD.m_iCameraMode ? 1 : 0) || (g_iUser1 && (g_iUser2 == gEngfuncs.GetLocalPlayer()->index));
}

void DLLEXPORT CL_CameraOffset(float* ofs)
{
	VectorCopy(vec3_origin, ofs);
}
