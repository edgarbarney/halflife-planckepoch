//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include<VGUI.h>
#include<VGUI_SurfaceBase.h>
#include<VGUI_Dar.h>

namespace vgui
{

class Panel;
class Cursor;

class VGUIAPI Surface : public SurfaceBase
{
public:
	Surface(Panel* embeddedPanel);
public:
	void setTitle(const char* title) override;
	bool setFullscreenMode(int wide,int tall,int bpp) override;
	void setWindowedMode() override;
	void setAsTopMost(bool state) override;
	int  getModeInfoCount() override;
	void createPopup(Panel* embeddedPanel) override;
	bool hasFocus() override;
	bool isWithin(int x,int y) override;
	void GetMousePos( int &x, int &y ) override;
protected:
	int  createNewTextureID(void) override;
	void drawSetColor(int r,int g,int b,int a) override;
	void drawFilledRect(int x0,int y0,int x1,int y1) override;
	void drawOutlinedRect(int x0,int y0,int x1,int y1) override;
	void drawSetTextFont(Font* font) override;
	void drawSetTextColor(int r,int g,int b,int a) override;
	void drawSetTextPos(int x,int y) override;
	void drawPrintText(const char* text,int textLen) override;
	void drawSetTextureRGBA(int id,const char* rgba,int wide,int tall) override;
	void drawSetTexture(int id) override;
	void drawTexturedRect(int x0,int y0,int x1,int y1) override;
	void invalidate(Panel *panel) override;
	virtual bool createPlat();
	virtual bool recreateContext();
	void enableMouseCapture(bool state) override;
	void setCursor(Cursor* cursor) override;
	void swapBuffers() override;
	void pushMakeCurrent(Panel* panel,bool useInsets) override;
	void popMakeCurrent(Panel* panel) override;
	void applyChanges() override;
protected:
	class SurfacePlat* _plat;
	bool               _needsSwap;
	Panel*             _embeddedPanel;
	Dar<char*>         _modeInfoDar;
	friend class App;
	friend class Panel;
};

}
