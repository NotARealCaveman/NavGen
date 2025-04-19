#include "SimulationEffects.h"

using namespace Manifest_Simulation;

extern void Manifest_Simulation::AttachEffect(std::vector<EffectApplicator>& effectApplicators, EffectApplicator&& effectApplicator)
{
	effectApplicators.emplace_back(std::move(effectApplicator));
}

void Manifest_Simulation::UpdateEffects(const MFfloat dt, std::vector<EffectApplicator>& effectApplciators)
{
	//remove old effect applicators
	std::_Erase_remove_if(effectApplciators, [](const EffectApplicator& effectApplicator)
	{ return effectApplicator.elapsedTime >= effectApplicator.lifeTime; });	 	
	//apply current effect applicators
	std::ranges::for_each(effectApplciators, [&](EffectApplicator& effectApplicator) ->void
		{
			effectApplicator.simulationEffect();
			effectApplicator.elapsedTime += dt;
		});	
}