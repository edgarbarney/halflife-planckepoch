/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012
Spirinity Rendering Engine - Copyright FranticDreamer 2020-2021

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Water Shader
Written by Andrew Lucas
*/

#if !defined ( WATERSHADER_H )
#define WATERSHADER_H
#if defined( _WIN32 )
#pragma once
#endif

#include "PlatformHeaders.h"
#include "gl/gl.h"
#include "pm_defs.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "dlight.h"
#include "parsemsg.h"
#include "cvardef.h"
#include "textureloader.h"
#include "rendererdefs.h"

/*
====================
CWaterShader

====================
*/
class CWaterShader
{
public:
	void	Init( );
	void	Shutdown();
	void	VidInit( );
	void	Restore( );
	void	ClearEntities();

	void	AddEntity( cl_entity_t *entity );
	void	DrawWater( );

	void	DrawWaterPasses( ref_params_t *pparams );
	void	DrawScene( ref_params_t *pparams, bool forcemodels );

	void	SetupRefract( );
	void	FinishRefract( );

	void	SetupReflect( );
	void	FinishReflect( );

	void	SetupClipping( ref_params_t *pparams, bool isrefracting );
	void	LoadScript( );

	bool	ViewInWater( );
	bool	ShouldReflect( int index );

	Vector	GetWaterOrigin(cl_water_t* pwater = nullptr);

public:
	bool			m_bViewInWater;
	Vector			m_vViewOrigin;

	cl_water_t		m_pWaterEntities[MAX_WATER_ENTITIES];
	int				m_iNumWaterEntities;

	cvar_t			*m_pCvarWaterShader;
	cvar_t			*m_pCvarWaterDebug;
	cvar_t			*m_pCvarWaterResolution;

	cl_texture_t	*m_pNormalTexture;
	cl_water_t		*m_pCurWater;

	ref_params_t	*m_pViewParams;
	ref_params_t	m_pWaterParams;

	Vector			m_vWaterOrigin;
	Vector			m_vWaterPlaneMins;
	Vector			m_vWaterPlaneMaxs;
	Vector			m_vWaterEntMins;
	Vector			m_vWaterEntMaxs;

	int				m_iNumPasses;
public:
	GLuint			m_uiVertexPrograms[MAX_WATER_VERTEX_SHADERS];
	GLuint			m_uiFragmentPrograms[MAX_WATER_FRAGMENT_SHADERS];

public:
	fog_settings_t	m_pMainFogSettings;
	fog_settings_t	m_pWaterFogSettings;

	float			m_flFresnelTerm;
};

extern CWaterShader gWaterShader;
#endif