#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"

#include "raylib.h"

#define MAX_FX_SOUNDS   64

ModuleAudio::ModuleAudio(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	fx_count = 0;
	music = Music{ 0 };
}

// Destructor
ModuleAudio::~ModuleAudio()
{
}

// Es crida abans que el render estigui disponible
bool ModuleAudio::Init()
{
	LOG("Loading Audio Mixer");
	bool ret = true;

	LOG("Loading raylib audio system");

	InitAudioDevice();

	return ret;
}

// Es crida abans de sortir
bool ModuleAudio::CleanUp()
{
	LOG("Freeing sound FX, closing Mixer and Audio subsystem");

	// Allibera els sons
	for (unsigned int i = 0; i < fx_count; i++)
	{
		UnloadSound(fx[i]);
	}

	// Allibera la música
	if (IsMusicReady(music))
	{
		StopMusicStream(music);
		UnloadMusicStream(music);
	}

	CloseAudioDevice();

	return true;
}

// Reprodueix un fitxer de música
bool ModuleAudio::PlayMusic(const char* path, float fade_time)
{
	if (IsEnabled() == false)
		return false;

	bool ret = true;

	StopMusicStream(music);
	music = LoadMusicStream(path);

	PlayMusicStream(music);

	LOG("Successfully playing %s", path);

	return ret;
}

// Carrega un WAV
unsigned int ModuleAudio::LoadFx(const char* path)
{
	if (IsEnabled() == false)
		return 0;

	unsigned int ret = 0;

	Sound sound = LoadSound(path);

	if (sound.stream.buffer == NULL)
	{
		LOG("Cannot load sound: %s", path);
	}
	else
	{
		fx[fx_count++] = sound;
		ret = fx_count;
	}

	return ret;
}

// Reprodueix un WAV
bool ModuleAudio::PlayFx(unsigned int id, int repeat)
{
	if (IsEnabled() == false) return false;
	if (id > 0 && id <= fx_count)
	{
		PlaySound(fx[id - 1]);
		return true;
	}
	return false;
}