// ----------------------------------------------------
// Timer.cpp
// Body for CPU Tick Timer class
// ----------------------------------------------------

#include "Timer.h"
#include <ctime>
// ---------------------------------------------
Timer::Timer()
{
	Start();
}

// ---------------------------------------------
void Timer::Start()
{
	reset = false;
	running = true;
	started_at = std::clock();
	time_paused = started_at;
	stopped_at = 0;
}

// ---------------------------------------------
void Timer::Reset()
{
	reset = true;
}

void Timer::Pause() {
	stopped_at = std::clock();
	running = false;
}

void Timer::Resume() {
	time_paused += (std::clock() - stopped_at);
	running = true;
}

// ---------------------------------------------
int Timer::Read() const
{
	if (!reset)
	{
		if (running)
			return std::clock() - time_paused;
		else
			return stopped_at;
	}
	else
		return 0;
}

float Timer::ReadSeconds() const {
	return (float)((std::clock() - time_paused)/1000);
}
