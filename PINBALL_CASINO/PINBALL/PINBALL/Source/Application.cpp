
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "ModuleGame.h"

#include "Application.h"

Application::Application()
{
	window = new ModuleWindow(this);
	renderer = new ModuleRender(this);
	audio = new ModuleAudio(this, true);
	physics = new ModulePhysics(this);
	scene_intro = new ModuleGame(this);

	// L'ordre de les crides és molt important!
	// Els mòduls fan Init(), Start() i Update() en aquest ordre
	// i CleanUp() en l'ordre invers

	// Mòduls principals
	AddModule(window);
	AddModule(physics);
	AddModule(audio);

	// Escenes
	AddModule(scene_intro);

	// El dibuix es fa al final
	AddModule(renderer);
}

Application::~Application()
{
	for (auto it = list_modules.rbegin(); it != list_modules.rend(); ++it)
	{
		Module* item = *it;
		delete item;
	}
	list_modules.clear();

}

bool Application::Init()
{
	bool ret = true;

	// Crida Init() a tots els mòduls
	for (auto it = list_modules.begin(); it != list_modules.end() && ret; ++it)
	{
		Module* module = *it;
		ret = module->Init();
	}

	// Després de tots els Init(), es crida Start() a tots els mòduls
	LOG("Application Start --------------");

	for (auto it = list_modules.begin(); it != list_modules.end() && ret; ++it)
	{
		Module* module = *it;
		ret = module->Start();
	}

	return ret;
}

// Crida PreUpdate, Update i PostUpdate a tots els mòduls
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;

	for (auto it = list_modules.begin(); it != list_modules.end() && ret == UPDATE_CONTINUE; ++it)
	{
		Module* module = *it;
		if (module->IsEnabled())
		{
			ret = module->PreUpdate();
		}
	}

	for (auto it = list_modules.begin(); it != list_modules.end() && ret == UPDATE_CONTINUE; ++it)
	{
		Module* module = *it;
		if (module->IsEnabled())
		{
			ret = module->Update();
		}
	}

	for (auto it = list_modules.begin(); it != list_modules.end() && ret == UPDATE_CONTINUE; ++it)
	{
		Module* module = *it;
		if (module->IsEnabled())
		{
			ret = module->PostUpdate();
		}
	}

	if (WindowShouldClose()) ret = UPDATE_STOP;

	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;
	for (auto it = list_modules.rbegin(); it != list_modules.rend() && ret; ++it)
	{
		Module* item = *it;
		ret = item->CleanUp();
	}

	return ret;
}

void Application::AddModule(Module* mod)
{
	list_modules.emplace_back(mod);
}