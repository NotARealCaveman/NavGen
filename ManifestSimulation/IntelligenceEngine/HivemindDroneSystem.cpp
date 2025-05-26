#include "HivemindDroneSystem.h"

using namespace Manifest_Simulation;

Descriptor HivemindDroneSystem::RUNTIME_CAPABILITIES{ 0 };
Descriptor HivemindDroneSystem::RUNTIME_ATTRIBUTES{ 0 };

void HivemindDroneSystem::RegisterRuntimeCapabilities(const Descriptor runtimeCapabilities)
{
	assert((runtimeCapabilities & RUNTIME_CAPABILITIES) == 0, "SUPPLIED CAPABILITIY ALREADY REGISTERED");
	RUNTIME_CAPABILITIES |= runtimeCapabilities;
}

void HivemindDroneSystem::RegisterRuntimeAttributes(const Descriptor runtimeAttributes)
{
	assert((runtimeAttributes & RUNTIME_ATTRIBUTES) == 0, "SUPPLIED ATTRIBUTE ALREADY REGISTERED");
	RUNTIME_ATTRIBUTES |= runtimeAttributes;
}