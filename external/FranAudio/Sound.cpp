// FranticDreamer 2022

#include "FranAudio.hpp"
#include "Channel.hpp"
#include "Sound.hpp"

#include "Utilities.hpp"
#include "ALWrappers.hpp"
#include "ConversionUtils.hpp"

#include "libnyquist/Decoders.h"

nqr::AudioData snd_emptyAudioData = nqr::AudioData();
nqr::NyquistIO snd_loader;

FranAudio::Sound::Sound()
	: entity(INT32_MAX),
	channelType(ChannelType::Auto),
	sampleDir("ERR"),
	volume(0),
	attenuation(0),
	flags(0),
	pitch(0),
	firstUpdate(true),
	mouthSoundBuffer(nullptr)
{

}

FranAudio::Sound::Sound(int _entityIndex, int _channel, const char* _sample, float _volume, float _attenuation, int _flags, int _pitch) 
	: entity(_entityIndex), 
	channelType((ChannelType)_channel), 
	sampleDir(FranAudio::Utilities::ReturnPlayableDir(_sample, false)), 
	volume(_volume), 
	attenuation(_attenuation), 
	flags(_flags), 
	pitch(_pitch),
	firstUpdate(true),
	mouthSoundBuffer(nullptr)
{

	if (sampleDir == "ERR")
		return;

	PrecacheSoundSingle(sampleDir);

	auto& sound = FindPrecachedSoundSingle(sampleDir);

	if (sound.samples.empty())
	{
		// Load unsuccessful
		sampleDir = "ERR";
		return;
	}

	FranAudio::Globals::LogMessage(std::string(std::string("Trying to play sample: ") + "\"" + sampleDir + "\"" + " - " + std::to_string(sound.lengthSeconds) + " | " + std::to_string(sound.channelCount) + " | " + std::to_string(sound.sampleRate) + " | " + std::to_string(sound.samples.size())).c_str());

	mouthSoundBuffer = nullptr;
	mouthSoundBuffer = new uint8_t[sound.samples.size()];
	FranAudio::ConversionUtils::Resample_32to8(sound.samples.data(), mouthSoundBuffer, sound.samples.size());

	FranAudio_AlFunction(alGenBuffers, 1, &sourceBuffer);
	//FranAudio_AlFunction(alBufferData, sourceBuffer, GetFormat(sound.channelCount, sound.sourceFormat), mouthSoundBuffer, sound.samples.size(), sound.sampleRate);
	//FranAudio_AlFunction(alBufferData, sourceBuffer, FranAudio::Sound::GetFormat(sound.channelCount), sound.samples.data(), sound.samples.size() * 4 /* Float is 4x size of 8 bit int */, sound.sampleRate);
	
	const auto tempFormat = FranAudio::Sound::GetFormat(sound.channelCount);
	FranAudio_AlFunction(alBufferData, sourceBuffer, tempFormat, sound.samples.data(), sound.samples.size() * 4 /* Float is 4x size of 8 bit int */, sound.sampleRate);

	
	FranAudio_AlFunction(alGenSources, 1, &sourceHandle);
	FranAudio_AlFunction(alSourcef, sourceHandle, AL_PITCH, 1);
	FranAudio_AlFunction(alSourcef, sourceHandle, AL_GAIN, volume);

	FranAudio_AlFunction(alSourcei, sourceHandle, AL_SOURCE_RELATIVE, false);
	
	FranAudio_AlFunction(alSourcefv, sourceHandle, AL_POSITION, Vector{0, 0, 0});
	FranAudio_AlFunction(alSourcefv, sourceHandle, AL_VELOCITY, Vector{0, 0, 0});
	FranAudio_AlFunction(alSourcei, sourceHandle, AL_LOOPING, AL_FALSE);
	FranAudio_AlFunction(alSourcei, sourceHandle, AL_BUFFER, sourceBuffer);

	FranAudio_AlFunction(alSourcePlay, sourceHandle);

	sourceState = AL_PLAYING;

	FranAudio::Globals::LogMessage(std::string(std::string("Sample ") + "\"" + sampleDir.c_str() + "\"" + " playing.").c_str());

}

bool FranAudio::Sound::operator==(const Sound& _other)
{
	if (entity != _other.entity)
		return false;

	if (channelType != _other.channelType)
		return false;

	if (sampleDir != _other.sampleDir)
		return false;

	if (volume != _other.volume)
		return false;

	if (attenuation != _other.attenuation)
		return false;

	if (flags != _other.flags)
		return false;

	if (pitch != _other.pitch)
		return false;

	return true;
}

void FRANAUDIO_API FranAudio::Sound::Update(cl_entity_t* _ent)
{
	if (_ent != nullptr && sourceState == AL_STOPPED)
		_ent->mouth.mouthopen = 0;

	if (sampleDir == "ERR" || _ent == nullptr || sourceState == AL_STOPPED)
	{
		Kill();
		return;
	}

	if (firstUpdate)
	{
		firstUpdate = false;

		if (_ent->player)
			FranAudio_AlFunction(alSourcef, sourceHandle, AL_GAIN, 0.01f);
	}

	FranAudio_AlFunction(alGetSourcei, sourceHandle, AL_SOURCE_STATE, &sourceState);

	FranAudio_AlFunction(alSourcefv, sourceHandle, AL_POSITION, _ent->origin);
	
	if (mouthSoundBuffer == nullptr)
	{
		_ent->mouth.mouthopen = 0;
		return;
	}

	ALint sampleOffset;
	FranAudio_AlFunction(alGetSourcei, sourceHandle, AL_SAMPLE_OFFSET, &sampleOffset);

	//auto& sound = FindPrecachedSoundSingle(sampleDir);

	_ent->mouth.mouthopen = mouthSoundBuffer[sampleOffset] / 3;

	//if (_ent->model)
		//sampleHandle.setRadius(_ent->model->radius);
}

void FRANAUDIO_API FranAudio::Sound::SetPaused(bool _pause, bool isMenu)
{
	if (sampleDir == "ERR")
		return;

	FranAudio_AlFunction(alGetSourcei, sourceHandle, AL_SOURCE_STATE, &sourceState);

	if (isMenu)
	{
		isPausedByPauseMenu = _pause;

		if (_pause)
			FranAudio_AlFunction(alSourcePause, sourceHandle);
		else if (sourceState == AL_PAUSED)
			FranAudio_AlFunction(alSourcePlay, sourceHandle);
	}
	else if (!isPausedByPauseMenu)
	{
		if (_pause)
			FranAudio_AlFunction(alSourcePause, sourceHandle);
		else if(sourceState == AL_PAUSED)
			FranAudio_AlFunction(alSourcePlay, sourceHandle);
	}
}

void FranAudio::Sound::SetVolume(float _volume)
{
	volume = _volume;
	FranAudio_AlFunction(alSourcef, sourceHandle, AL_GAIN, _volume);
}

void FranAudio::Sound::SetPitch(float _pitch)
{
	pitch = _pitch;
	FranAudio_AlFunction(alSourcef, sourceHandle, AL_PITCH, _pitch / 100);
}

std::string FranAudio::Sound::GetDir()
{
	return sampleDir;
}

bool FranAudio::Sound::Kill()
{
	if (sampleDir == "ERR")
	{
		// Remove this sound from the list;
		size_t indexToPop = FranAudio::Sound::SoundsVector.Find(*this);
		if (indexToPop != SIZE_MAX || indexToPop < FranAudio::Sound::SoundsVector.Size())
		{
			FranAudio::Sound::SoundsVector.PopIndex(indexToPop);
			return true;
		}
		else
		{
			return false;
		}
	}

	if (mouthSoundBuffer != nullptr)
	{
		delete[] mouthSoundBuffer;
		mouthSoundBuffer = nullptr;
	}

	FranAudio_AlFunction(alSourceStop, sourceHandle);
	FranAudio_AlFunction(alDeleteSources, 1, &sourceHandle);
	FranAudio_AlFunction(alDeleteBuffers, 1, &sourceBuffer);

	//sampleDir = "ERR";

	// Remove this sound from the list;
	size_t indexToPop = FranAudio::Sound::SoundsVector.Find(*this);
	if (indexToPop != SIZE_MAX || indexToPop < FranAudio::Sound::SoundsVector.Size())
	{
		FranAudio::Sound::SoundsVector.PopIndex(indexToPop);
		return true;
	}
	else
	{
		return false;
	}
		
}

int FRANAUDIO_API FranAudio::Sound::EntIndex()
{
	return entity;
}

void FranAudio::Sound::KillAllSounds()
{
	while (SoundsVector.Size() > 0)
	{
		size_t beginsize = SoundsVector.Size();
		for (size_t i = 0; i < beginsize; i++)
		{
			if (beginsize > SoundsVector.Size())
				break;

			SoundsVector[i].Kill();
		}
	}
}

bool FRANAUDIO_API FranAudio::Sound::PrecacheSoundLegacy(const char* _dir)
{
	return PrecacheSound(_dir);
}

bool FranAudio::Sound::PrecacheSound(std::string _dir)
{
	if (SoundWaveMap.find(_dir) == SoundWaveMap.end())
	{
		FranAudio::Globals::LogMessage(std::string(std::string("Trying to load ") + "\"" + _dir.c_str() + "\"").c_str());

		nqr::AudioData _sample{};

		//if (snd_loader.IsFileSupported(_dir))
		//{
		//	FranAudio::Globals::LogMessage(std::string(std::string("Error loading sound: ") + "\"" + _dir.c_str() + "\"").c_str());
		//	return false;
		//}

		snd_loader.Load(&_sample, _dir);

		FranAudio::Globals::LogMessage(std::string(std::string("Loaded: ") + "\"" + _dir.c_str() + "\"" + " - " + std::to_string(_sample.lengthSeconds) + " | " + std::to_string(_sample.channelCount) + " | " + std::to_string(_sample.sampleRate) + " | " + std::to_string(_sample.samples.size())).c_str());

		SoundWaveMap.insert({_dir, _sample});
	}
	
	return true;
}

inline nqr::AudioData& FranAudio::Sound::FindPrecachedSound(std::string _dir)
{
	if (SoundWaveMap.find(_dir) == SoundWaveMap.end())
	{
		return snd_emptyAudioData; // Return empty sound sample
	}
	else
	{
		return SoundWaveMap.at(_dir);
	}
}

ALenum FranAudio::Sound::GetFormat(int _channels, bool _isDouble)
{
	if (FranAudio::Globals::IsFloatFormatSupported() && _channels == 1 && !_isDouble)
		return AL_FORMAT_MONO_FLOAT32;
	else if (FranAudio::Globals::IsFloatFormatSupported() && _channels == 2 && !_isDouble)
		return AL_FORMAT_STEREO_FLOAT32;
	else if (FranAudio::Globals::IsDoubleFormatSupported() && _channels == 1 && _isDouble)
		return AL_FORMAT_MONO_DOUBLE_EXT;
	else if (FranAudio::Globals::IsDoubleFormatSupported() && _channels == 2 && _isDouble)
		return AL_FORMAT_STEREO_DOUBLE_EXT;
	else
		return 0;
}

ALenum FranAudio::Sound::GetComplexFormat(int _channels, nqr::PCMFormat _format)
{
	if (_channels == 1 && _format == nqr::PCMFormat::PCM_U8)
		return AL_FORMAT_MONO8;
	else if (_channels == 1 && _format == nqr::PCMFormat::PCM_16)
		return AL_FORMAT_MONO16;
	else if (_channels == 2 && _format == nqr::PCMFormat::PCM_U8)
		return AL_FORMAT_STEREO8;
	else if (_channels == 2 && _format == nqr::PCMFormat::PCM_16)
		return AL_FORMAT_STEREO16;
	else if (FranAudio::Globals::IsFloatFormatSupported() && _channels == 1 && _format == nqr::PCMFormat::PCM_FLT)
		return AL_FORMAT_MONO_FLOAT32;
	else if (FranAudio::Globals::IsFloatFormatSupported() && _channels == 2 && _format == nqr::PCMFormat::PCM_FLT)
		return AL_FORMAT_STEREO_FLOAT32;
	else if (FranAudio::Globals::IsDoubleFormatSupported() && _channels == 1 && _format == nqr::PCMFormat::PCM_DBL)
		return AL_FORMAT_MONO_DOUBLE_EXT;
	else if (FranAudio::Globals::IsDoubleFormatSupported() && _channels == 2 && _format == nqr::PCMFormat::PCM_DBL)
		return AL_FORMAT_STEREO_DOUBLE_EXT;
	else
		return 0;
}

