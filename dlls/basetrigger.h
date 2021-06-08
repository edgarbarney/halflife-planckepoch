#ifndef BASETRIGGER_H
#define BASETRIGGER_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CBaseTrigger : public CBaseToggle
{
public:
	//LRC - this was very bloated. I moved lots of methods into the
	// subclasses where they belonged.
	void InitTrigger();
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	BOOL CanTouch(entvars_t* pevToucher);

	int	ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

#endif