#include "PhysicsTimer.h"

using namespace Manifest_Core;

PhysicsTimer::PhysicsTimer()
	:timer{}, timeStep{ 20ms }//~50fps
{
};
PhysicsTimer::PhysicsTimer(const Timer::Duration& _timeStep)

	:timer{}, timeStep{ _timeStep }
{
};

void PhysicsTimer::Update()
{
	StepTimer(timer);
}