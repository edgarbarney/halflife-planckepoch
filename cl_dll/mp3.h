// mp3 support added by Killar

#ifndef MP3_H
#define MP3_H

#include "fmod.h"
#include "fmod_errors.h"
#include "Platform.h"
#include "PlatformHeaders.h"

#ifdef _WIN32
#define LIBRARY_EXTENSION "dll"
#define LOAD_LIBRARY(name) LoadLibrary(name)
#define UNLOAD_LIBRARY(ptr) FreeLibrary(ptr)
#define LIBRARY_TYPE HINSTANCE
#define GET_FUNCTION(var, lib, name, addrsize) (FARPROC&)(var) = GetProcAddress(lib, "_" #name "@" #addrsize)
#define OUTPUT_DRIVER FSOUND_OUTPUT_DSOUND
#else
#include <dlfcn.h>
#define LIBRARY_EXTENSION "so"
#define LOAD_LIBRARY(name) dlopen(name, RTLD_LAZY)
#define UNLOAD_LIBRARY(ptr) dlclose(ptr)
#define LIBRARY_TYPE void*
#define GET_FUNCTION(var, lib, name, addrsize) (void*&)(var) = dlsym(lib, #name);
#define OUTPUT_DRIVER FSOUND_OUTPUT_ALSA
#define _stdcall
#endif
#ifdef OSX // I don't plan on actually trying to get this working on OSX
#define LIBRARY_EXTENSION "dylib"
//#define OUTPUT_DRIVER FSOUND_OUTPUT_MAC // Maybe? Who knows.
#endif

class CMP3
{
private:
	float(_stdcall* VER)(void); //AJH get fmod dll version
	signed char(_stdcall* SCL)(FSOUND_STREAM* stream);
	signed char(_stdcall* SOP)(int outputtype);
	signed char(_stdcall* SBS)(int len_ms);
	signed char(_stdcall* SDRV)(int driver);
	signed char(_stdcall* INIT)(int mixrate, int maxsoftwarechannels, unsigned int flags);
	FSOUND_STREAM*(_stdcall* SOF)(const char* filename, unsigned int mode, int memlength);			  //AJH old fmod
	FSOUND_STREAM*(_stdcall* SO)(const char* filename, unsigned int mode, int offset, int memlength); //AJH use new fmod
	int(_stdcall* SPLAY)(int channel, FSOUND_STREAM* stream);
	void(_stdcall* CLOSE)(void);

	FSOUND_STREAM* m_Stream;
	int m_iIsPlaying;
	LIBRARY_TYPE m_hFMod;
	bool fmodInit;

public:
	int Initialize();
	int Shutdown();
	int PlayMP3(const char* pszSong);
	int StopMP3();
};

extern CMP3 gMP3;
#endif
