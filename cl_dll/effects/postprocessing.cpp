#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "windows.h"

#include <string.h>
#include <memory.h>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include "postprocessing.h"

extern inline float sgn(float a);

void CPostProcessing::Init( void ) 
{
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
}

void CPostProcessing::VidInit( void ) 
{

}

void CPostProcessing::Restore( void ) 
{

}

void CPostProcessing::Draw(void)
{

}
