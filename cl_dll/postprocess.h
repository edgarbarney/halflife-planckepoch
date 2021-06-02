#ifndef __POSTPROCESS_H
#define __POSTPROCESS_H

class CPostProcess
{
public:
	void Init();
	void Reset(bool stay = false);

	void InitScreenTexture();
	void MakeWeightsTexture();
	void DrawQuad(int width, int height, int ofsX, int ofsY);
	void ApplyPostEffects();
	int UseRectangleTextures();

	void CallTemporaryGrayscale(float startpower = 1.0f, float endpower = 1.0f, float gstime = 1.0f, bool stay = false, bool reset = false);

	bool m_bIsTemporaryActive; // Should we think now?

	float m_fGrayscalePower;

protected:
	float GetGrayscalePower();
	void TempGrayscaleThink();
	
	//TGS = Temporary Grayscale;
	//float m_fTGS_Time;
	float m_fTGS_StartTime;
	float m_fTGS_EndTime;
	float m_fTGS_StartPower;
	float m_fTGS_EndPower;
	bool m_bTGS_FadeOut;

	int m_iScreenTextureVal = 0;
	int m_iWeightsTextureVal = 0; //For GPUs with no register combiners support
	
};
extern CPostProcess gPostProcess;
#endif // __POSTPROCESS_H
