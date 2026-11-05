#pragma once

#include "Globals.h"
#include "Timer.h"

class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	double ReadSec() const;

private:

	// Moment en què s'ha engegat, en segons
	double started_at;
};