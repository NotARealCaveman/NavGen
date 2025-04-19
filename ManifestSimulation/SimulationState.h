#pragma once
#include <atomic>//contact point render

#include <ManifestMath/Quaternion.h>
#include <ManifestMath/Point3.h>
#include <ManifestPersistence/DatabaseTypes.h>

using namespace Manifest_Math;
using Manifest_Persistence::PrimaryKey;

namespace Manifest_Simulation
{		
	struct SimulationState
	{
		//to be removed, for debug and using statepairs for scope?
		static std::atomic<MFpoint3*> contactPoint;		

		PrimaryKey* nodeID{ nullptr };		
		MFquaternion* orientation{ nullptr };		
		MFpoint3* position{ nullptr };
		MFpoint3* previousPosition{ nullptr };
		MFvec3* scale{ nullptr };
		MFu32 stateCount{ 0 };
		~SimulationState()
		{
			if (nodeID)
				delete[] nodeID;			
			if (orientation)
				delete[] orientation;			
			if (position)
				delete[] position;
			if (previousPosition)
				delete[] previousPosition;
			if (scale)
				delete[] scale;

			nodeID = nullptr;			
			orientation = nullptr;			
			position = nullptr;
			previousPosition = nullptr;
			scale = nullptr;
		};
	};
}