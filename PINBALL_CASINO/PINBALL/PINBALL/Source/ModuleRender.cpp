#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include <math.h>

ModuleRender::ModuleRender(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    background = RAYWHITE;
}

// Destructor
ModuleRender::~ModuleRender()
{
}

// Es crida abans que el render estigui disponible
bool ModuleRender::Init()
{
    LOG("Creating Renderer context");
    bool ret = true;

    return ret;
}

// PreUpdate: obre el fotograma i el neteja, abans que el joc dibuixi res
update_status ModuleRender::PreUpdate()
{
    BeginDrawing();
    ClearBackground(background);

    return UPDATE_CONTINUE;
}

update_status ModuleRender::Update()
{
    return UPDATE_CONTINUE;
}

// PostUpdate: tanca el fotograma i el mostra
update_status ModuleRender::PostUpdate()
{
    DrawFPS(10, 10);

    EndDrawing();

    return UPDATE_CONTINUE;
}

// Es crida abans de sortir
bool ModuleRender::CleanUp()
{
    return true;
}

void ModuleRender::SetBackgroundColor(Color color)
{
    background = color;
}

// Dibuixa a la pantalla
bool ModuleRender::Draw(Texture2D texture, int x, int y, const Rectangle* section, double angle, int pivot_x, int pivot_y) const
{
    bool ret = true;

    float scale = 1.0f;
    Vector2 position = { (float)x, (float)y };
    Rectangle rect = { 0.f, 0.f, (float)texture.width, (float)texture.height };

    if (section != NULL) rect = *section;

    position.x = (float)(x - pivot_x) * scale + camera.x;
    position.y = (float)(y - pivot_y) * scale + camera.y;

    rect.width *= scale;
    rect.height *= scale;

    DrawTextureRec(texture, rect, position, WHITE);

    return ret;
}

bool ModuleRender::DrawText(const char* text, int x, int y, Font font, int spacing, Color tint) const
{
    bool ret = true;

    Vector2 position = { (float)x, (float)y };

    DrawTextEx(font, text, position, (float)font.baseSize, (float)spacing, tint);

    return ret;
}