// FranticDreamer 2022

#ifndef HUD_IMGUI_SETTINGS_AUDIO_H
#define HUD_IMGUI_SETTINGS_AUDIO_H

struct SoundSettings_s
{
	union
	{
		struct
		{
			float masterVolume;
			float mp3Volume;
			float hevVolume;
			bool hiRezSound;
		};
		float data[3];
	};
};

class CClientImguiAudioSettings
{
private:
	// Sound settings that are being controlled live
	SoundSettings_s liveSoundSettings;

	// Sound settings that are applied OR loaded from config.cfg. AKA. Current sound settings
	SoundSettings_s currentSoundSettings;

public:
	void Init();

	// Parse this file only once? Not individually for keyboard, video etc.
	void ParseConfigFile(std::string filedir);

	void DrawAudioSettingsTab();

	void ApplySoundSettings();
	void CancelSoundSettings();

	SoundSettings_s GetSoundSettings();

};

#endif