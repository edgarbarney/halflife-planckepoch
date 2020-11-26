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
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);
	//	void PostSpawn( void );
	void Precache(void);
	//void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT StartPlayFrom(void);
	void EXPORT Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void ForceStopSound(void);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flAttenuation;		// attenuation value

	float m_fVolBase = 10;		// Base volume.
	float m_fVolValue = 10;		// Volume to play. Changes with fade effects.
	float m_pPitch = 0;			// 

	CBasePlayer* m_pPlayer;

	BOOL	m_fActive;	// only TRUE when the entity is playing a looping sound
	BOOL	m_fLooping;	// TRUE when the sound played will loop
	edict_t* m_pPlayFrom; //LRC - the entity to play from

	int		m_iChannel = CHAN_STREAM; //LRC - the channel to play from, for "play from X" sounds
};

LINK_ENTITY_TO_CLASS(env_soundscape, CSoundScape);
TYPEDESCRIPTION	CSoundScape::m_SaveData[] =
{
	DEFINE_FIELD(CSoundScape, m_flAttenuation, FIELD_FLOAT),
	DEFINE_FIELD(CSoundScape, m_fActive, FIELD_BOOLEAN),
	DEFINE_FIELD(CSoundScape, m_fLooping, FIELD_BOOLEAN),
	DEFINE_FIELD(CSoundScape, m_iChannel, FIELD_INTEGER),
	DEFINE_FIELD(CSoundScape, m_pPlayFrom, FIELD_EDICT),
	DEFINE_FIELD(CSoundScape, m_fVolValue, FIELD_FLOAT),
	DEFINE_FIELD(CSoundScape, m_fVolBase, FIELD_FLOAT),
	DEFINE_FIELD(CSoundScape, m_pPitch, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CSoundScape, CBaseEntity);

//
// env_soundscape - user-defined environment sound
//
void CSoundScape::Spawn(void)
{
	//Shephard's Code for player
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
		if (!pPlayer) // Failed to retrieve a player at this index, skip and move on to the next one
			continue;

		m_pPlayFrom = ENT(pPlayer->pev);
	}

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
void CSoundScape::Precache(void)
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

void CSoundScape::ForceStopSound(void)
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
				0, 0, SND_STOP, m_pPitch);
		}
	}
}

//LRC - for some reason, I can't get other entities to start playing sounds during Activate;
// this function is used to delay the effect until the first Think, which seems to fix the problem.
void CSoundScape::StartPlayFrom(void)
{
	char* szSoundFile = (char*)STRING(pev->message);
	if (!m_pPlayFrom) {
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
			if (!pPlayer) // Failed to retrieve a player at this index, skip and move on to the next one
				continue;

			m_pPlayFrom = ENT(pPlayer->pev);
		}
	}
	EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC
		m_fVolValue, m_flAttenuation, SND_SPAWNING, m_pPitch);

	SetNextThink(0.1);
}

void CSoundScape::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	char* szSoundFile = (char*)STRING(pev->message);
	edict_t* ent = NULL;
	ent = FIND_ENTITY_BY_CLASSNAME(ent, "env_soundscape");
	bool ranAlready = true;
	int count = 1;
	//Thanks to Admer
	while (!FNullEnt(ent))
	{
		CBaseEntity* pEnt = CBaseEntity::Instance(ent);
		CSoundScape* pSoundScape = (CSoundScape*)pEnt;
		if (pSoundScape)
		{
			pSoundScape->ForceStopSound();
		}
		ent = FIND_ENTITY_BY_CLASSNAME(ent, "env_soundscape");
		count++;
	}
	StartPlayFrom();
}

//Removed due to new use system

/*
void CSoundScape::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	char* szSoundFile = (char*)STRING(pev->message);

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

	if (useType != USE_TOGGLE)
	{
		if ((m_fActive && useType == USE_ON) || (!m_fActive && useType == USE_OFF))
			return;
	}
	// Directly change pitch if arg passed. Only works if sound is already playing.

	if (useType == USE_SET && m_fActive)		// Momentary buttons will pass down a float in here
	{

		if (m_pPlayFrom)
		{
			EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, 0, 0, SND_CHANGE_PITCH, m_pPitch);
		}
		else
		{
			EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC
				0, 0, SND_STOP, 0);
		}

		return;
	}

	// Toggle

	// m_fActive is TRUE only if a looping sound is playing.

	if (m_fActive)
	{// turn sound off
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
				0, 0, SND_STOP, m_pPitch);
		}
	}
	else
	{// turn sound on

		// only toggle if this is a looping sound.  If not looping, each
		// trigger will cause the sound to play.  If the sound is still
		// playing from a previous trigger press, it will be shut off
		// and then restarted.

		if (m_fLooping)
		{
			m_fActive = TRUE;
		}
		else if (m_pPlayFrom)
		{
			STOP_SOUND(m_pPlayFrom, m_iChannel, szSoundFile); //LRC
		}
		else
		{
			// shut sound off now - may be interrupting a long non-looping sound
			EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC
				0, 0, SND_STOP, 0);
		}

		if (m_pPlayFrom)
		{
			EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC
				m_fVolValue, m_flAttenuation, 0, m_pPitch);
		}
		else
		{
			ALERT(at_console, "Cannot find entity(player) to play from\n");
		}

		SetNextThink(0.1);
	}
}
*/

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

	else
		CBaseEntity::KeyValue(pkvd);
}
