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

extern "C" 
{
	void DLLEXPORT CAM_Think( void );
	int DLLEXPORT CL_IsThirdPerson( void );
	void DLLEXPORT CL_CameraOffset( float *ofs );
}

int iMouseInUse = 0;

void DLLEXPORT CAM_Think( void )
{
}

void CAM_Init( void )
{
}

int DLLEXPORT CL_IsThirdPerson( void )
{
	return (gHUD.m_iCameraMode ? 1 : 0);
}

void DLLEXPORT CL_CameraOffset( float *ofs )
{
	VectorCopy( vec3_origin, ofs );
}
