//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_BORDER_H
#define VGUI_BORDER_H

#include<VGUI.h>
#include<VGUI_Image.h>

//TODO: all borders should be titled

namespace vgui
{

class Panel;

class VGUIAPI Border : public Image
{
public:
	Border();
	Border(int left,int top,int right,int bottom);
public:
	virtual void setInset(int left,int top,int right,int bottom);
	virtual void getInset(int& left,int& top,int& right,int& bottom);
protected:
	void drawFilledRect(int x0,int y0,int x1,int y1) override;
	void drawOutlinedRect(int x0,int y0,int x1,int y1) override;
	void drawSetTextPos(int x,int y) override;
	void drawPrintText(int x,int y,const char* str,int strlen) override;
	void drawPrintChar(int x,int y,char ch) override;
protected:
	int _inset[4];
private:
	Panel* _panel;
friend class Panel;
friend class BorderPair;
};

}

#endif