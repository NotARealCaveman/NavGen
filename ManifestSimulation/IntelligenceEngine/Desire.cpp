#include "Desire.h"

using namespace Manifest_Simulation;

Desire Manifest_Simulation::SENTINEL_DESIRE
{
	.comparativeValue{ [](const BlackBoard&, const MFu64, const MFu64)->WorldState {return {0}; } },
	.satisfier{TERMINATE },
	.termiantor{TERMINATE},
	.objectiveID {NO_DESIRE_SET},
	.assignedID{NO_DESIRE_SET},
	.representation{ NO_DESIRE_SET },
	.priority{ 0 }
};