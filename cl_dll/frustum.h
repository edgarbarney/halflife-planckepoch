//==============================================================================//
//			Copyright (c) 2010 - 2011, Richard Roh·Ë, All rights reserved.		//
//==============================================================================//

#pragma once
#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "windows.h"
#include "gl/gl.h"
#include "pm_defs.h"
#include "cl_entity.h"
#include "com_model.h"
#include "ref_params.h"
#include "dlight.h"
#include "parsemsg.h"
#include "cvardef.h"

#define PITCH				0
#define YAW					1
#define ROLL				2

#define FARCLIP_OFF			0
#define	FARCLIP_DEPTH		1
#define FARCLIP_RADIAL		2

typedef float Q_Vector[3];

typedef struct Q_mplane_s
{
    Q_Vector	vNormal;
    float		flDist; 
    byte		type;
    byte		signbits;
    byte		pad[2];
} Q_mplane_t;

/*
===============
CFrustum

===============
*/
class FrustumCheck
{
public:
	void SetFrustum( Q_Vector vAngles, Q_Vector vOrigin, float flFOV, float flFarDist = 0, bool bView = false );
	bool CullBox( Q_Vector vMins, Q_Vector vMaxs );
	bool RadialCullBox( Q_Vector vMins, Q_Vector vMaxs );
	bool ExtraCullBox( Q_Vector vMins, Q_Vector vMaxs );

	void SetExtraCullBox( Q_Vector vMins, Q_Vector vMaxs );
	void DisableExtraCullBox( void );
	
	float CalcFov( float flFovX, float flWidth, float flHeight );
	void Q_AngleVectors( Q_Vector vAngles, Q_Vector vForward, Q_Vector vRight, Q_Vector vUp );
	int  Q_BoxOnPlaneSide( Q_Vector emins, Q_Vector emaxs, Q_mplane_t *p );
	int	 Q_SignbitsForPlane( Q_mplane_t *pOut );
	void Q_RotatePointAroundVector( Q_Vector vDest, const Q_Vector vDir, Q_Vector vPoint, float flDegrees );
	void Q_CrossProduct( Q_Vector v1, Q_Vector v2, Q_Vector cross );

private:
	Q_mplane_t	m_sFrustum[5];
	int		m_iFarClip;

	Q_Vector	m_vCullBoxMins;
	Q_Vector	m_vCullBoxMaxs;

	bool		m_bExtraCull;
	Q_Vector	m_vExtraCullMins;
	Q_Vector	m_vExtraCullMaxs;	
};
#endif