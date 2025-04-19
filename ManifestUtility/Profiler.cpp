#include "Profiler.h"

using namespace Manifest_Utility;

std::chrono::steady_clock* Profiler::mainProfiler(new std::chrono::steady_clock);
std::chrono::duration<double> Profiler::profilier_accumulator(0);

Profiler::Profiler()
{
}

void Profiler::BeginProfiler()
{
	profilier_accumulator = mainProfiler->now().time_since_epoch();
}

double Profiler::EndProfiler()
{
	profilier_accumulator = mainProfiler->now().time_since_epoch() - profilier_accumulator;

	return std::chrono::duration_cast<std::chrono::microseconds>(profilier_accumulator).count();
}

Profiler::~Profiler()
{
}
