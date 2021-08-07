/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012
Spirinity Rendering Engine - Copyright FranticDreamer 2020-2021

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Bone Utilities
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "studio.h"

#ifndef ACTIVITY_H
#include "activity.h"
#endif

#ifndef ANIMATION_H
#include "animation.h"
#endif

#ifndef SCRIPTEVENT_H
#include "scriptevent.h"
#endif

//RENDERERS START
void GetBoneNames(entvars_t* callerPev, std::vector<char*>& bones)
{
	void* pmodel = GET_MODEL_PTR(ENT(callerPev));

	//bool isHitboxFound[8];

	studiohdr_t* pstudiohdr;

	pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
	{
		ALERT(at_error, "Bad header in GetBoneNames!\n");
		return;
	}

	bones.clear();

	mstudiobone_t* pbone = (mstudiobone_t*)((byte*)pstudiohdr + pstudiohdr->boneindex);

	int limit = pstudiohdr->numbones;
	// go through the bones
	for (int i = 0; i < limit; i++, pbone++)
	{
		bones.emplace_back(pbone->name);
		//if ()
	}
}
//RENDERERS END