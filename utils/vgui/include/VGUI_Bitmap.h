//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include<VGUI.h>
#include<VGUI_Image.h>

namespace vgui
{

class Panel;

class VGUIAPI Bitmap : public Image
{
private:
	int         _id;
	bool        _uploaded;
public:
	Bitmap();
protected:
	void setSize(int wide,int tall) override;
	virtual void setRGBA(int x,int y,uchar r,uchar g,uchar b,uchar a);
public:
	void paint(Panel* panel) override;
protected:
	uchar* _rgba;
};

}
