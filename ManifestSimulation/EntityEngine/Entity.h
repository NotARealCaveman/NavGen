#pragma once 

#include <ManifestSimulation/DataSystem.h>

using namespace Manifest_Math;	 
namespace Manifest_Simulation
{
	static constexpr MFsize MAX_ENTITIES{ 1000 };
	static constexpr MFfloat WALKABLE_TOLERANCE{ -0.775 };//45 degree slope

	struct EntitySpawnParams
	{
		/*unrelated*/
	}; 

	struct EntityPhysicsData : public DataSystem
	{					
		/*unrelated*/
	};	
}