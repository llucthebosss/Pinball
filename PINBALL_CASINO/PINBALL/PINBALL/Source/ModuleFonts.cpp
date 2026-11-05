#include "Application.h"
#include "ModuleRender.h"
#include "ModuleFonts.h"

#include "Defs.h"
#include "Log.h"

ModuleFonts::ModuleFonts() : Module()
{
}

// Destructor
ModuleFonts::~ModuleFonts()
{}

// Es crida quan la finestra ja està disponible
bool ModuleFonts::Awake()
{
	bool ret = true;

    LOG("Init default font");

	const char* path = "fonts/open_sans/OpenSans-Regular.ttf";
	int size = 36;

    font = LoadFontEx(path, size, NULL, 0);

	return ret;
}

// Es crida abans de sortir
bool ModuleFonts::CleanUp()
{
	LOG("Freeing font");

    UnloadFont(font);

	return true;
}

Font ModuleFonts::GetFont() const
{
    return font;
}

// Calcula l'alçada d'una font
int ModuleFonts::GetFontHeight(Font font) const
{
    return font.baseSize;
}

// Calcula la mida d'un text
bool ModuleFonts::GetTextSize(const char* text, int& width, int& height, int spacing) const
{
	bool ret = false;

    MeasureTextEx(font, text, font.baseSize, spacing);

	return ret;
}

