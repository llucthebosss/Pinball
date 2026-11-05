#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include <algorithm>
#include <math.h>
#include <time.h>

namespace
{
	
	constexpr float INLANE_WALL_THICKNESS = 5.0f;

	// Radi dels bumpers
	constexpr int BUMPER_RADIUS = 50;

	// Pivots dels flippers. 
	constexpr int FLIPPER_LEFT_X = 185;
	constexpr int FLIPPER_LEFT_Y = 747;
	constexpr int FLIPPER_RIGHT_X = 376;
	constexpr int FLIPPER_RIGHT_Y = 742;
	// En repòs tenen la mateixa inclinació que les guies (uns 30 graus)
	constexpr float FLIPPER_REST_ANGLE = 0.53f;
	constexpr float FLIPPER_UP_ANGLE = 0.45f;

	// Fons del carril HIGH ROLLER, entre la paret blava de la dreta i el carril de llançament
	constexpr int HIGH_ROLLER_X = 522;
	constexpr int HIGH_ROLLER_Y = 495;
	constexpr int HIGH_ROLLER_W = 28;
	constexpr int HIGH_ROLLER_H = 50;
	constexpr int HIGH_ROLLER_POINTS = 500;
	// Velocitat (m/s) amb què la bola surt disparada cap amunt pel carril
	constexpr float HIGH_ROLLER_LAUNCH_SPEED = 32.0f;
}


//FLIPPERS


static void DrawFlipperVisual(b2RevoluteJoint* joint, bool left)
{
	if (!joint) return;

	b2Body* body = joint->GetBodyB();
	b2Vec2 pos = body->GetPosition();
	float angle = body->GetAngle();

	float px = METERS_TO_PIXELS(pos.x);
	float py = METERS_TO_PIXELS(pos.y);

	float length = 78.0f;
	float thickness = 16.0f;

	float hx = length * 0.5f;
	float hy = thickness * 0.5f;
	float cx = left ? hx : -hx;

	Vector2 corners[4] = {
		{ cx - hx, -hy },
		{ cx + hx, -hy },
		{ cx + hx,  hy },
		{ cx - hx,  hy }
	};

	Vector2 screen[4];
	for (int i = 0; i < 4; ++i) {
		float rx = corners[i].x * cos(angle) - corners[i].y * sin(angle);
		float ry = corners[i].x * sin(angle) + corners[i].y * cos(angle);
		screen[i].x = px + rx;
		screen[i].y = py + ry;
	}

	DrawTriangle(screen[0], screen[2], screen[1], ORANGE);
	DrawTriangle(screen[0], screen[3], screen[2], ORANGE);

	DrawLineV(screen[0], screen[1], DARKGRAY);
	DrawLineV(screen[1], screen[2], DARKGRAY);
	DrawLineV(screen[2], screen[3], DARKGRAY);
	DrawLineV(screen[3], screen[0], DARKGRAY);

	DrawCircle(px, py, 3, RED);
}


static void DrawTriSafe(Vector2 a, Vector2 b, Vector2 c, Color col)
{
	float cross = (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
	if (cross > 0.0f) DrawTriangle(a, c, b, col);
	else DrawTriangle(a, b, c, col);
}

// Rectangle centrat 
static void DrawRotRect(float cx, float cy, float w, float h, float deg, Color c)
{
	DrawRectanglePro(Rectangle{ cx, cy, w, h }, Vector2{ w * 0.5f, h * 0.5f }, deg, c);
}

// Mida de lletra
static int FitFont(const char* text, int maxW, int maxSize)
{
	int s = maxSize;
	while (s > 8 && MeasureText(text, s) > maxW) s -= 2;
	return s;
}


static void DrawSparkle(float x, float y, float s, Color col)
{
	if (s < 0.5f) return;
	float w = s * 0.18f;
	DrawCircleV(Vector2{ x, y }, s * 0.55f, ColorAlpha(col, 0.12f));
	DrawTriSafe(Vector2{ x, y - s }, Vector2{ x + w, y }, Vector2{ x - w, y }, col);
	DrawTriSafe(Vector2{ x, y + s }, Vector2{ x + w, y }, Vector2{ x - w, y }, col);
	DrawTriSafe(Vector2{ x - s, y }, Vector2{ x, y + w }, Vector2{ x, y - w }, col);
	DrawTriSafe(Vector2{ x + s, y }, Vector2{ x, y + w }, Vector2{ x, y - w }, col);
}

// Text de neó
static void DrawNeonText(const char* text, int x, int y, int size, Color core, Color glow, float inten)
{
	DrawText(text, x + 3, y + 4, size, Color{ 0, 0, 0, 150 });

	for (int i = 12; i > 0; i -= 3)
	{
		float a = 0.16f * inten * (1.0f - (float)i / 14.0f);
		int d = (int)(i * 0.7f);
		Color g = ColorAlpha(glow, a);
		DrawText(text, x - i, y, size, g);
		DrawText(text, x + i, y, size, g);
		DrawText(text, x, y - i, size, g);
		DrawText(text, x, y + i, size, g);
		DrawText(text, x - d, y - d, size, g);
		DrawText(text, x + d, y - d, size, g);
		DrawText(text, x - d, y + d, size, g);
		DrawText(text, x + d, y + d, size, g);
	}

	DrawText(text, x, y, size, core);
	DrawText(text, x, y - 1, size, ColorAlpha(WHITE, 0.28f * inten));
}

//Efectes mení
static void DrawBulbRect(float x, float y, float w, float h, float spacing, float t, Color col)
{
	float total = 2.0f * (w + h);
	int n = (int)(total / spacing);
	if (n < 4) return;
	int shift = (int)(t * 9.0f);

	for (int i = 0; i < n; i++)
	{
		float d = total * (float)i / (float)n;
		Vector2 p;
		if (d < w) p = Vector2{ x + d, y };
		else if (d < w + h) p = Vector2{ x + w, y + (d - w) };
		else if (d < 2.0f * w + h) p = Vector2{ x + w - (d - w - h), y + h };
		else p = Vector2{ x, y + h - (d - 2.0f * w - h) };

		bool on = ((i + shift) % 3) != 0;
		DrawCircleV(p, 4.5f, ColorAlpha(col, on ? 0.28f : 0.06f));
		DrawCircleV(p, 2.6f, on ? ColorAlpha(Color{ 255, 250, 210, 255 }, 1.0f) : ColorAlpha(col, 0.30f));
	}
}

// Espiral daurada
static void DrawGoldScroll(float x, float y, float s, float dir)
{
	for (int half = 0; half < 2; half++)
	{
		float sgn = (half == 0) ? -1.0f : 1.0f;   // una corba cap amunt i una altra cap avall
		Vector2 prev = { x, y };
		for (int k = 1; k <= 60; k++)
		{
			float a = (float)k * 0.24f;
			float r = s * (1.0f - (float)k / 66.0f);
			Vector2 p = { x + dir * (s - r * cosf(a)), y + sgn * r * sinf(a) };
			DrawLineEx(prev, p, 2.2f, GOLD);
			prev = p;
		}
		DrawCircleV(prev, 2.6f, GOLD);
	}


	DrawLineEx(Vector2{ x - dir * s * 1.3f, y }, Vector2{ x, y }, 2.4f, GOLD);
	DrawEllipse((int)(x - dir * s * 0.7f), (int)(y - s * 0.22f), s * 0.28f, s * 0.10f, ColorAlpha(GOLD, 0.9f));
	DrawEllipse((int)(x - dir * s * 0.7f), (int)(y + s * 0.22f), s * 0.28f, s * 0.10f, ColorAlpha(GOLD, 0.9f));
}


static void DrawSuit(int suit, Vector2 c, float s, float ang, Color col)
{
	float ca = cosf(ang), sa = sinf(ang);
	auto P = [&](float x, float y) {
		return Vector2{ c.x + (x * ca - y * sa) * s, c.y + (x * sa + y * ca) * s };
	};

	switch (suit)
	{
	case 0: // cor
		DrawCircleV(P(-0.5f, -0.35f), 0.52f * s, col);
		DrawCircleV(P(0.5f, -0.35f), 0.52f * s, col);
		DrawTriSafe(P(-0.98f, -0.1f), P(0.98f, -0.1f), P(0.0f, 1.0f), col);
		break;
	case 1: // diamant
		DrawTriSafe(P(0.0f, -1.1f), P(-0.75f, 0.0f), P(0.75f, 0.0f), col);
		DrawTriSafe(P(0.0f, 1.1f), P(-0.75f, 0.0f), P(0.75f, 0.0f), col);
		break;
	case 2: // pica
		DrawCircleV(P(-0.5f, 0.25f), 0.52f * s, col);
		DrawCircleV(P(0.5f, 0.25f), 0.52f * s, col);
		DrawTriSafe(P(-0.98f, 0.1f), P(0.98f, 0.1f), P(0.0f, -1.0f), col);
		DrawTriSafe(P(0.0f, 0.3f), P(-0.35f, 1.05f), P(0.35f, 1.05f), col);
		break;
	default: // trèvol
		DrawCircleV(P(0.0f, -0.5f), 0.45f * s, col);
		DrawCircleV(P(-0.52f, 0.2f), 0.45f * s, col);
		DrawCircleV(P(0.52f, 0.2f), 0.45f * s, col);
		DrawCircleV(P(0.0f, 0.0f), 0.30f * s, col);
		DrawTriSafe(P(0.0f, 0.1f), P(-0.3f, 1.05f), P(0.3f, 1.05f), col);
		break;
	}
}

//Carta de pòquer
static void DrawPlayingCard(float cx, float cy, float w, float h, float deg, int suit)
{
	float rad = deg * DEG2RAD;
	float ca = cosf(rad), sa = sinf(rad);

	DrawRotRect(cx + 3.0f, cy + 5.0f, w, h, deg, Color{ 0, 0, 0, 90 });
	DrawRotRect(cx, cy, w, h, deg, Color{ 200, 160, 60, 255 });
	DrawRotRect(cx, cy, w - 3.0f, h - 3.0f, deg, Color{ 250, 246, 236, 255 });

	Color col = (suit < 2) ? Color{ 200, 20, 35, 255 } : Color{ 25, 25, 35, 255 };
	DrawSuit(suit, Vector2{ cx, cy }, w * 0.24f, rad, col);

	float ox = -w * 0.5f + 9.0f;
	float oy = -h * 0.5f + 11.0f;
	Vector2 p1 = { cx + ox * ca - oy * sa, cy + ox * sa + oy * ca };
	Vector2 p2 = { cx - ox * ca + oy * sa, cy - ox * sa - oy * ca };
	DrawSuit(suit, p1, w * 0.10f, rad, col);
	DrawSuit(suit, p2, w * 0.10f, rad + PI, col);
}

//Dau
static void DrawDie(float cx, float cy, float s, float deg, int value)
{
	float rad = deg * DEG2RAD;
	float ca = cosf(rad), sa = sinf(rad);

	DrawRotRect(cx + 3.0f, cy + 5.0f, s, s, deg, Color{ 0, 0, 0, 100 });
	DrawRotRect(cx, cy, s, s, deg, Color{ 160, 130, 85, 255 });
	DrawRotRect(cx, cy, s - 4.0f, s - 4.0f, deg, Color{ 245, 235, 210, 255 });

	
	int pts[6][2];
	int n = 0;
	switch (value)
	{
	case 1: pts[n][0] = 0;  pts[n][1] = 0;  n++; break;
	case 2: pts[n][0] = -1; pts[n][1] = -1; n++; pts[n][0] = 1;  pts[n][1] = 1;  n++; break;
	case 3: pts[n][0] = -1; pts[n][1] = -1; n++; pts[n][0] = 0;  pts[n][1] = 0;  n++; pts[n][0] = 1;  pts[n][1] = 1;  n++; break;
	case 4: pts[n][0] = -1; pts[n][1] = -1; n++; pts[n][0] = 1;  pts[n][1] = -1; n++;
	        pts[n][0] = -1; pts[n][1] = 1;  n++; pts[n][0] = 1;  pts[n][1] = 1;  n++; break;
	case 5: pts[n][0] = -1; pts[n][1] = -1; n++; pts[n][0] = 1;  pts[n][1] = -1; n++;
	        pts[n][0] = 0;  pts[n][1] = 0;  n++;
	        pts[n][0] = -1; pts[n][1] = 1;  n++; pts[n][0] = 1;  pts[n][1] = 1;  n++; break;
	default:
	        pts[n][0] = -1; pts[n][1] = -1; n++; pts[n][0] = 1;  pts[n][1] = -1; n++;
	        pts[n][0] = -1; pts[n][1] = 0;  n++; pts[n][0] = 1;  pts[n][1] = 0;  n++;
	        pts[n][0] = -1; pts[n][1] = 1;  n++; pts[n][0] = 1;  pts[n][1] = 1;  n++; break;
	}

	for (int i = 0; i < n; i++)
	{
		float lx = pts[i][0] * s * 0.27f;
		float ly = pts[i][1] * s * 0.27f;
		Vector2 p = { cx + lx * ca - ly * sa, cy + lx * sa + ly * ca };
		DrawCircleV(p, s * 0.085f, Color{ 30, 20, 20, 255 });
	}
}

//Fitxa de casino
static void DrawChip(float cx, float cy, float r, Color main, float rotDeg)
{
	Vector2 c = { cx, cy };
	Color dark = { (unsigned char)(main.r * 0.5f), (unsigned char)(main.g * 0.5f), (unsigned char)(main.b * 0.5f), 255 };
	Color cream = { 245, 240, 225, 255 };

	DrawCircleV(Vector2{ cx + 1.0f, cy + r * 0.22f + 2.0f }, r, Color{ 0, 0, 0, 100 });
	DrawCircleV(Vector2{ cx, cy + r * 0.20f }, r, dark);
	DrawCircleV(c, r, main);

	for (int i = 0; i < 8; i++)
	{
		float a0 = rotDeg + i * 45.0f;
		DrawRing(c, r * 0.74f, r * 0.98f, a0, a0 + 22.0f, 4, cream);
	}

	DrawCircleV(c, r * 0.70f, main);
	DrawRing(c, r * 0.52f, r * 0.58f, 0.0f, 360.0f, 24, ColorAlpha(cream, 0.9f));
	DrawCircleV(c, r * 0.50f, dark);
	DrawCircleV(c, r * 0.42f, main);
	DrawCircleV(Vector2{ cx - r * 0.3f, cy - r * 0.3f }, r * 0.14f, ColorAlpha(WHITE, 0.25f));
}

//Moneda daurada que gira
static void DrawCoin(float cx, float cy, float r, float phase)
{
	float w = fabsf(cosf(phase));
	float rx = fmaxf(r * w, 1.5f);

	DrawEllipse((int)cx, (int)cy + 3, rx + 1.0f, r, Color{ 140, 90, 8, 255 });
	DrawEllipse((int)cx, (int)cy, rx, r, GOLD);
	DrawEllipse((int)cx, (int)cy, rx * 0.78f, r * 0.78f, Color{ 255, 222, 90, 255 });

	if (w > 0.45f)
	{
		const char* d = "$";
		int fs = (int)(r * 1.1f);
		DrawText(d, (int)(cx - MeasureText(d, fs) / 2), (int)(cy - fs / 2), fs, Color{ 170, 115, 10, 255 });
	}
	DrawEllipse((int)(cx - rx * 0.3f), (int)(cy - r * 0.35f), rx * 0.18f + 0.5f, r * 0.18f, ColorAlpha(WHITE, 0.5f));
}

//Roda de ruleta (com els bumpers del tauler)
static void DrawRouletteWheel(float cx, float cy, float R, float rotDeg, float t)
{
	Vector2 c = { cx, cy };

	DrawCircleV(Vector2{ cx, cy + 4.0f }, R + 2.0f, Color{ 0, 0, 0, 110 });
	DrawCircleV(c, R, Color{ 120, 80, 10, 255 });
	DrawCircleV(c, R - 2.0f, GOLD);
	DrawCircleV(c, R - 6.0f, Color{ 25, 8, 20, 255 });

	// Caselles vermelles i negres (una de verda)
	for (int i = 0; i < 36; i++)
	{
		float a0 = rotDeg + i * 10.0f;
		Color col = (i == 0) ? Color{ 0, 130, 70, 255 } : ((i % 2) ? Color{ 200, 25, 35, 255 } : Color{ 18, 18, 24, 255 });
		DrawRing(c, R * 0.64f, R * 0.88f, a0, a0 + 9.4f, 4, col);
	}
	DrawRing(c, R * 0.88f, R * 0.90f, 0.0f, 360.0f, 48, GOLD);
	DrawRing(c, R * 0.62f, R * 0.64f, 0.0f, 360.0f, 40, GOLD);

	// Anell interior vermell amb segments daurats
	DrawCircleV(c, R * 0.62f, Color{ 170, 15, 28, 255 });
	for (int i = 0; i < 8; i++)
	{
		float a0 = -rotDeg * 0.5f + i * 45.0f;
		DrawRing(c, R * 0.38f, R * 0.60f, a0, a0 + 22.0f, 6, GOLD);
	}
	DrawRing(c, R * 0.36f, R * 0.39f, 0.0f, 360.0f, 32, GOLD);

	// Centre fosc
	DrawCircleV(c, R * 0.36f, Color{ 8, 10, 44, 255 });
	float p = 0.5f + 0.5f * sinf(t * 3.0f + cx);
	DrawCircleV(c, R * 0.30f, ColorAlpha(Color{ 60, 120, 255, 255 }, 0.10f + 0.12f * p));
	DrawCircleLines((int)cx, (int)cy, R * 0.30f, ColorAlpha(Color{ 120, 200, 255, 255 }, 0.35f + 0.3f * p));

	// Boleta que dona voltes
	float ba = -t * 2.4f + cx;
	DrawCircleV(Vector2{ cx + cosf(ba) * R * 0.76f, cy + sinf(ba) * R * 0.76f }, R * 0.05f, RAYWHITE);
}

//Traçat d'un arc
static void DrawArchPath(float W, float H, float inset, float cr, float th, Color c)
{
	if (cr < 1.0f) return;
	float x1 = inset + cr;
	float x2 = W - inset - cr;
	float cy = inset + cr;
	float r0 = fmaxf(cr - th * 0.5f, 0.0f);
	float r1 = cr + th * 0.5f;

	DrawRing(Vector2{ x1, cy }, r0, r1, 180.0f, 270.0f, 32, c);
	DrawRing(Vector2{ x2, cy }, r0, r1, 270.0f, 360.0f, 32, c);

	if (x2 > x1) DrawRectangleRec(Rectangle{ x1, inset - th * 0.5f, x2 - x1, th }, c);
	DrawRectangleRec(Rectangle{ inset - th * 0.5f, cy, th, H - cy }, c);
	DrawRectangleRec(Rectangle{ W - inset - th * 0.5f, cy, th, H - cy }, c);
}

// Arc de neó: halo + nucli + centre blanc
static void DrawNeonArch(float W, float H, float inset, float cr, float th, Color col, float pulse)
{
	DrawArchPath(W, H, inset, cr, th + 10.0f, ColorAlpha(col, 0.07f * pulse));
	DrawArchPath(W, H, inset, cr, th + 6.0f, ColorAlpha(col, 0.16f * pulse));
	DrawArchPath(W, H, inset, cr, th + 2.0f, ColorAlpha(col, 0.35f * pulse));
	DrawArchPath(W, H, inset, cr, th, col);
	DrawArchPath(W, H, inset, cr, th * 0.35f, ColorAlpha(WHITE, 0.55f));
}


// ================= CAPES DEL MENÚ PRINCIPAL =================


// Fons blau nit de casino
static void DrawMenuBackground(float W, float H, float t)
{
	DrawRectangleGradientV(0, 0, (int)W, (int)H, Color{ 5, 8, 34, 255 }, Color{ 14, 22, 78, 255 });
	DrawCircleGradient((int)(W * 0.5f), (int)(H * 0.42f), W * 0.95f, Color{ 40, 60, 160, 120 }, Color{ 0, 0, 0, 0 });

	// Focus
	for (int i = 0; i < 3; i++)
	{
		float a = sinf(t * 0.5f + i * 2.1f) * 0.30f + (i - 1) * 0.42f;
		float len = H * 0.75f;
		float spread = 0.16f;
		Vector2 o = { W * 0.5f, -10.0f };
		Vector2 p1 = { o.x + tanf(a - spread) * len, len };
		Vector2 p2 = { o.x + tanf(a + spread) * len, len };
		DrawTriSafe(o, p1, p2, ColorAlpha(GOLD, 0.035f));
	}

	
	float cell = 42.0f;
	Color lat = ColorAlpha(Color{ 90, 120, 255, 255 }, 0.07f);
	for (float x = -H; x < W + H; x += cell)
	{
		DrawLine((int)x, 0, (int)(x + H), (int)H, lat);
		DrawLine((int)(x + H), 0, (int)x, (int)H, lat);
	}

	for (int gy = 0; gy < (int)(H / 84.0f) + 1; gy++)
	{
		for (int gx = 0; gx < (int)(W / 84.0f) + 1; gx++)
		{
			float px = gx * 84.0f + ((gy % 2) ? 42.0f : 0.0f);
			float py = gy * 84.0f;
			float a = 0.05f + 0.07f * (0.5f + 0.5f * sinf(t * 1.5f + px * 0.05f + py * 0.07f));
			DrawPoly(Vector2{ px, py }, 4, 5.0f, 0.0f, ColorAlpha(Color{ 150, 170, 255, 255 }, a));
		}
	}


	for (int i = 0; i < 50; i++)
	{
		float speed = 14.0f + (i % 5) * 8.0f;
		float px = (float)((i * 137) % (int)W);
		float py = H - fmodf((float)(i * 73) + t * speed, H);
		float b = 0.4f + 0.6f * (0.5f + 0.5f * sinf(t * 3.0f + i));
		DrawCircleV(Vector2{ px, py }, 1.5f, ColorAlpha(GOLD, b * 0.55f));
	}

	for (int i = 0; i < 14; i++)
	{
		float px = fmodf(i * 197.0f + 53.0f, W - 80.0f) + 40.0f;
		float py = fmodf(i * 331.0f + 37.0f, H - 220.0f) + 70.0f;
		float s = 3.0f + 7.0f * fmaxf(0.0f, sinf(t * 2.0f + i * 1.3f));
		DrawSparkle(px, py, s, ColorAlpha(Color{ 255, 225, 140, 255 }, 0.85f));
	}


	DrawRectangleGradientH(0, 0, (int)(W * 0.25f), (int)H, Color{ 0, 0, 8, 150 }, Color{ 0, 0, 8, 0 });
	DrawRectangleGradientH((int)(W * 0.75f), 0, (int)(W * 0.25f), (int)H, Color{ 0, 0, 8, 0 }, Color{ 0, 0, 8, 150 });
}

// Ruletes, cartes, fitxes, monedes i un dau
static void DrawMenuDecor(float W, float H, float t, float sx, float sy)
{
	float k = fminf(sx, sy);

	
	DrawRouletteWheel(W * 0.33f, 448.0f * sy, 58.0f * k, fmodf(t * 14.0f, 360.0f), t);
	DrawRouletteWheel(W * 0.67f, 448.0f * sy, 58.0f * k, fmodf(-t * 14.0f, 360.0f), t);
	DrawRouletteWheel(W * 0.50f, 516.0f * sy, 58.0f * k, fmodf(t * 18.0f, 360.0f), t);


	float bob = sinf(t * 1.3f) * 4.0f;
	DrawPlayingCard(W * 0.14f, 450.0f * sy + bob, 52.0f * k, 76.0f * k, -20.0f + sinf(t * 0.9f) * 4.0f, 2);
	DrawPlayingCard(W * 0.185f, 505.0f * sy - bob, 52.0f * k, 76.0f * k, 14.0f + sinf(t * 1.1f) * 4.0f, 1);
	DrawChip(W * 0.11f, 568.0f * sy + sinf(t * 1.7f) * 3.0f, 20.0f * k, Color{ 200, 25, 35, 255 }, t * 20.0f);
	DrawChip(W * 0.20f, 582.0f * sy + sinf(t * 1.4f + 1.0f) * 3.0f, 16.0f * k, Color{ 25, 25, 32, 255 }, -t * 20.0f);

	DrawCoin(W * 0.90f, 420.0f * sy + sinf(t * 1.5f) * 4.0f, 15.0f * k, t * 3.0f);
	DrawCoin(W * 0.86f, 470.0f * sy + sinf(t * 1.2f + 2.0f) * 4.0f, 14.0f * k, t * 2.4f + 1.0f);
	DrawDie(W * 0.885f, 535.0f * sy + sinf(t * 1.1f) * 3.0f, 38.0f * k, 20.0f + sinf(t * 0.8f) * 14.0f, 1 + ((int)(t * 0.8f)) % 6);
	DrawChip(W * 0.80f, 580.0f * sy + sinf(t * 1.5f + 3.0f) * 3.0f, 16.0f * k, Color{ 30, 70, 190, 255 }, t * 15.0f);
}


static void DrawMenuFrame(float W, float H, float t)
{
	float cr = W * 0.28f;
	float pulse = 0.85f + 0.15f * sinf(t * 2.5f);


	DrawRing(Vector2{ cr, cr }, cr, cr * 1.6f, 180.0f, 270.0f, 40, Color{ 26, 3, 12, 255 });
	DrawRing(Vector2{ W - cr, cr }, cr, cr * 1.6f, 270.0f, 360.0f, 40, Color{ 26, 3, 12, 255 });


	DrawArchPath(W, H, 15.5f, cr - 15.5f, 19.0f, Color{ 70, 6, 22, 255 });
	DrawArchPath(W, H, 3.0f, cr - 3.0f, 6.0f, Color{ 26, 3, 12, 255 });


	DrawArchPath(W, H, 6.0f, cr - 6.0f, 3.0f, GOLD);
	DrawNeonArch(W, H, 15.0f, cr - 15.0f, 7.0f, Color{ 255, 45, 45, 255 }, pulse);
	DrawArchPath(W, H, 25.0f, cr - 25.0f, 3.0f, GOLD);
	DrawNeonArch(W, H, 33.0f, cr - 33.0f, 4.0f, Color{ 50, 175, 255, 255 }, pulse);
	DrawArchPath(W, H, 40.0f, cr - 40.0f, 2.0f, ColorAlpha(GOLD, 0.6f));
}


static void DrawMenuTitle(float W, float H, float t, float sx, float sy)
{
	float k = fminf(sx, sy);

	//JACKPOT
	{
		const char* s = "JACKPOT";
		int size = FitFont(s, (int)(W * 0.58f), (int)(60.0f * k));
		int w = MeasureText(s, size);
		float inten = 0.8f + 0.2f * sinf(t * 2.2f);
		DrawNeonText(s, (int)((W - w) * 0.5f), (int)(46.0f * sy), size, Color{ 255, 205, 70, 255 }, Color{ 255, 150, 30, 255 }, inten);
	}

	float mx = W * 0.13f;
	float mw = W * 0.74f;
	float my = 126.0f * sy;
	float mh = 150.0f * sy;
	float pulse = 0.5f + 0.5f * sinf(t * 3.0f);

	DrawRectangle((int)(mx - 18), (int)(my - 18), (int)(mw + 36), (int)(mh + 36), ColorAlpha(RED, 0.08f + 0.06f * pulse));
	DrawRectangle((int)(mx - 10), (int)(my - 10), (int)(mw + 20), (int)(mh + 20), Color{ 110, 12, 24, 255 });
	DrawRectangleLinesEx(Rectangle{ mx - 10, my - 10, mw + 20, mh + 20 }, 2.0f, GOLD);
	DrawRectangleGradientV((int)mx, (int)my, (int)mw, (int)mh, Color{ 28, 24, 84, 255 }, Color{ 6, 8, 34, 255 });
	DrawRectangleLinesEx(Rectangle{ mx, my, mw, mh }, 2.0f, GOLD);
	DrawRectangleLinesEx(Rectangle{ mx + 5, my + 5, mw - 10, mh - 10 }, 1.0f, ColorAlpha(GOLD, 0.4f));
	DrawBulbRect(mx - 5.0f, my - 5.0f, mw + 10.0f, mh + 10.0f, 17.0f, t, YELLOW);

	float cs = 12.0f;
	DrawRectangle((int)(mx - 16), (int)(my - 16), (int)cs, (int)cs, GOLD);
	DrawRectangle((int)(mx + mw + 4), (int)(my - 16), (int)cs, (int)cs, GOLD);
	DrawRectangle((int)(mx - 16), (int)(my + mh + 4), (int)cs, (int)cs, GOLD);
	DrawRectangle((int)(mx + mw + 4), (int)(my + mh + 4), (int)cs, (int)cs, GOLD);

	//CASINO
	{
		const char* s = "CASINO";
		int size = FitFont(s, (int)(mw * 0.86f), (int)(100.0f * k));
		int w = MeasureText(s, size);
		int x = (int)(mx + (mw - w) * 0.5f);
		int y = (int)(my + 6.0f * sy);


		float fl = (sinf(t * 23.0f) > 0.97f && sinf(t * 1.7f) > 0.6f) ? 0.45f : 1.0f;

	
		Color gold = ColorAlpha(GOLD, fl);
		DrawText(s, x - 2, y, size, gold);
		DrawText(s, x + 2, y, size, gold);
		DrawText(s, x, y - 2, size, gold);
		DrawText(s, x, y + 2, size, gold);

		DrawNeonText(s, x, y, size, ColorAlpha(Color{ 255, 100, 35, 255 }, fl), Color{ 255, 60, 20, 255 }, fl * (0.8f + 0.2f * sinf(t * 3.0f)));

		float shineP = fmodf(t * 0.45f, 1.6f) - 0.3f;
		float s0 = fmaxf(mx + shineP * mw, mx);
		float s1 = fminf(mx + shineP * mw + 34.0f, mx + mw);
		if (s1 > s0)
		{
			BeginScissorMode((int)s0, (int)my, (int)(s1 - s0), (int)mh);
			DrawText(s, x, y, size, ColorAlpha(WHITE, 0.75f));
			EndScissorMode();
		}

		// LAS VEGAS
		const char* lv = "LAS VEGAS";
		int lsize = (int)(20.0f * k);
		int lw = MeasureText(lv, lsize);
		int ly = (int)(my + mh - 32.0f * sy);
		int lx = (int)(mx + (mw - lw) * 0.5f);
		DrawText(lv, lx + 1, ly + 1, lsize, Color{ 0, 0, 0, 180 });
		DrawText(lv, lx, ly, lsize, Color{ 255, 220, 120, 255 });
		float lyc = (float)ly + lsize * 0.5f;
		DrawRectangle((int)(mx + 24), (int)lyc, (int)(lx - mx - 40), 1, ColorAlpha(GOLD, 0.7f));
		DrawRectangle(lx + lw + 16, (int)lyc, (int)(mx + mw - 24 - (lx + lw + 16)), 1, ColorAlpha(GOLD, 0.7f));
		DrawCircleV(Vector2{ (float)(lx - 10), lyc }, 2.5f, GOLD);
		DrawCircleV(Vector2{ (float)(lx + lw + 10), lyc }, 2.5f, GOLD);
	}

	//Cinta PINBALL
	float rx = W * 0.2f;
	float rw = W * 0.6f;
	float ry = my + mh + 22.0f * sy;
	float rh = 46.0f * sy;

	// Cues de la cinta
	DrawTriSafe(Vector2{ rx - 30.0f, ry + rh * 0.5f }, Vector2{ rx + 2.0f, ry + 6.0f }, Vector2{ rx + 2.0f, ry + rh - 6.0f }, Color{ 170, 118, 20, 255 });
	DrawTriSafe(Vector2{ rx + rw + 30.0f, ry + rh * 0.5f }, Vector2{ rx + rw - 2.0f, ry + 6.0f }, Vector2{ rx + rw - 2.0f, ry + rh - 6.0f }, Color{ 170, 118, 20, 255 });
	DrawTriSafe(Vector2{ rx - 20.0f, ry + rh * 0.5f }, Vector2{ rx + 2.0f, ry + 12.0f }, Vector2{ rx + 2.0f, ry + rh - 12.0f }, Color{ 110, 8, 20, 255 });
	DrawTriSafe(Vector2{ rx + rw + 20.0f, ry + rh * 0.5f }, Vector2{ rx + rw - 2.0f, ry + 12.0f }, Vector2{ rx + rw - 2.0f, ry + rh - 12.0f }, Color{ 110, 8, 20, 255 });

	DrawRectangle((int)rx, (int)ry, (int)rw, (int)rh, GOLD);
	DrawRectangleGradientV((int)rx + 3, (int)ry + 3, (int)rw - 6, (int)rh - 6, Color{ 160, 14, 30, 255 }, Color{ 90, 6, 18, 255 });
	DrawRectangleLinesEx(Rectangle{ rx + 6, ry + 6, rw - 12, rh - 12 }, 1.0f, ColorAlpha(GOLD, 0.7f));

	{
		const char* s = "P I N B A L L";
		int size = FitFont(s, (int)(rw * 0.82f), (int)(30.0f * k));
		int w = MeasureText(s, size);
		DrawNeonText(s, (int)(rx + (rw - w) * 0.5f), (int)(ry + (rh - size) * 0.5f), size, Color{ 255, 240, 190, 255 }, Color{ 255, 200, 60, 255 }, 0.7f + 0.3f * sinf(t * 2.6f + 1.0f));
	}

	//Tres caselles lluminoses
	int lit = (int)fmodf(t * 1.5f, 3.0f);
	for (int i = 0; i < 3; i++)
	{
		float bw = 46.0f * sx, bh = 22.0f * sy;
		float bx = W * 0.5f + (i - 1) * (bw + 14.0f) - bw * 0.5f;
		float by = ry + rh + 20.0f * sy;
		bool on = (i == lit);

		DrawRectangle((int)bx, (int)by, (int)bw, (int)bh, on ? Color{ 40, 150, 255, 255 } : Color{ 14, 22, 70, 255 });
		if (on) DrawRectangle((int)(bx - 6), (int)(by - 6), (int)(bw + 12), (int)(bh + 12), ColorAlpha(Color{ 60, 170, 255, 255 }, 0.22f));
		DrawRectangleLinesEx(Rectangle{ bx, by, bw, bh }, 2.0f, GOLD);
		DrawRectangleLinesEx(Rectangle{ bx + 3, by + 3, bw - 6, bh - 6 }, 1.0f, ColorAlpha(GOLD, 0.35f));
	}


	DrawGoldScroll(W * 0.115f, ry + rh * 0.5f, 18.0f * k, 1.0f);
	DrawGoldScroll(W * 0.885f, ry + rh * 0.5f, 18.0f * k, -1.0f);


	for (int i = 0; i < 6; i++)
	{
		float px = (i % 2 == 0) ? mx - 24.0f : mx + mw + 24.0f;
		float py = my + (i / 2) * (mh * 0.5f) + 6.0f;
		float s = 4.0f + 7.0f * fmaxf(0.0f, sinf(t * 3.0f + i * 1.1f));
		DrawSparkle(px, py, s, Color{ 255, 235, 160, 255 });
	}
}

// Placa "PREM ENTER" i tauler de controls
static void DrawMenuPrompts(float W, float H, float t, float sx, float sy)
{
	float k = fminf(sx, sy);

	//PREM ENTER
	float px = W * 0.2f;
	float pw = W * 0.6f;
	float py = 592.0f * sy;
	float ph = 54.0f * sy;
	float pulse = 0.5f + 0.5f * sinf(t * 2.8f);

	DrawRectangleRounded(Rectangle{ px - 8, py - 8, pw + 16, ph + 16 }, 0.5f, 16, ColorAlpha(RED, 0.15f + 0.15f * pulse));
	DrawRectangleRounded(Rectangle{ px - 3, py - 3, pw + 6, ph + 6 }, 0.5f, 16, GOLD);
	DrawRectangleRounded(Rectangle{ px, py, pw, ph }, 0.5f, 16, Color{ 10, 12, 46, 255 });
	DrawRectangleRounded(Rectangle{ px + 6, py + 4, pw - 12, ph * 0.42f }, 0.5f, 16, ColorAlpha(WHITE, 0.06f));

	{
		const char* s = "PREM ENTER";
		int size = FitFont(s, (int)(pw * 0.70f), (int)(34.0f * k));
		int w = MeasureText(s, size);
		DrawNeonText(s, (int)(px + (pw - w) * 0.5f), (int)(py + (ph - size) * 0.5f), size, Color{ 255, 245, 200, 255 }, Color{ 255, 190, 40, 255 }, 0.6f + 0.4f * pulse);
	}

	// Fletxes laterals
	float ay = py + ph * 0.5f;
	float off = sinf(t * 5.0f) * 3.0f;
	DrawTriSafe(Vector2{ px + 18.0f + off, ay - 10.0f }, Vector2{ px + 18.0f + off, ay + 10.0f }, Vector2{ px + 32.0f + off, ay }, GOLD);
	DrawTriSafe(Vector2{ px + pw - 18.0f - off, ay - 10.0f }, Vector2{ px + pw - 18.0f - off, ay + 10.0f }, Vector2{ px + pw - 32.0f - off, ay }, GOLD);

	//Tauler de controls (tecla a l'esquerra, acció a la dreta)
	float cx = W * 0.10f;
	float cw = W * 0.80f;
	float cy = 676.0f * sy;
	float ch = H - 34.0f * sy - cy;

	float cabPulse = 0.5f + 0.5f * sinf(t * 3.0f);
	DrawRectangle((int)(cx - 16), (int)(cy - 16), (int)(cw + 32), (int)(ch + 32), ColorAlpha(RED, 0.08f + 0.06f * cabPulse));
	DrawRectangle((int)(cx - 10), (int)(cy - 10), (int)(cw + 20), (int)(ch + 20), Color{ 110, 12, 24, 255 });
	DrawRectangleLinesEx(Rectangle{ cx - 10, cy - 10, cw + 20, ch + 20 }, 2.0f, GOLD);
	DrawRectangleGradientV((int)cx, (int)cy, (int)cw, (int)ch, Color{ 26, 32, 96, 255 }, Color{ 8, 10, 38, 255 });
	DrawRectangleLinesEx(Rectangle{ cx, cy, cw, ch }, 2.0f, GOLD);
	DrawRectangleLinesEx(Rectangle{ cx + 4, cy + 4, cw - 8, ch - 8 }, 1.0f, ColorAlpha(GOLD, 0.45f));
	DrawBulbRect(cx - 5.0f, cy - 5.0f, cw + 10.0f, ch + 10.0f, 16.0f, t, YELLOW);

	const char* title = "CONTROLS";
	int tfs = (int)(20.0f * k);
	int tw = MeasureText(title, tfs);
	int ty = (int)(cy + 12.0f * sy);
	DrawText(title, (int)(W * 0.5f - tw * 0.5f) + 1, ty + 1, tfs, Color{ 0, 0, 0, 180 });
	DrawText(title, (int)(W * 0.5f - tw * 0.5f), ty, tfs, GOLD);
	DrawRectangle((int)(cx + 24), ty + tfs + 6, (int)(cw - 48), 1, ColorAlpha(GOLD, 0.6f));

	const int rows = 6;
	const char* keys[rows] = { "ESQUERRA / DRETA", "AVALL", "F1", "F2 / F3", "F4 / F5", "F6 / F7" };
	const char* actions[rows] = {
		"Flippers",
		"Llan\xC3\xA7" "ador",
		"Mode depuraci\xC3\xB3",
		"FPS objectiu",
		"Gravetat",
		"Rebot dels bumpers"
	};

	int fs = (int)(16.0f * k);
	float rowsTop = (float)(ty + tfs + 16);
	float lh = (cy + ch - 12.0f * sy - rowsTop) / (float)rows;
	float colX = W * 0.5f + 10.0f * sx;
	Color soft = { 200, 212, 255, 255 };

	for (int i = 0; i < rows; i++)
	{
		int ry = (int)(rowsTop + lh * i + (lh - fs) * 0.5f);
		int kw = MeasureText(keys[i], fs);
		DrawText(keys[i], (int)(colX - 16.0f - kw), ry, fs, Color{ 255, 220, 120, 255 });
		DrawCircleV(Vector2{ colX, ry + fs * 0.5f }, 2.5f, GOLD);
		DrawText(actions[i], (int)(colX + 16.0f), ry, fs, soft);
	}
}

// ============================================================
// ======================== MODULEGAME ========================

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleGame::~ModuleGame()
{
}

bool ModuleGame::Start()
{
	LOG("Loading game assets");

	SetRandomSeed((unsigned int)time(NULL));

	boardTex = LoadTexture("Assets/Background.png");

	if (boardTex.id == 0)
	{
		TraceLog(LOG_ERROR, "No s'ha pogut carregar Assets/Background.png");
	}

	SetTargetFPS(targetFps);

	//GEOMETRIA DEL TAULER

	// 1. Contorn exterior
	int outerWalls[34] = {
		19, 215,   34, 153,   64, 104,   106, 64,   158, 34,
		210, 18,   265, 13,   317, 10,   365, 14,   426, 24,
		472, 46,   495, 64,   538, 104,  555, 137,  578, 193,
		578, 890,  19, 890
	};
	App->physics->CreateSolidChain(0, 0, outerWalls, 34, 10.0f);

	// 2. Paret lateral esquerra
	int leftWall[110] = {
		22, 537,   19, 221,   23, 201,   27, 186,   33, 170,
		40, 155,   49, 140,   61, 125,   71, 111,   81, 98,
		92, 87,    104, 75,   120, 63,   133, 55,   148, 44,
		162, 34,   165, 86,   160, 97,   154, 105,  140, 113,
		130, 117,  117, 125,  107, 130,  96, 140,   86, 151,
		76, 162,   67, 174,   60, 187,   55, 197,   49, 206,
		46, 218,   43, 229,   43, 239,   41, 249,   41, 257,
		41, 267,   42, 279,   46, 285,   50, 296,   52, 308,
		54, 321,   58, 334,   61, 346,   65, 359,   68, 369,
		86, 408,   90, 419,   96, 428,   103, 439,  108, 448,
		114, 456,  118, 460,  117, 507,  110, 513,  28, 538
	};
	App->physics->CreateSolidChain(0, 0, leftWall, 110, 10.0f);

	// 3. Paret lateral dreta
	int rightWall[88] = {
		422, 96,   427, 102,  437, 108,  446, 112,  456, 122,
		466, 134,  474, 146,  479, 161,  483, 171,  487, 185,
		490, 197,  490, 206,  490, 218,  489, 464,  487, 475,
		485, 485,  482, 497,  476, 505,  469, 511,  462, 518,
		467, 525,  478, 528,  489, 533,  502, 539,  510, 542,
		519, 546,  528, 549,  537, 554,  539, 543,  507, 527,
		506, 509,  506, 195,  505, 181,  503, 170,  500, 158,
		495, 147,  489, 134,  483, 123,  473, 113,  462, 103,
		453, 94,   442, 87,   433, 82,   423, 85
	};
	App->physics->CreateSolidChain(0, 0, rightWall, 88, 10.0f);

	// 4. Paret del carril de llançament
	// Al dibuix acaba una mica més amunt, però allà el carril és més estret que la bola i s'hi encallaria
	int laneWall[8] = { 538, 205,  544, 205,  544, 840,  538, 840 };
	App->physics->CreateSolidChain(0, 0, laneWall, 8, 8.0f);

	// 5. Mur del desguàs dret
	int rightDrainWall[8] = { 355, 885,  355, 818,  541, 704,  539, 883 };
	App->physics->CreateSolidChain(0, 0, rightDrainWall, 8, 10.0f);

	// 6. Mur del desguàs esquerre
	int leftDrainWall[8] = { 208, 886,  207, 818,  20, 711,  18, 882 };
	App->physics->CreateSolidChain(0, 0, leftDrainWall, 8, 10.0f);

	// 7. Guia lateral dreta
	int rightRamp[20] = {
		393, 755,  474, 705,  482, 690,  481, 583,  477, 665,
		470, 677,  455, 689,  380, 733,  388, 734,  396, 741
	};
	App->physics->CreateSolidChain(0, 0, rightRamp, 20, INLANE_WALL_THICKNESS);

	// 8. Guia lateral esquerra
	int leftRamp[18] = {
		167, 755,  80, 704,  73, 694,  72, 584,  77, 675,
		83, 681,  179, 737,  172, 741,  168, 748
	};
	App->physics->CreateSolidChain(0, 0, leftRamp, 18, INLANE_WALL_THICKNESS);

	// 9. Slingshot dret
	int rightSlingshot[20] = {
		435, 557,  374, 686,  375, 693,  381, 694,  387, 694,
		435, 668,  441, 662,  442, 651,  442, 559,  437, 554
	};
	App->physics->CreateSolidChain(0, 0, rightSlingshot, 20, INLANE_WALL_THICKNESS);

	// 10. Slingshot esquerre
	int leftSlingshot[20] = {
		165, 696,  173, 694,  176, 685,  119, 557,  113, 554,
		109, 558,  108, 657,  112, 663,  117, 668,  163, 694
	};
	App->physics->CreateSolidChain(0, 0, leftSlingshot, 20, INLANE_WALL_THICKNESS);

	
	int safetyBox[8] = { 0, 0,  SCREEN_WIDTH, 0,  SCREEN_WIDTH, SCREEN_HEIGHT,  0, SCREEN_HEIGHT };
	App->physics->CreateSolidChain(0, 0, safetyBox, 8, 10.0f);


	// Flippers
	flipperL = App->physics->CreateFlipper(FLIPPER_LEFT_X, FLIPPER_LEFT_Y, 78, 16, true, FLIPPER_REST_ANGLE, FLIPPER_UP_ANGLE);
	flipperR = App->physics->CreateFlipper(FLIPPER_RIGHT_X, FLIPPER_RIGHT_Y, 78, 16, false, FLIPPER_REST_ANGLE, FLIPPER_UP_ANGLE);

	// Llançador
	kicker = App->physics->CreateKicker(557, 847, 20, 8, 30);

	// Sensor inferior: la bola s'ha perdut pel desguàs
	bottomSensor = App->physics->CreateRectangleSensor(270, 892, 500, 20);
	bottomSensor->listener = this;

	// So
	bumperFx = App->audio->LoadFx("Assets/bonus.wav");

	// Objectius
	int targetX[3] = { 235, 290, 346 };
	for (int i = 0; i < 3; ++i)
	{
		PhysBody* t = App->physics->CreateRectangleSensor(targetX[i], 170, 40, 24);
		t->listener = this;
		targets.push_back(t);
	}

	// Carril HIGH ROLLER
	highRollerSensor = App->physics->CreateRectangleSensor(HIGH_ROLLER_X, HIGH_ROLLER_Y, HIGH_ROLLER_W, HIGH_ROLLER_H);
	highRollerSensor->listener = this;

	// Bumpers
	int bumperPos[3][2] = { {206, 279}, {373, 279}, {289, 379} };
	for (int i = 0; i < 3; ++i)
	{
		PhysBody* b = App->physics->CreateCircle(bumperPos[i][0], bumperPos[i][1], BUMPER_RADIUS, b2_staticBody);
		b->body->GetFixtureList()->SetRestitution(bumperRestitution);
		b->listener = this;
		bumpers.push_back(b);
		bumperFlashTimers.push_back(0.0f);
	}

	return true;
}

void ModuleGame::SpawnBall()
{
	ball = App->physics->CreateCircle(555, 845, 10);
	ball->body->SetBullet(true);
	ball->body->SetLinearDamping(0.1f);
	ball->body->SetAngularDamping(0.1f);
}

void ModuleGame::StartRound()
{
	for (int i = 0; i < 3; ++i) targetLit[i] = false;
	score = 0;
	ballsLeft = 3;
	gameOver = false;
	SpawnBall();
}

void ModuleGame::EndRound()
{
	previousScore = lastScore;
	lastScore = score;
	if (score > highScore) highScore = score;
	gameOver = true;
	gameOverTime = GetTime();
}

void ModuleGame::DrawGameOver()
{
	const float W = (float)SCREEN_WIDTH;
	const float H = (float)SCREEN_HEIGHT;
	float t = (float)(GetTime() - gameOverTime);

	DrawRectangle(0, 0, (int)W, (int)H, Color{ 0, 0, 10, 170 });

	float pw = 380.0f;
	float ph = 330.0f;
	float px = (W - pw) * 0.5f;
	float py = (H - ph) * 0.5f;

	DrawRectangle((int)(px - 6), (int)(py - 6), (int)(pw + 12), (int)(ph + 12), Color{ 110, 12, 24, 255 });
	DrawRectangleLinesEx(Rectangle{ px - 6, py - 6, pw + 12, ph + 12 }, 2.0f, GOLD);
	DrawRectangleGradientV((int)px, (int)py, (int)pw, (int)ph, Color{ 26, 32, 96, 255 }, Color{ 8, 10, 38, 255 });
	DrawRectangleLinesEx(Rectangle{ px, py, pw, ph }, 2.0f, GOLD);

	const char* title = "GAME OVER";
	int tsize = 50;
	int tw = MeasureText(title, tsize);
	DrawNeonText(title, (int)(px + (pw - tw) * 0.5f), (int)(py + 28), tsize, Color{ 255, 70, 60, 255 }, Color{ 255, 40, 40, 255 }, 0.8f + 0.2f * sinf(t * 3.0f));

	const char* labels[3] = { "Puntuaci\xC3\xB3", "Anterior", "Millor" };
	int values[3] = { lastScore, previousScore, highScore };
	int fs = 26;
	float rowY = py + 112.0f;

	for (int i = 0; i < 3; i++)
	{
		int y = (int)(rowY + i * 42.0f);
		Color c = (i == 0) ? WHITE : Color{ 200, 212, 255, 255 };
		DrawText(labels[i], (int)(px + 40), y, fs, c);
		const char* v = TextFormat("%d", values[i]);
		DrawText(v, (int)(px + pw - 40 - MeasureText(v, fs)), y, fs, (i == 2) ? GOLD : c);
	}

	DrawRectangle((int)(px + 30), (int)(py + ph - 82), (int)(pw - 60), 1, ColorAlpha(GOLD, 0.6f));

	const char* opt1 = "ENTER: tornar a jugar";
	const char* opt2 = "M: men\xC3\xBA principal";
	int ofs = 20;
	float blink = 0.6f + 0.4f * sinf(t * 4.0f);
	DrawText(opt1, (int)(px + (pw - MeasureText(opt1, ofs)) * 0.5f), (int)(py + ph - 66), ofs, ColorAlpha(YELLOW, blink));
	DrawText(opt2, (int)(px + (pw - MeasureText(opt2, ofs)) * 0.5f), (int)(py + ph - 38), ofs, LIGHTGRAY);
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	if (bodyA == bottomSensor && bodyB == ball)
	{
		ballLost = true;
		return;
	}

	// La velocitat no es pot canviar durant el pas de la física: es dispara a Update()
	if (bodyA == highRollerSensor && bodyB == ball && highRollerCooldown <= 0.0f)
	{
		highRollerHit = true;
		return;
	}

	if (bodyB == ball)
	{
		for (size_t i = 0; i < targets.size(); ++i)
		{
			if (bodyA == targets[i] && !targetLit[i])
			{
				targetLit[i] = true;
				score += 50;
				App->audio->PlayFx(bumperFx);

				if (targetLit[0] && targetLit[1] && targetLit[2])
				{
					score += 1000;
					ballsLeft++;
					for (int k = 0; k < 3; ++k) targetLit[k] = false;
				}
				return;
			}
		}
	}


	if (bodyB == ball && std::find(bumpers.begin(), bumpers.end(), bodyA) != bumpers.end())
	{
		score += 100;
		App->audio->PlayFx(bumperFx);

		for (size_t i = 0; i < bumpers.size(); ++i)
		{
			if (bodyA == bumpers[i])
			{
				bumperFlashTimers[i] = 0.2f;
				break;
			}
		}
	}
}

void ModuleGame::LaunchFromHighRoller()
{
	highRollerHit = false;
	if (ball == nullptr) return;

	ball->body->SetLinearVelocity(b2Vec2(0.0f, -HIGH_ROLLER_LAUNCH_SPEED));
	ball->body->SetAngularVelocity(0.0f);

	score += HIGH_ROLLER_POINTS;
	App->audio->PlayFx(bumperFx);

	// Mentre la bola puja pel carril no torna a comptar
	highRollerCooldown = 0.8f;
	highRollerFlash = 0.8f;
}


void ModuleGame::DrawHighRollerFlash()
{
	if (highRollerFlash <= 0.0f) return;

	float a = highRollerFlash / 0.8f;
	DrawRectangleRounded(Rectangle{ 507.0f, 222.0f, 31.0f, 300.0f }, 0.5f, 8, ColorAlpha(Color{ 255, 80, 60, 255 }, 0.35f * a));
	DrawRectangleRoundedLines(Rectangle{ 507.0f, 222.0f, 31.0f, 300.0f }, 0.5f, 8, ColorAlpha(GOLD, a));

	const char* text = TextFormat("+%d", HIGH_ROLLER_POINTS);
	int size = 26;
	float rise = (1.0f - a) * 40.0f;
	DrawText(text, 470 - MeasureText(text, size) / 2 + 1, (int)(470.0f - rise) + 1, size, ColorAlpha(BLACK, a));
	DrawText(text, 470 - MeasureText(text, size) / 2, (int)(470.0f - rise), size, ColorAlpha(GOLD, a));
}

bool ModuleGame::CleanUp()
{
	LOG("Unloading game");
	UnloadTexture(boardTex);
	return true;
}

update_status ModuleGame::Update()
{
	// 1) Fons
	if (boardTex.id != 0)
	{
		DrawTexturePro(
			boardTex,
			Rectangle{ 0.0f, 0.0f, (float)boardTex.width, (float)boardTex.height },
			Rectangle{ 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
			Vector2{ 0.0f, 0.0f },
			0.0f,
			WHITE
		);
	}
	else
	{
		DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, DARKGRAY);
	}

	// 2) Pantalla de títol
	if (inMenu)
	{
		if (IsKeyPressed(KEY_ENTER))
		{
			inMenu = false;
			menuTimerStarted = false;
			StartRound();
		}

		// Temporitzador propi del menú (es reinicia cada cop que s'hi entra)
		if (!menuTimerStarted)
		{
			menuTimer.Start();
			menuTimerStarted = true;
		}
		float t = (float)menuTimer.ReadSec();

		// El disseny està pensat per a 600x900; tot s'escala amb aquestes proporcions
		const float W = (float)SCREEN_WIDTH;
		const float H = (float)SCREEN_HEIGHT;
		const float sx = W / 600.0f;
		const float sy = H / 900.0f;

		// ================ AMBIENT DEL TAULER (MENÚ) =================
	
		DrawMenuBackground(W, H, t);
		DrawMenuDecor(W, H, t, sx, sy);
		DrawMenuFrame(W, H, t);
		DrawMenuTitle(W, H, t, sx, sy);
		DrawMenuPrompts(W, H, t, sx, sy);

		return UPDATE_CONTINUE;
	}

	// --- BOLA, FLIPPERS I BUMPERS ---

	if (ball)
	{
		int bx, by;
		ball->GetPosition(bx, by);
		DrawCircle(bx, by, 10, LIGHTGRAY);
		DrawCircleLines(bx, by, 10, DARKGRAY);
		DrawCircle(bx - 3, by - 3, 3, RAYWHITE);
	}

	DrawFlipperVisual(flipperL, true);
	DrawFlipperVisual(flipperR, false);


	for (size_t i = 0; i < bumpers.size(); ++i)
	{
		if (bumperFlashTimers[i] <= 0.0f) continue;

		int bx, by;
		bumpers[i]->GetPosition(bx, by);
		float a = bumperFlashTimers[i] / 0.2f;
		DrawCircle(bx, by, (float)BUMPER_RADIUS, ColorAlpha(YELLOW, 0.25f * a));
		DrawRing(Vector2{ (float)bx, (float)by }, BUMPER_RADIUS - 4.0f, (float)BUMPER_RADIUS, 0.0f, 360.0f, 48, ColorAlpha(GOLD, a));
		bumperFlashTimers[i] -= GetFrameTime();
	}

	if (highRollerCooldown > 0.0f) highRollerCooldown -= GetFrameTime();
	if (highRollerFlash > 0.0f) highRollerFlash -= GetFrameTime();
	if (highRollerHit) LaunchFromHighRoller();
	DrawHighRollerFlash();

	if (IsKeyPressed(KEY_F2) && targetFps > 10) { targetFps -= 10; SetTargetFPS(targetFps); }
	if (IsKeyPressed(KEY_F3) && targetFps < 240) { targetFps += 10; SetTargetFPS(targetFps); }

	float g = App->physics->GetGravity();
	if (IsKeyPressed(KEY_F4) && g > 0.0f)  App->physics->SetGravity(g - 1.0f);
	if (IsKeyPressed(KEY_F5) && g < 30.0f) App->physics->SetGravity(g + 1.0f);

	bool restitutionChanged = false;
	if (IsKeyPressed(KEY_F6) && bumperRestitution > 0.1f) { bumperRestitution -= 0.1f; restitutionChanged = true; }
	if (IsKeyPressed(KEY_F7) && bumperRestitution < 2.0f) { bumperRestitution += 0.1f; restitutionChanged = true; }
	if (restitutionChanged)
		for (PhysBody* b : bumpers)
			b->body->GetFixtureList()->SetRestitution(bumperRestitution);

	for (size_t i = 0; i < targets.size(); ++i)
	{
		int x, y;
		targets[i]->GetPosition(x, y);
		if (targetLit[i]) DrawRectangle(x - 20, y - 12, 40, 24, Color{ 0, 255, 0, 150 });
	}

	if (gameOver)
	{
		if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
		{
			StartRound();
		}
		else if (IsKeyPressed(KEY_M))
		{
			gameOver = false;
			inMenu = true;
			return UPDATE_CONTINUE;
		}
	}
	else if (ballLost)
	{
		App->physics->DestroyBody(ball);
		ball = nullptr;
		ballLost = false;

		ballsLeft--;
		if (ballsLeft > 0) SpawnBall();
		else EndRound();
	}

	if (IsKeyPressed(KEY_ONE))
		App->physics->CreateCircle(GetMouseX(), GetMouseY(), 12);

	flipperL->SetMotorSpeed(IsKeyDown(KEY_LEFT) ? -15.0f : 15.0f);
	flipperR->SetMotorSpeed(IsKeyDown(KEY_RIGHT) ? 15.0f : -15.0f);
	kicker->SetMotorSpeed(IsKeyDown(KEY_DOWN) ? 3.0f : -25.0f);

	DrawText(TextFormat("Puntuaci\xC3\xB3: %d", score), 10, 40, 20, WHITE);
	DrawText(TextFormat("Anterior: %d", lastScore), 10, 65, 20, LIGHTGRAY);
	DrawText(TextFormat("Millor: %d", highScore), 10, 90, 20, LIGHTGRAY);
	DrawText(TextFormat("Boles: %d", ballsLeft), 10, 115, 20, WHITE);
	DrawText(TextFormat("FPS objectiu: %d (F2/F3)", targetFps), 10, 150, 14, SKYBLUE);
	DrawText(TextFormat("Gravetat: %.1f (F4/F5)", App->physics->GetGravity()), 10, 168, 14, SKYBLUE);
	DrawText(TextFormat("Rebot bumpers: %.1f (F6/F7)", bumperRestitution), 10, 186, 14, SKYBLUE);

	if (gameOver)
	{
		DrawGameOver();
	}

	return UPDATE_CONTINUE;
}
