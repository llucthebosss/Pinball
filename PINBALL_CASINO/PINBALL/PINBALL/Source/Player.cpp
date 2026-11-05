#include "Globals.h"
#include "Application.h"
#include "ModulePlayer.h"

ModulePlayer::ModulePlayer(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModulePlayer::~ModulePlayer()
{}

// Carrega els recursos
bool ModulePlayer::Start()
{
	LOG("Loading player");
	return true;
}

// Allibera els recursos
bool ModulePlayer::CleanUp()
{
	LOG("Unloading player");

	return true;
}

// Update: dibuixa el fons
update_status ModulePlayer::Update()
{
	return UPDATE_CONTINUE;
}



