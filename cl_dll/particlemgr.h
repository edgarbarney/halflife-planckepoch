// 02/08/02 November235: Particle System
#pragma once

#include "particlesys.h"

class ParticleSystemManager
{
public:
				ParticleSystemManager( );
				~ParticleSystemManager( );
	void		AddSystem( ParticleSystem* );
	ParticleSystem *FindSystem( cl_entity_t* pEntity );
	void		UpdateSystems( float frametime );
	void		ClearSystems( );
	void		SortSystems( );
//	void		DeleteSystem( ParticleSystem* );

//private:
	ParticleSystem* m_pFirstSystem;
	//ParticleSystem* systemio;
};

extern ParticleSystemManager* g_pParticleSystems; 
