/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012
Spirinity Rendering Engine - Copyright FranticDreamer 2020-2021

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

PostProcess Renderer
Original code by Buzer, DeathWish and Fragbait0.
Based on Spirit's glow and Paranoia's renderer
Re-purposed for Spirinity by FranticDreamer in 2021
Special Thanks to Admer456 of TWHL
*/
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "com_model.h"
#include "studio_util.h"
#include "bsprenderer.h"
#include "postprocess.h"
#include "FranUtils.hpp"

void CPostProcess::CallTemporaryGrayscale(float startpower, float endpower, float gstime, bool stay, bool reset)
{
	if (startpower != endpower || gstime > 0)
	{
		m_bTGS_Stay = stay;
		m_fGrayscalePower = m_fTGS_StartPower = startpower;

		m_fTGS_StartTime = gEngfuncs.GetClientTime();
		m_fTGS_EndPower = endpower;
	
		if (stay)
			m_fTGS_EndTime = 0;
		else
			m_fTGS_EndTime = gEngfuncs.GetClientTime() + gstime;

		m_bTGS_FadeOut = m_fTGS_StartPower < m_fTGS_EndPower;
		m_bIsTemporaryActive = true;
	}
}

void CPostProcess::TempGrayscaleThink()
{
	if (!m_bIsTemporaryActive)
		return;

	if (m_bTGS_FadeOut) // If fading out, startpower should be less than endpower
	{
		if (m_fGrayscalePower <= m_fTGS_EndPower)
		{
			m_fGrayscalePower = FranUtils::Lerp(FranUtils::WhereInBetween(gEngfuncs.GetClientTime(), m_fTGS_StartTime, m_fTGS_EndTime), m_fTGS_EndPower, m_fTGS_StartPower);
			if (m_fGrayscalePower > 1) m_fGrayscalePower = 1;
			if (m_fGrayscalePower < 0)
			{
				Reset(m_bTGS_Stay); // Reset
				gEngfuncs.Con_Printf("\n---Grayscale Fade Ended---\n");
			}
		}
	}
	else // If fading in, startpower should be greater than endpower
	{
		if (m_fGrayscalePower >= m_fTGS_EndPower)
		{
			m_fGrayscalePower = FranUtils::Lerp(FranUtils::WhereInBetween(gEngfuncs.GetClientTime(), m_fTGS_StartTime, m_fTGS_EndTime), m_fTGS_EndPower, m_fTGS_StartPower);
			if (m_fGrayscalePower < 0) m_fGrayscalePower = 0;
			if (m_fGrayscalePower > 1)
			{
				Reset(m_bTGS_Stay); // Reset
				gEngfuncs.Con_Printf("\n---Grayscale Fade Ended---\n");
			}
		} 
	}
}

float CPostProcess::GetGrayscalePower()
{
	if (!gBSPRenderer.m_pCvarPPGrayscale->value)
		return 0;

	float grayscale = m_fGrayscalePower;

	//TODO: Fix grayscale staying bug
	//if (m_fTGS_StartTime + 10.0f > m_fTGS_EndTime)
	//	m_fTGS_EndTime = m_fTGS_StartTime + 9.9f;

	if (grayscale > 1) grayscale = 1;

	//return 1; //debug

	return grayscale;
}

int CPostProcess::UseRectangleTextures()
{
	if (gBSPRenderer.m_bTexRectangeSupport &&
		gBSPRenderer.m_iTexRectangleSize >= ScreenWidth &&
		gBSPRenderer.m_iTexRectangleSize >= ScreenHeight)
		return true;

	return false;
}

void CPostProcess::Init()
{
	Reset();
}

void CPostProcess::Reset(bool stay)
{
	if (stay)
	{ 
		m_bIsTemporaryActive = false;
		m_fTGS_StartTime = 0;
		m_fTGS_EndTime = 0;
		m_fTGS_StartPower = 0;
		m_fTGS_EndPower = 0;
		//m_bTGS_FadeOut = false;
	}
	else
	{
		m_bIsTemporaryActive = false;
		m_fGrayscalePower = 0;
		m_fTGS_StartTime = 0;
		m_fTGS_EndTime = 0;
		m_fTGS_StartPower = 0;
		m_fTGS_EndPower = 0;
		m_bTGS_FadeOut = false;
	}
}

void CPostProcess::InitScreenTexture()
{
	if (m_iScreenTextureVal)
		return;

	if (!UseRectangleTextures())
	{
		// just create 256x256 texture
		unsigned char* pBlankTex = new unsigned char[256 * 256 * 3];
		memset(pBlankTex, 0, 256 * 256 * 3);
		int oldbinding;
		// GL START
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbinding);
		glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pBlankTex);
		glBindTexture(GL_TEXTURE_2D, oldbinding);
		// GL END
		delete[] pBlankTex;
		m_iScreenTextureVal = current_ext_texture_id;
		current_ext_texture_id++;
		gEngfuncs.Con_Printf("Grayscale: created 256x256 2D texture\n");
		return;
	}

	unsigned char* pBlankTex = new unsigned char[ScreenWidth * ScreenHeight * 3];
	memset(pBlankTex, 255, ScreenWidth * ScreenHeight * 3);

	// GL START
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, current_ext_texture_id);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 3, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pBlankTex);
	glDisable(GL_TEXTURE_RECTANGLE_NV);
	// GL END
	gEngfuncs.Con_Printf("Grayscale: created %dx%d rectangle texture\n", ScreenWidth, ScreenHeight);

	m_iScreenTextureVal = current_ext_texture_id;
	current_ext_texture_id++;
	delete[] pBlankTex;
}

void CPostProcess::MakeWeightsTexture()
{
	if (m_iWeightsTextureVal)
		return;

	unsigned char buf[16 * 16 * 3];
	for (int i = 0; i < (16 * 16); i++)
	{
		buf[3 * i + 0] = 168;
		buf[3 * i + 1] = 202;
		buf[3 * i + 2] = 139;
	}

	int oldbinding;

	// GL START
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbinding);
	glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
	glBindTexture(GL_TEXTURE_2D, oldbinding);
	// GL END

	m_iWeightsTextureVal = current_ext_texture_id;
	current_ext_texture_id++;
	gEngfuncs.Con_Printf("Grayscale: created weights texture\n");
}

void CPostProcess::DrawQuad(int width, int height, int ofsX = 0, int ofsY = 0)
{
	// GL START
	glTexCoord2f(ofsX, ofsY);
	glVertex3f(0, 1, -1);
	glTexCoord2f(ofsX, height + ofsY);
	glVertex3f(0, 0, -1);
	glTexCoord2f(width + ofsX, height + ofsY);
	glVertex3f(1, 0, -1);
	glTexCoord2f(width + ofsX, ofsY);
	glVertex3f(1, 1, -1);
	// GL END
}

void CPostProcess::ApplyPostEffects()
{
	TempGrayscaleThink(); //Think if temporary Grayscale is active

	// GL START
	if (!gBSPRenderer.m_bShaderSupport || !gBSPRenderer.m_pCvarPostProcessing->value)
		return;
	// GL END

	float grayscale = GetGrayscalePower();
	if (grayscale <= 0 || grayscale > 1)
		return;

	InitScreenTexture();
	int texenvmode1, texenvmode2, oldbinding1, oldbinding2;

	// GL START
	// setup ortho projection
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, 0.1, 100);

	int blendenabled = glIsEnabled(GL_BLEND);
	glEnable(GL_BLEND);
	glColor4f(1, 1, 1, grayscale);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// GL END

	//
	// Setup grayscale shader
	//
	if (gBSPRenderer.m_bNVCombinersSupport && !gBSPRenderer.m_pCvarSpecNoCombiners)
	{
		// GL START
		// use combiners
		GLfloat grayscale_weights[] = { 0.320000, 0.590000, 0.090000, 0.000000 };
		gBSPRenderer.glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);

		// RC 1 setup: 
		// spare0.rgb = dot(tex0.rgb, {0.32, 0.59, 0.09})
		gBSPRenderer.glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, grayscale_weights);
		gBSPRenderer.glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_CONSTANT_COLOR0_NV,
			GL_SIGNED_IDENTITY_NV, GL_RGB);
		gBSPRenderer.glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_TEXTURE0_ARB,
			GL_SIGNED_IDENTITY_NV, GL_RGB);
		gBSPRenderer.glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB,
			GL_SPARE0_NV,          // AB output
			GL_DISCARD_NV,         // CD output
			GL_DISCARD_NV,         // sum output
			GL_NONE, GL_NONE,
			GL_TRUE,               // AB = A dot B
			GL_FALSE, GL_FALSE);

		// Final RC setup:
		//	out.rgb = spare0.rgb
		//	out.a = spare0.a
		gBSPRenderer.glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gBSPRenderer.glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gBSPRenderer.glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gBSPRenderer.glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_SPARE0_NV,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gBSPRenderer.glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gBSPRenderer.glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gBSPRenderer.glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_PRIMARY_COLOR_NV,
			GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);

		glEnable(GL_REGISTER_COMBINERS_NV);
		// GL END
	}
	else
	{
		// use env_dot3
		MakeWeightsTexture();

		// GL START
		// 1st TU:
		// out = texture * 0.5 + 0.5
		gBSPRenderer.glActiveTextureARB(GL_TEXTURE0_ARB);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode1);

		GLfloat temp[] = { 0.5, 0.5, 0.5, 0.5 };
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, temp);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB); // {1, 1, 1}
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB); // {0.5, 0.5, 0.5}
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		// GL END

		// GL START
		// 2nd TU:
		// make dot3
		gBSPRenderer.glActiveTextureARB(GL_TEXTURE1_ARB);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode2);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGB_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbinding2);
		glBindTexture(GL_TEXTURE_2D, m_iWeightsTextureVal);

		gBSPRenderer.glActiveTextureARB(GL_TEXTURE0_ARB);
		// GL END
	}

	// GL START
	// copy screen to texture
	if (UseRectangleTextures())
	{
		
		glEnable(GL_TEXTURE_RECTANGLE_NV);
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_iScreenTextureVal);
		//	gl.glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);
		glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, ScreenWidth, ScreenHeight);

		glBegin(GL_QUADS);
		DrawQuad(ScreenWidth, ScreenHeight);
		glEnd();

		glDisable(GL_TEXTURE_RECTANGLE_NV);
	}
	else
	{
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbinding1);
		glBindTexture(GL_TEXTURE_2D, m_iScreenTextureVal);

		int ofsy2 = ScreenHeight;
		for (int ofsy = 0; ofsy < ScreenHeight; ofsy += 256, ofsy2 -= 256)
		{
			for (int ofsx = 0; ofsx < ScreenWidth; ofsx += 256)
			{
				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ofsx, ofsy, 256, 256);
				glBegin(GL_QUADS);
				glTexCoord2f(0, 0);
				glVertex3f(ofsx / (float)ScreenWidth, ofsy2 / (float)ScreenHeight, -1);

				glTexCoord2f(1, 0);
				glVertex3f((ofsx + 256) / (float)ScreenWidth, ofsy2 / (float)ScreenHeight, -1);

				glTexCoord2f(1, 1);
				glVertex3f((ofsx + 256) / (float)ScreenWidth, (ofsy2 - 256) / (float)ScreenHeight, -1);

				glTexCoord2f(0, 1);
				glVertex3f(ofsx / (float)ScreenWidth, (ofsy2 - 256) / (float)ScreenHeight, -1);
				glEnd();
			}
		}

		glBindTexture(GL_TEXTURE_2D, oldbinding1);
	}
	// GL END

	if (gBSPRenderer.m_bNVCombinersSupport && !gBSPRenderer.m_pCvarSpecNoCombiners)
	{
		glDisable(GL_REGISTER_COMBINERS_NV);
	}
	else
	{
		gBSPRenderer.glActiveTextureARB(GL_TEXTURE1_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode2);
		glBindTexture(GL_TEXTURE_2D, oldbinding2);
		glDisable(GL_TEXTURE_2D);
		gBSPRenderer.glActiveTextureARB(GL_TEXTURE0_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode1);
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	if (!blendenabled) glDisable(GL_BLEND);
}
