#include "Timer.h"

using namespace Manifest_Core;

Timer::Timer()
	:start{ Clock::now() }, elapsed{}, stop{ TimePoint::max() }
{
}

Timer::Timer(const Duration& timeLimit)
	: start{ Clock::now() }, elapsed{}, stop{ Clock::now() + timeLimit }
{

}

void Manifest_Core::StepTimer(Timer& timer)
{
	Timer::TimePoint current = Timer::Clock::now();
	timer.elapsed += current - timer.start;
	timer.start = current;
}

MFbool Manifest_Core::TimerStopped(const Timer& timer)
{//if limit was never given stop = TimePoint::max()
	return timer.start > timer.stop;
}


