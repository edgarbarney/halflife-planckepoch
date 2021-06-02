#ifndef __POSTPROCESS_H
#define __POSTPROCESS_H

class CPostProcess
{
public:
	void Init();

	void InitScreenTexture();

	void MakeWeightsTexture();
	void DrawQuad(int width, int height, int ofsX, int ofsY);
	void ApplyPostEffects();
	
	int UseRectangleTextures();

	float m_fGrayscalePower;

protected:
	float GetGrayscalePower();

	int m_iScreenTextureVal = 0;
	int m_iWeightsTextureVal = 0; //For GPUs with no register combiners support
	
};
extern CPostProcess gPostProcess;
#endif // __POSTPROCESS_H
