//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_DEFAULTINPUTSIGNAL_H
#define VGUI_DEFAULTINPUTSIGNAL_H
#ifdef _WIN32
#pragma once
#endif


#include "VGUI_InputSignal.h"


namespace vgui
{
	// This class derives from vgui::InputSignal and implements empty defaults for all of its functions.
	class CDefaultInputSignal : public vgui::InputSignal
	{
	public:
		void cursorMoved(int x,int y,Panel* panel) override				{}
		void cursorEntered(Panel* panel) override						{}
		void cursorExited(Panel* panel) override							{}
		void mousePressed(MouseCode code,Panel* panel) override			{}
		void mouseDoublePressed(MouseCode code,Panel* panel) override	{}
		void mouseReleased(MouseCode code,Panel* panel) override			{}
		void mouseWheeled(int delta,Panel* panel) override				{}
		void keyPressed(KeyCode code,Panel* panel) override				{}
		void keyTyped(KeyCode code,Panel* panel) override				{}
		void keyReleased(KeyCode code,Panel* panel) override				{}
		void keyFocusTicked(Panel* panel) override						{}
	};
}


#endif // VGUI_DEFAULTINPUTSIGNAL_H
