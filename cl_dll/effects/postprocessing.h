#if !defined ( POSTPROCESSING_H )
#define POSTPROCESSING_H
#pragma once

#include "windows.h"

#define	MAX_PP_VERTEX_SHADERS	2
#define MAX_PP_FRAGMENT_SHADERS	4

class CPostProcessing
{
public:
	void	Init( void );
	void	VidInit( void );
	void	Restore( void );
	void	Draw( void );

public:
	GLuint			m_glVertexShaders[MAX_PP_VERTEX_SHADERS];
	GLuint			m_glFragmentShaders[MAX_PP_FRAGMENT_SHADERS];
};

extern CPostProcessing gWaterShader;
#endif