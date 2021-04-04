//FranticDreamer's implementation of the basic function of the env_soundscape
//
//Mostly based on standard ambient_generic

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

class CSoundScape : public CBaseEntity
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	//	void PostSpawn( void );
	void Precache() override;
	//void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT StartPlayFrom();
	void EXPORT Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void ForceStopSound();
	void RefreshPlayerVar();
	char* GetSoundFileDir();

	int		Save(CSave& save) override;
	int		Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flAttenuation;		// attenuation value

	float m_fVolBase = 10;		// Base volume.
	float m_fVolValue = 10;		// Volume to play. Changes with fade effects.
	float m_pPitch = 0;			// Pitch
	
	int m_ssType = 0;


	CBasePlayer* m_pPlayer;

	BOOL	m_fCanPlay; // Make this entity playable when others cant
	BOOL	m_fActive;	// only TRUE when the entity is playing a looping sound
	BOOL	m_fLooping;	// TRUE when the sound played will loop
	edict_t* m_pPlayFrom; //LRC - the entity to play from

	int		m_iChannel = CHAN_AUTO; //LRC - the channel to play from, for "play from X" sounds
};

LINK_ENTITY_TO_CLASS(env_soundscape, CSoundScape);
TYPEDESCRIPTION	CSoundScape::m_SaveData[] =
{
	DEFINE_FIELD(CSoundScape, m_flAttenuation, FIELD_FLOAT),
	DEFINE_FIELD(CSoundScape, m_fActive, FIELD_BOOLEAN),
	DEFINE_FIELD(CSoundScape, m_fCanPlay, FIELD_BOOLEAN),
	DEFINE_FIELD(CSoundScape, m_fLooping, FIELD_BOOLEAN),
	DEFINE_FIELD(CSoundScape, m_iChannel, FIELD_INTEGER),
	DEFINE_FIELD(CSoundScape, m_pPlayFrom, FIELD_EDICT),
	DEFINE_FIELD(CSoundScape, m_fVolValue, FIELD_FLOAT),
	DEFINE_FIELD(CSoundScape, m_fVolBase, FIELD_FLOAT),
	DEFINE_FIELD(CSoundScape, m_pPitch, FIELD_FLOAT),
	DEFINE_FIELD(CSoundScape, m_ssType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CSoundScape, CBaseEntity);

//
// env_soundscape - user-defined environment sound
//

void CSoundScape::RefreshPlayerVar()
{
	//Shephard's Code for player
	if (!m_pPlayFrom) {
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
			if (!pPlayer) // Failed to retrieve a player at this index, skip and move on to the next one
				continue;

			m_pPlayFrom = ENT(pPlayer->pev);
		}
	}
}

char* CSoundScape::GetSoundFileDir() 
{
	return (char*)STRING(pev->message);
}

void CSoundScape::Spawn()
{
	m_flAttenuation = ATTN_NONE;

	char* szSoundFile = (char*)STRING(pev->message);

	if (FStringNull(pev->message) || strlen(szSoundFile) < 1)
	{
		ALERT(at_error, "env_soundscape \"%s\" at (%f, %f, %f) has no sound file\n",
			STRING(pev->targetname), pev->origin.x, pev->origin.y, pev->origin.z);
		SetNextThink(0.1);
		SetThink(&CSoundScape::SUB_Remove);
		return;
	}
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	DontThink();

	// "Use" function to change ambient

	SetUse(&CSoundScape::Use);

	m_fActive = FALSE;

	if (FBitSet(pev->spawnflags, AMBIENT_SOUND_NOT_LOOPING))
		m_fLooping = FALSE;
	else
		m_fLooping = TRUE;
	Precache();
}

// this function needs to be called when the game is loaded, not just when the entity spawns.
// Don't make this a PostSpawn function.
void CSoundScape::Precache()
{
	char* szSoundFile = (char*)STRING(pev->message);

	if (!FStringNull(pev->message) && strlen(szSoundFile) > 1)
	{
		if (*szSoundFile != '!')
			PRECACHE_SOUND(szSoundFile);
	}

	if (!FBitSet(pev->spawnflags, AMBIENT_SOUND_START_SILENT))
	{
		// start the sound ASAP
		if (m_fLooping)
			m_fActive = TRUE;
	}

	if (m_fActive)
	{
		if (m_pPlayFrom)
		{
			SetThink(&CSoundScape::StartPlayFrom); //LRC
//			EMIT_SOUND_DYN( m_pPlayFrom, m_iChannel, szSoundFile, //LRC
//					(m_dpv.vol * 0.01), m_flAttenuation, SND_SPAWNING, m_dpv.pitch);

//			ALERT(at_console, "AMBGEN: spawn start\n");
		}
		else
		{
			EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC
				m_fVolValue, m_flAttenuation, SND_SPAWNING, m_pPitch);
		}
		SetNextThink(0.1);
	}
}

void CSoundScape::ForceStopSound()
{
	char* szSoundFile = (char*)STRING(pev->message);
	if (m_fActive)
	{
		// turn sound off
		m_fActive = FALSE;

		// HACKHACK - this makes the code in Precache() work properly after a save/restore
		pev->spawnflags |= AMBIENT_SOUND_START_SILENT;

		if (m_pPlayFrom)
		{
			STOP_SOUND(m_pPlayFrom, m_iChannel, szSoundFile);
		}
		else
		{
			EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC
				m_fVolValue, 0, SND_STOP, m_pPitch);
		}
	}
}

//LRC - for some reason, I can't get other entities to start playing sounds during Activate;
// this function is used to delay the effect until the first Think, which seems to fix the problem.
void CSoundScape::StartPlayFrom()
{
	if (!m_fActive){
		char* szSoundFile = (char*)STRING(pev->message);
		RefreshPlayerVar();
		EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC
			m_fVolValue, m_flAttenuation, SND_SPAWNING, m_pPitch);

		SetNextThink(0.1);
		m_fActive = true;
	}
}

//Thanks to Admer and Solokiller
void CSoundScape::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CSoundScape* soundScape = nullptr;

	while (((soundScape = static_cast<CSoundScape*>(UTIL_FindEntityByClassname(soundScape, "env_soundscape")))) != nullptr && !FNullEnt(soundScape->pev))
	{
		if (!soundScape->m_fCanPlay && GetSoundFileDir() != soundScape->GetSoundFileDir()){
			soundScape->ForceStopSound();
		}
	}
	if (m_ssType == 0)
	{
		RefreshPlayerVar();
		StartPlayFrom();
	}
	else
	{
		ForceStopSound();
	}
}

// KeyValue - Load keyvalues from the entity data thingy
// NOTE: called BEFORE spawn!

void CSoundScape::KeyValue(KeyValueData* pkvd)
{
	// pitchrun
	if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		m_pPitch = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		if (m_pPitch > 255) m_pPitch = 255;
		if (m_pPitch < 0) m_pPitch = 0;
	}

	// volstart
	else if (FStrEq(pkvd->szKeyName, "health"))
	{
		m_fVolValue = atoi(pkvd->szValue);

		if (m_fVolValue > 10) m_fVolValue = 10;
		if (m_fVolValue < 0) m_fVolValue = 0;

		m_fVolValue *= 10;	// 0 - 100

		pkvd->fHandled = TRUE;
	}

	// pitchrun
	else if (FStrEq(pkvd->szKeyName, "sstype"))
	{
		m_ssType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else
		CBaseEntity::KeyValue(pkvd);
}
