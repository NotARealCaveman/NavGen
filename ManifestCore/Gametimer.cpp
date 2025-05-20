#include "Gametimer.h"

using namespace Manifest_Core;

GameTimer::GameTimer()
	:timer{}, update{ 16.67ms }
{
	timer.stop = timer.start + update;
};
GameTimer::GameTimer(const Timer::Duration& _update)
	:timer{}, update{ _update }
{		
};

void GameTimer::SetRunTime(const Timer::Duration& _update)
{
	update = _update;
}

void GameTimer::Update()
{
	//get current timer
	Timer::Duration prev = timer.elapsed;
	StepTimer(timer);
	dt = timer.elapsed-prev;	
	//add prediction to frame stop from current time
	timer.stop += update;
}

void GameTimer::Sleep() const
{
	//check if step predicition is less than current time, if so sleep
	//if frame is out of time just exit and begin next frame, slowpoke
	auto prediction = Timer::Duration{ timer.stop.time_since_epoch() };
	auto now = Timer::Duration{ Timer::Clock::now().time_since_epoch() };			
	if (prediction > now)
	{
		//RLOG(std::this_thread::get_id()<<"sleeping for: " << (prediction - now).count());
		std::this_thread::sleep_until(timer.stop);
	}	
}