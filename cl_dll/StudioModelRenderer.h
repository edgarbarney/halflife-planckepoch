/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Model Renderer
Original code by Valve
Additional code written by Andrew Lucas
Transparency code by Neil "Jed" Jedrzejewski
*/

#if !defined ( STUDIOMODELRENDERER_H )
#define STUDIOMODELRENDERER_H
#if defined( _WIN32 )
#pragma once
#endif

#include "windows.h"
#include "gl/gl.h"
#include "gl/glext.h"
#include "dlight.h"

#include "rendererdefs.h"

#define MAX_FRAGMENT_SHADERS 2

/*
====================
CStudioModelRenderer

====================
*/
class CStudioModelRenderer
{
public:
	// Construction/Destruction
	CStudioModelRenderer( );
	virtual ~CStudioModelRenderer( );

	// Initialization
	virtual void Init( );
	virtual void VidInit( );

public:  
	// Public Interfaces
	virtual int StudioDrawModel ( int flags );
	virtual int StudioDrawPlayer ( int flags, struct entity_state_s *pplayer );

public:
	// Local interfaces
	//

	// Look up animation data for sequence
	virtual mstudioanim_t *StudioGetAnim ( model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc );

	// Interpolate model position and angles and set up matrices
	virtual void StudioSetUpTransform ( int trivial_accept );

	// Set up model bone positions
	virtual void StudioSetupBones ( );	

	// Find final attachment points
	virtual void StudioCalcAttachments ( );

	// Determine interpolation fraction
	virtual float StudioEstimateInterpolant( );

	// Determine current frame for rendering
	virtual float StudioEstimateFrame ( mstudioseqdesc_t *pseqdesc );

	// Apply special effects to transform matrix
	virtual void StudioFxTransform( cl_entity_t *ent, float transform[3][4] );

	// Spherical interpolation of bones
	virtual void StudioSlerpBones ( vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s );

	// Compute bone adjustments ( bone controllers )
	virtual void StudioCalcBoneAdj ( float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen );

	// Get bone quaternions
	virtual void StudioCalcBoneQuaterion ( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q );

	// Get bone positions
	virtual void StudioCalcBonePosition ( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos );

	// Compute rotations
	virtual void StudioCalcRotations ( float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f );

	// Send bones and verts to renderer
	virtual void StudioRenderModel ( );

	// Finalize rendering
	virtual void StudioRenderFinal ();

	virtual void StudioSaveBones( );
	virtual void StudioMergeBones ( model_t *m_pSubModel );

	// Player specific data
	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend ( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch );

	// Estimate gait frame for player
	virtual void StudioEstimateGait ( entity_state_t *pplayer );

	// Process movement of player
	virtual void StudioProcessGait ( entity_state_t *pplayer );

public:

	// Client clock
	double			m_clTime;				
	// Old Client clock
	double			m_clOldTime;			

	// Do interpolation?
	int				m_fDoInterp;			
	// Do gait estimation?
	int				m_fGaitEstimation;		

	// Current render frame #
	int				m_nFrameCount;

	// Cvars that studio model code needs to reference
	//
	// Use high quality models?
	cvar_t			*m_pCvarHiModels;	
	// Developer debug output desired?
	cvar_t			*m_pCvarDeveloper;
	// Draw entities bone hit boxes, etc?
	cvar_t			*m_pCvarDrawEntities;

	// The entity which we are currently rendering.
	cl_entity_t		*m_pCurrentEntity;		

	// The model for the entity being rendered
	model_t			*m_pRenderModel;

	// Player info for current player, if drawing a player
	player_info_t	*m_pPlayerInfo;

	// The index of the player being drawn
	int				m_nPlayerIndex;

	// The player's gait movement
	float			m_flGaitMovement;

	// Pointer to header block for studio model data
	studiohdr_t		*m_pStudioHeader;
	
	// Pointers to current body part and submodel
	mstudiobodyparts_t *m_pBodyPart;
	mstudiomodel_t	*m_pSubModel;

	// Palette substition for top and bottom of model
	int				m_nTopColor;			
	int				m_nBottomColor;

	//
	// Sprite model used for drawing studio model chrome
	model_t			*m_pChromeSprite;

	// Current view vectors and render origin
	float			m_vUp[ 3 ];
	float			m_vRight[ 3 ];
	float			m_vNormal[ 3 ];

	float			m_vRenderOrigin[ 3 ];
	
	// Model render counters ( from engine )
	int				*m_pStudioModelCount;
	int				*m_pModelsDrawn;

	// Matrices
	// Model to world transformation
	float			(*m_protationmatrix)[ 3 ][ 4 ];	
	// Model to view transformation
	float			(*m_paliastransform)[ 3 ][ 4 ];	

	// Concatenated bone and light transforms
	float			(*m_pbonetransform) [ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float			(*m_plighttransform) [ MAXSTUDIOBONES ][ 3 ][ 4 ];

	// Caching
	// Number of bones in bone cache
	int				m_nCachedBones; 
	// Names of cached bones
	char			m_nCachedBoneNames[ MAXSTUDIOBONES ][ 32 ];
	// Cached bone & light transformation matrices
	float			m_rgCachedBoneTransform [ MAXSTUDIOBONES ][ 3 ][ 4 ];

public:
	virtual void	StudioSetupModel( int bodypart );
	virtual void	StudioDrawPoints( );
	virtual void	StudioDrawMesh( mstudiomesh_t *pmesh, mstudiotexture_t *ptex );
	virtual void	StudioDrawWireframe( );

	virtual void	StudioSetupTextureHeader( );
	virtual void	StudioSetupRenderer( int rendermode );
	virtual void	StudioRestoreRenderer( );
	virtual qboolean	StudioCheckBBox( );

	virtual void	StudioEntityLight( );
	virtual bool	StudioCullBBox( const Vector &mins, const Vector &maxs );

	virtual void	StudioSetupLighting( );
	virtual int		StudioRecursiveLightPoint( entextrainfo_t *ext, mnode_t *node, const Vector &start, const Vector &end, Vector &color );
	
	virtual void	StudioSetTextureFlags( );
	virtual void	StudioSetChromeVectors( );
	virtual void	StudioChromeForMesh( int j, mstudiomesh_t *pmesh );

	virtual void	StudioSwapEngineCache( );

	virtual entextrainfo_t *StudioAllocExtraInfo( );

	virtual void	StudioDrawBBox( );
	virtual void	StudioDrawModelSolid( );
	virtual void	StudioDrawPointsSolid( );

	float			m_fChrome[MAXSTUDIOVERTS][2];
	Vector			m_vChromeUp[MAXSTUDIOBONES];
	Vector			m_vChromeRight[MAXSTUDIOBONES];

	studiohdr_t		*m_pTextureHeader;

	Vector			m_vMins;
	Vector			m_vMaxs;

	Vector			m_vVertexTransform[MAXSTUDIOVERTS];	// transformed vertices
	Vector			m_vNormalTransform[MAXSTUDIOVERTS]; // transformed normals

	Vector			*m_pVertexTransform; // pointer to vertex transform
	Vector			*m_pNormalTransform; // pointer to normal transform

	lighting_ext	m_pLighting; // buz

	mlight_t		*m_pModelLights[MAX_MODEL_LIGHTS];
	int				m_iNumModelLights;

	entextrainfo_t	m_pExtraInfo[MAXRENDERENTS];
	int				m_iNumExtraInfo;

	float			m_fAlpha;

	bool			m_bUseBlending;
	bool			m_bExternalEntity;
	bool			m_bChromeShell;

	int				m_iCurrentBinding;
	int				m_iEngineBinding;

	GLuint			m_uiVertexShaders[MAX_MODEL_SHADERS];
	GLuint			m_uiFragmentShaders[MAX_FRAGMENT_SHADERS];

	cvar_t			*m_pCvarDrawModels;
	cvar_t			*m_pCvarModelsBBoxDebug;
	cvar_t			*m_pCvarModelsLightDebug;
	cvar_t			*m_pCvarModelShaders;
	cvar_t			*m_pCvarModelDecals;
	cvar_t			*m_pCvarGlowShellFreq;

	cvar_t			*m_pCvarSkyVecX;
	cvar_t			*m_pCvarSkyVecY;
	cvar_t			*m_pCvarSkyVecZ;

	cvar_t			*m_pCvarSkyColorX;
	cvar_t			*m_pCvarSkyColorY;
	cvar_t			*m_pCvarSkyColorZ;

public:
	virtual void	StudioDrawExternalEntity( cl_entity_t *pEntity );
	virtual void	StudioRenderModelEXT( );
	virtual void	StudioDrawPointsEXT( );
	virtual void	StudioDrawMeshEXT( mstudiotexture_t *ptex, vbomesh_t *pmesh );
	virtual void	StudioDrawWireframeEXT( );
	
	virtual void	StudioDrawExternalEntitySolid( cl_entity_t *pEntity );
	virtual void	StudioDrawPointsSolidEXT( );

	virtual void	StudioSaveModelData( modeldata_t *pExtraData );
	virtual void	StudioSaveUniqueData( entextradata_t *pExtraData );
	virtual void	StudioManageVertex( studiovert_t *pvert );

	vboheader_t		*m_pVBOHeader;
	vbosubmodel_t	*m_pVBOSubModel;

	int				m_iNumEngineCacheModels;
public:
	virtual model_t	*Mod_LoadModel( char *szName );
	virtual void	Mod_LoadTexture( mstudiotexture_t *ptexture, byte *pbuffer, char *szmodelname );

	model_t			m_pStudioModels[MAX_CACHE_MODELS];
	int				m_iNumStudioModels;
	
	studiovert_t	m_pRefArray[65535];
	int				m_iNumRefVerts;

	brushvertex_t	m_pVBOVerts[65535];
	int				m_iNumVBOVerts;

	unsigned int	m_usIndexes[65535];
	int				m_iNumIndexes;
	int				m_iCurStart;

public:
	virtual void	StudioDrawDecals( );
	virtual studiodecal_t *StudioAllocDecal( );
	virtual studiodecal_t *StudioAllocDecalSlot( );

	virtual void	StudioDecalExternal( Vector vpos, Vector vnorm, const char *name );
	virtual void	StudioDecalForEntity( Vector position, Vector normal, const char *szName, cl_entity_t *pEntity );
	virtual void	StudioDecalForSubModel( Vector position, Vector normal, studiodecal_t *decal );
	virtual void	StudioDecalTriangle( studiotri_t *tri, Vector position, Vector normal, studiodecal_t *decal );

	studiodecal_t	m_pStudioDecals[MAX_CUSTOMDECALS];
	int				m_iNumStudioDecals;
};

#endif // STUDIOMODELRENDERER_H