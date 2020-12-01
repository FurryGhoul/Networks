#ifndef __TIMER_H__
#define __TIMER_H__


class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	void Reset();
	void Pause();
	void Resume();

	int Read() const; // 
	float ReadSeconds() const;

private:

	bool	running = false;
	bool	reset = false;
	float	started_at;
	float	stopped_at;
	float   time_paused;
};

#endif //__TIMER_H__