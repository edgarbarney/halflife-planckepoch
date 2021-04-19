/***
*
* NEW file for the Mod "Half-Life: Death"
* now also used in Spirit of Half-Life 1.4
* Created 22/8/04
*
***/
/*

===== glow.cpp ========================================================

DeathWish's tron glow effect in shaders and low-end.
Merged and given some nicer CVARs by FragBait0, tlevi@tpg.com.au
*/

//From tri.cpp...
#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "particlemgr.h"
//End tri.cpp

#include <gl/gl.h>
#include <gl/glext.h>
#include <cg/cg.h>
#include <cg/cgGL.h>
#include "r_studioint.h"

#define DLLEXPORT __declspec( dllexport )
#define GL_TEXTURE_RECTANGLENV 0x84F5

// START glow (shader+lowend) -- FragBait0
extern engine_studio_api_t IEngineStudio;

bool bGlowShaderInitialised = false;
bool bGlowLowEndInitialised = false;

#ifdef _WIN32
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = nullptr;
#endif

CGcontext g_cgContext;
CGprofile g_cgVertProfile;
CGprofile g_cgFragProfile;

CGprogram g_cgVP_GlowDarken;
CGprogram g_cgFP_GlowDarken;

CGprogram g_cgVP_GlowBlur;
CGprogram g_cgFP_GlowBlur;

CGprogram g_cgVP_GlowCombine;
CGprogram g_cgFP_GlowCombine;

CGparameter g_cgpVP0_ModelViewMatrix;
CGparameter g_cgpVP1_ModelViewMatrix;
CGparameter g_cgpVP1_XOffset;
CGparameter g_cgpVP1_YOffset;
CGparameter g_cgpVP2_ModelViewMatrix;

unsigned int g_uiSceneTex;
unsigned int g_uiBlurTex;

// TEXTURES
unsigned int g_uiScreenTex = 0;
unsigned int g_uiGlowTex = 0;

// FUNCTIONS
void InitScreenGlow();
void InitScreenGlowShader();
void InitScreenGlowLowEnd();
void RenderScreenGlow();
void RenderScreenGlowShader();
void RenderScreenGlowLowEnd();
bool LoadProgram(CGprogram* pDest, CGprofile profile, const char* szFile);


void DrawQuad(int width, int height, int ofsX = 0, int ofsY = 0)
{
     glBegin(GL_QUADS);

     glTexCoord2f(ofsX,ofsY);
     glVertex3f(0, 1, -1);
     glTexCoord2f(ofsX,height+ofsY);
     glVertex3f(0, 0, -1);
     glTexCoord2f(width+ofsX,height+ofsY);
     glVertex3f(1, 0, -1);
     glTexCoord2f(width+ofsX,ofsY);
     glVertex3f(1, 1, -1);

     glEnd();
}

void RenderScreenGlow()
{
 	if (IEngineStudio.IsHardware() != 1)
	    return;

	if (CVAR_GET_FLOAT("r_glow") == 0)	 //check the cvar for the glow is on.
		return;

	if (CVAR_GET_FLOAT("r_glow") == 1){	//check the mode is shader
		RenderScreenGlowShader();	//AJH don't need r_glowmode, use r_glow 0/1/2 instead
	}

	else if (CVAR_GET_FLOAT("r_glow") == 2){ //If its not a shader its the lowend
		RenderScreenGlowLowEnd();	//AJH don't need r_glowmode, use r_glow 0/1/2 instead
	}
}

void InitScreenGlow()
{
	if (IEngineStudio.IsHardware() != 1)
		return;

	if (CVAR_GET_FLOAT("r_glow") == 0)
		return;

	if (CVAR_GET_FLOAT("r_glow") == 1){
		InitScreenGlowShader();	//AJH don't need r_glowmode, use r_glow 0/1/2 instead

	}
	else if (CVAR_GET_FLOAT("r_glow") == 2){
		InitScreenGlowLowEnd();	//AJH don't need r_glowmode, use r_glow 0/1/2 instead
	}
}
//END glow effect functions -- FragBait0

//START shader glow effect -- FragBait0
inline bool LoadProgram(CGprogram* pDest, CGprofile profile, const char* szFile)
{
     const char* szGameDir = gEngfuncs.pfnGetGameDirectory();
     char file[512];
     sprintf(file, "%s/%s", szGameDir, szFile);

     *pDest = cgCreateProgramFromFile(g_cgContext, CG_SOURCE, file, profile, "main", nullptr);
     if (!(*pDest)) {
          CONPRINT(cgGetErrorString(cgGetError()));
          CONPRINT("\n");
          return false;
     }

     cgGLLoadProgram(*pDest);

     return true;
}

void InitScreenGlowShader()
{
	 bGlowShaderInitialised = false;
    
#ifdef _WIN32
     // OPENGL EXTENSION LOADING
     glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
#endif

     // TEXTURE CREATION
     unsigned char* pBlankTex = new unsigned char[ScreenWidth*ScreenHeight*3];
     memset(pBlankTex, 0, ScreenWidth*ScreenHeight*3);

     glGenTextures(1, &g_uiSceneTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiSceneTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth, ScreenHeight, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

     glGenTextures(1, &g_uiBlurTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiBlurTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth/4, ScreenHeight/4, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

     delete[] pBlankTex;

     // CG INITIALISATION
     g_cgContext = cgCreateContext();
     if (!g_cgContext) {
          CONPRINT("Couldn't make Cg context\n");
          return;
     }

     // VERTEX PROFILE
     g_cgVertProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
     if (g_cgVertProfile == CG_PROFILE_UNKNOWN) {
          CONPRINT("Couldn't fetch valid VP profile\n");
          return;
     }

     cgGLSetOptimalOptions(g_cgVertProfile);

     // VP LOADING
     if (!LoadProgram(&g_cgVP_GlowDarken, g_cgVertProfile, "cgshaders/glow_darken_vp.cg"))
          return;

     if (!LoadProgram(&g_cgVP_GlowBlur, g_cgVertProfile, "cgshaders/glow_blur_vp.cg"))
          return;

     if (!LoadProgram(&g_cgVP_GlowCombine, g_cgVertProfile, "cgshaders/glow_combine_vp.cg"))
          return;

     // VP PARAM GRABBING
     g_cgpVP0_ModelViewMatrix = cgGetNamedParameter(g_cgVP_GlowDarken, "ModelViewProj");

     g_cgpVP1_ModelViewMatrix = cgGetNamedParameter(g_cgVP_GlowBlur, "ModelViewProj");
     g_cgpVP1_XOffset = cgGetNamedParameter(g_cgVP_GlowBlur, "XOffset");
     g_cgpVP1_YOffset = cgGetNamedParameter(g_cgVP_GlowBlur, "YOffset");

     g_cgpVP2_ModelViewMatrix = cgGetNamedParameter(g_cgVP_GlowCombine, "ModelViewProj");

     // FRAGMENT PROFILE
     g_cgFragProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
     if (g_cgFragProfile == CG_PROFILE_UNKNOWN) {
          CONPRINT("Couldn't fetch valid FP profile\n");
          return;
     }

     cgGLSetOptimalOptions(g_cgFragProfile);

     // FP LOADING
     if (!LoadProgram(&g_cgFP_GlowDarken, g_cgFragProfile, "cgshaders/glow_darken_fp.cg"))
          return;

     if (!LoadProgram(&g_cgFP_GlowBlur, g_cgFragProfile, "cgshaders/glow_blur_fp.cg"))
          return;

     if (!LoadProgram(&g_cgFP_GlowCombine, g_cgFragProfile, "cgshaders/glow_combine_fp.cg"))
          return;

	 bGlowShaderInitialised = true;
}

void DoBlur(unsigned int uiSrcTex, unsigned int uiTargetTex, int srcTexWidth, int srcTexHeight, int destTexWidth, int destTexHeight, float xofs, float yofs)
{
	cgGLBindProgram(g_cgVP_GlowBlur);
	cgGLBindProgram(g_cgFP_GlowBlur);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, uiSrcTex);

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, uiSrcTex);

	glActiveTextureARB(GL_TEXTURE2_ARB);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, uiSrcTex);

	glActiveTextureARB(GL_TEXTURE3_ARB);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, uiSrcTex);

	cgGLSetParameter1f(g_cgpVP1_XOffset, xofs);
	cgGLSetParameter1f(g_cgpVP1_YOffset, yofs);

	glViewport(0, 0, destTexWidth, destTexHeight);

	DrawQuad(srcTexWidth, srcTexHeight);

	glBindTexture(GL_TEXTURE_RECTANGLE_NV, uiTargetTex);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, destTexWidth, destTexHeight, 0);
}

void RenderScreenGlowShader()
{
	// check to see if we can render it.
	if (IEngineStudio.IsHardware() != 1){
		return;
	}
/*	AJH - This is redundant as the function now returns if r_glow!=1
	//check the cvar for the glow is on.
    if (CVAR_GET_FLOAT("r_glow") == 0)
         return;
*/
	//if the mode isn't shader then return.
	if (CVAR_GET_FLOAT("r_glow") != 1)
		  return;

    if (!bGlowShaderInitialised)
		InitScreenGlowShader();

	if (!bGlowShaderInitialised){ //if the initialisation dosent work for some reason
		gEngfuncs.Cvar_SetValue( "r_glow", 0 );
		return;
	}

    // STEP 1: Grab the screen and put it into a texture
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_RECTANGLE_NV);

    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiSceneTex);
    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

    // STEP 2: Set up an orthogonal projection
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, 0.1, 100);

    glColor3f(1,1,1);

    // STEP 3: Initialize Cg programs and parameters for darken
    cgGLEnableProfile(g_cgVertProfile);
    cgGLEnableProfile(g_cgFragProfile);

    cgGLBindProgram(g_cgVP_GlowDarken);
    cgGLBindProgram(g_cgFP_GlowDarken);

    cgGLSetStateMatrixParameter(g_cgpVP0_ModelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

    // STEP 4: Render the current scene texture to a new, lower-res texture, darkening non-bright areas of the scene
    glViewport(0, 0, ScreenWidth/4, ScreenHeight/4);
    glActiveTextureARB(GL_TEXTURE0_ARB);

    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiSceneTex);

    DrawQuad(ScreenWidth, ScreenHeight);

    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiBlurTex);
    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth/4, ScreenHeight/4, 0);

	// STEP 5: Initialise Cg programs and parameters for blurring
	cgGLBindProgram(g_cgVP_GlowBlur);
	cgGLBindProgram(g_cgFP_GlowBlur);

	cgGLSetStateMatrixParameter(g_cgpVP1_ModelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

    // STEP 6: Apply blur
    int iNumBlurSteps = CVAR_GET_FLOAT("r_glowblur"); //use new CVAR --FragBait0
    for (int i = 0; i < iNumBlurSteps; i++) {
         DoBlur(g_uiBlurTex, g_uiBlurTex, ScreenWidth/4, ScreenHeight/4, ScreenWidth/4, ScreenHeight/4, 1, 0);
         DoBlur(g_uiBlurTex, g_uiBlurTex, ScreenWidth/4, ScreenHeight/4, ScreenWidth/4, ScreenHeight/4, 0, 1);
    }

    // STEP 7: Set up Cg for combining blurred glow with original scene
    cgGLBindProgram(g_cgVP_GlowCombine);
    cgGLBindProgram(g_cgFP_GlowCombine);

    cgGLSetStateMatrixParameter(g_cgpVP2_ModelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiSceneTex);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiBlurTex);

    // STEP 8: Do the combination, rendering to the screen without grabbing it to a texture
    glViewport(0, 0, ScreenWidth, ScreenHeight);

	DrawQuad(ScreenWidth/4, ScreenHeight/4);

    // STEP 9: Restore the original projection and modelview matrices and disable rectangular textures on all units
	glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    cgGLDisableProfile(g_cgVertProfile);
    cgGLDisableProfile(g_cgFragProfile);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_NV);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_NV);

    glActiveTextureARB(GL_TEXTURE2_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_NV);

    glActiveTextureARB(GL_TEXTURE3_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_NV);

    glActiveTextureARB(GL_TEXTURE0_ARB);
	glClear(GL_DEPTH_BUFFER_BIT);
}
//END shader glow effect --FragBait0

//START lowend glow effect --FragBait0
void InitScreenGlowLowEnd()
{
     bGlowLowEndInitialised = false;

	// create a load of blank pixels to create textures with
     unsigned char* pBlankTex = new unsigned char[ScreenWidth*ScreenHeight*3];
     memset(pBlankTex, 0, ScreenWidth*ScreenHeight*3);

     // Create the SCREEN-HOLDING TEXTURE
     glGenTextures(1, &g_uiScreenTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiScreenTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth, ScreenHeight, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

	 // Create the BLURRED TEXTURE
     glGenTextures(1, &g_uiGlowTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiGlowTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth/4, ScreenHeight/4, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

     // free the memory
     delete[] pBlankTex;

     bGlowLowEndInitialised = true;
}

void RenderScreenGlowLowEnd()
{

     // check to see if we can render it.
	if (IEngineStudio.IsHardware() != 1){
		return;
	}

/*	AJH - This is redundant as the function now returns if r_glow!=2
	//check the cvar for the glow is on.
     if (CVAR_GET_FLOAT("r_glow") == 0)
          return;
*/
	 //check the mode is low-end
	if (CVAR_GET_FLOAT("r_glow") != 2)	//AJH don't need r_glowmode, use r_glow 0/1/2 instead
		return;

	if (!bGlowLowEndInitialised)
		InitScreenGlowLowEnd();

	if (!bGlowLowEndInitialised){ //if the initialisation dosent work for some reason
		gEngfuncs.Cvar_SetValue( "r_glow", 0 ); //AJH may as well set this here too.
		return;
	}
    // enable some OpenGL stuff
    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glColor3f(1,1,1);
    glDisable(GL_DEPTH_TEST);

    // STEP 1: Grab the screen and put it into a texture
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiScreenTex);
    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

    // STEP 2: Set up an orthogonal projection
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, 0.1, 100);

	// STEP 3: Render the current scene to a new, lower-res texture, darkening non-bright areas of the scene
    // by multiplying it with itself a few times.
    glViewport(0, 0, ScreenWidth/4, ScreenHeight/4);

    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiScreenTex);

    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    glDisable(GL_BLEND);

    DrawQuad(ScreenWidth, ScreenHeight);

    glEnable(GL_BLEND);

    int i;
    for (i = 0; i < CVAR_GET_FLOAT("r_glowdark"); i++)
		DrawQuad(ScreenWidth, ScreenHeight);

    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiGlowTex);
    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth/4, ScreenHeight/4, 0);

    // STEP 4: Blur the now darkened scene in the horizontal direction.
    float blurAlpha = 1 / (CVAR_GET_FLOAT("r_glowblur")*2 + 1);

    glColor4f(1,1,1,blurAlpha);

    glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

    DrawQuad(ScreenWidth/4, ScreenHeight/4);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (i = 0; i <= CVAR_GET_FLOAT("r_glowblur"); i++) {
	    DrawQuad(ScreenWidth/4, ScreenHeight/4, -i, 0);
		DrawQuad(ScreenWidth/4, ScreenHeight/4, i, 0);
    }

    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth/4, ScreenHeight/4, 0);

    // STEP 5: Blur the horizontally blurred image in the vertical direction.
    glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

    DrawQuad(ScreenWidth/4, ScreenHeight/4);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (i = 0; i <= CVAR_GET_FLOAT("r_glowblur"); i++) {
		DrawQuad(ScreenWidth/4, ScreenHeight/4, 0, -i);
		DrawQuad(ScreenWidth/4, ScreenHeight/4, 0, i);
    }

    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth/4, ScreenHeight/4, 0);

    // STEP 6: Combine the blur with the original image.
    glViewport(0, 0, ScreenWidth, ScreenHeight);

    glDisable(GL_BLEND);

    DrawQuad(ScreenWidth/4, ScreenHeight/4);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    for (i = 0; i < CVAR_GET_FLOAT("r_glowstrength"); i++) {
		DrawQuad(ScreenWidth/4, ScreenHeight/4);
    }

    glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiScreenTex);

    DrawQuad(ScreenWidth, ScreenHeight);

    // STEP 7: Restore the original projection and modelview matrices and disable rectangular textures.
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_TEXTURE_RECTANGLE_NV);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
	glClear(GL_DEPTH_BUFFER_BIT);
}
//END lowend glow effect --FragBait0