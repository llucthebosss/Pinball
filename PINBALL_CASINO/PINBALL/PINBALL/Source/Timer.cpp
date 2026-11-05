// ----------------------------------------------------
// Timer.cpp
// Temporitzador ràpid amb precisió de mil·lisegons
// ----------------------------------------------------

#include "Timer.h"

#include "raylib.h"

Timer::Timer()
{
	Start();
}

void Timer::Start()
{
	started_at = GetTime();
}

double Timer::ReadSec() const
{
	return (GetTime() - started_at);
}