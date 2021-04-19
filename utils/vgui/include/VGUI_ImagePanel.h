//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_IMAGEPANEL_H
#define VGUI_IMAGEPANEL_H

#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class Image;

class VGUIAPI ImagePanel : public Panel
{
public:
	inline ImagePanel()
	{
		_image=null;
	}

	ImagePanel(Image* image);
public:
	virtual void setImage(Image* image);
protected:
	void paintBackground() override;
protected:
	Image* _image;
};

}

#endif