#pragma once

#include "Module.h"

#define MAX_SOUNDS	16
#define DEFAULT_MUSIC_FADE_TIME 2.0f

class ModuleAudio : public Module
{
public:

	ModuleAudio(Application* app, bool start_enabled = true);
	~ModuleAudio();

	bool Init();
	bool CleanUp();

	// Reprodueix un fitxer de música
	bool PlayMusic(const char* path, float fade_time = DEFAULT_MUSIC_FADE_TIME);

	// Carrega un so a memòria
	unsigned int LoadFx(const char* path);

	// Reprodueix un so carregat prèviament
	bool PlayFx(unsigned int fx, int repeat = 0);

private:

	Music music;
	Sound fx[MAX_SOUNDS];
	unsigned int fx_count;
};
