#ifndef __MODULEFONTS_H__
#define __MODULEFONTS_H__

#include "Module.h"

class ModuleFonts : public Module
{
public:

	ModuleFonts();

	// Destructor
	virtual ~ModuleFonts();

	// Es crida abans que el render estigui disponible
	bool Awake();

	// Es crida abans de sortir
	bool CleanUp();

    Font GetFont() const;
    int GetFontHeight(Font font) const;
	bool GetTextSize(const char* text, int& width, int& height, int spacing = 0) const;

private:

    Font font;
};


#endif // __MODULEFONTS_H__