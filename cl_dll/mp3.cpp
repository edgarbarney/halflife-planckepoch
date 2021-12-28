//mp3 support added by Killar

#include "hud.h"
#include "cl_util.h"
#include "mp3.h"

int CMP3::Initialize()
{

	char fmodlib[256];

	m_iIsPlaying = 0;
	sprintf(fmodlib, "%s/fmod.%s", gEngfuncs.pfnGetGameDirectory(), LIBRARY_EXTENSION);
#ifdef _WIN32
	// replace forward slashes with backslashes
	for (int i = 0; i < 256; i++)
		if (fmodlib[i] == '/')
			fmodlib[i] = '\\';
#endif

	m_hFMod = LOAD_LIBRARY(fmodlib);

	if (m_hFMod != NULL)
	{
		// fill in the function pointers
		//	GET_FUNCTION(VER, m_hFMod, FSOUND_GetVersion, 0);
		GET_FUNCTION(SCL, m_hFMod, FSOUND_Stream_Close, 4);
		GET_FUNCTION(SOP, m_hFMod, FSOUND_SetOutput, 4);
		GET_FUNCTION(SBS, m_hFMod, FSOUND_SetBufferSize, 4);
		GET_FUNCTION(SDRV, m_hFMod, FSOUND_SetDriver, 4);
		GET_FUNCTION(INIT, m_hFMod, FSOUND_Init, 12);
		GET_FUNCTION(SOF, m_hFMod, FSOUND_Stream_OpenFile, 12); //
																//	GET_FUNCTION(LNGTH, m_hFMod, FSOUND_Stream_GetLength, 4);	//
		GET_FUNCTION(SO, m_hFMod, FSOUND_Stream_Open, 16);		//AJH Use new version of fmod
		GET_FUNCTION(SPLAY, m_hFMod, FSOUND_Stream_Play, 8);
		GET_FUNCTION(CLOSE, m_hFMod, FSOUND_Close, 0);

		if (!(SCL && SOP && SBS && SDRV && INIT && (SOF || SO) && SPLAY && CLOSE))
		{
			UNLOAD_LIBRARY(m_hFMod);
			gEngfuncs.Con_Printf("Fatal Error: FMOD functions couldn't be loaded!\n");
			return 0;
		}
	}
	else
	{
		gEngfuncs.Con_Printf("Fatal Error: FMOD library couldn't be loaded!\n");
		return 0;
	}
	gEngfuncs.Con_Printf("FMOD.dll loaded succesfully!\n");
	return 1;
}

int CMP3::Shutdown()
{
	if (m_hFMod)
	{
		CLOSE();
		fmodInit = false;

		UNLOAD_LIBRARY(m_hFMod);
		m_hFMod = NULL;
		m_iIsPlaying = 0;
		return 1;
	}
	else
		return 0;
}

int CMP3::StopMP3(void)
{
	SCL(m_Stream);
	m_iIsPlaying = 0;
	return 1;
}

int CMP3::PlayMP3(const char* pszSong)
{
	if (m_iIsPlaying)
	{
		// sound system is already initialized
		SCL(m_Stream);
	}
	else if (!fmodInit)
	{
		fmodInit = true; // These functions must only be run once!
		SOP(OUTPUT_DRIVER);
		SBS(200);
		SDRV(0);
		INIT(44100, 1, 0); // we need just one channel, multiple mp3s at a time would be, erm, strange...
	}					   //AJH not for really cool effects, say walking past cars in a street playing different tunes, might change this later.

	char song[256];

	sprintf(song, "%s/%s", gEngfuncs.pfnGetGameDirectory(), pszSong);

	//	gEngfuncs.Con_Printf("Using fmod.dll version %f\n",VER());

	if (SO)
	{
		m_Stream = SO(song, FSOUND_NORMAL | FSOUND_LOOP_NORMAL, 0, 0); //AJH new version fmod uses Open
	}
	else if (SOF)
	{
		m_Stream = SOF(song, FSOUND_NORMAL | FSOUND_LOOP_NORMAL, 1); //AJH old version fmod OpenFile
	}
	if (m_Stream)
	{
		SPLAY(0, m_Stream);
		m_iIsPlaying = 1;
		return 1;
	}
	else
	{
		m_iIsPlaying = 0;
		gEngfuncs.Con_Printf("Error: Could not load %s\n", song);
		return 0;
	}
}