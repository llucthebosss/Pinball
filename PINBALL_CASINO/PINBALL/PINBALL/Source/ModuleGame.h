#pragma once

#include "Globals.h"
#include "Module.h"
#include "p2Point.h"
#include "raylib.h"
#include "ModulePhysics.h"
#include "Timer.h"
#include <vector>

class PhysBody;

class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start();
	update_status Update();
	bool CleanUp();

	void OnCollision(PhysBody* bodyA, PhysBody* bodyB) override;

private:
	b2RevoluteJoint* flipperL = nullptr;
	b2RevoluteJoint* flipperR = nullptr;
	b2PrismaticJoint* kicker = nullptr;

	PhysBody* ball = nullptr;
	PhysBody* bottomSensor = nullptr;
	bool ballLost = false;
	void SpawnBall();

	// Carril HIGH ROLLER: si la bola hi cau, surt disparada cap amunt i dona punts
	PhysBody* highRollerSensor = nullptr;
	bool highRollerHit = false;
	float highRollerCooldown = 0.0f;
	float highRollerFlash = 0.0f;
	void LaunchFromHighRoller();
	void DrawHighRollerFlash();

	std::vector<float> bumperFlashTimers;

	std::vector<PhysBody*> bumpers;
	std::vector<PhysBody*> targets;
	bool targetLit[3] = { false, false, false };
	unsigned int bumperFx = 0;

	Texture2D boardTex = { 0 };

	Timer menuTimer;
	bool menuTimerStarted = false;

	int targetFps = 60;
	float bumperRestitution = 1.2f;

	int score = 0;
	int lastScore = 0;
	int previousScore = 0;
	int highScore = 0;
	int ballsLeft = 3;
	bool gameOver = false;
	double gameOverTime = 0.0;
	bool inMenu = true;

	void StartRound();
	void EndRound();
	void DrawGameOver();
};
