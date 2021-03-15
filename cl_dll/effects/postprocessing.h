#if !defined ( POSTPROCESSING_H )
#define POSTPROCESSING_H

#if defined( _WIN32 )
#pragma once
#endif

class CPostProcessing
{
public:
	void	Init(GLsizei w, GLsizei h);

	void	ChangeRes(GLsizei w, GLsizei h);
	void	Draw(void);
	void	UnDraw(void);
};

#endif