#pragma once
#include <deque>

#include <ManifestMath/Quaternion.h>
#include <ManifestMath/Point3.h>
#include <ManifestCommunication/EventSpace.h>
#include <ManifestUtility/TypeAssist.h>
#include <ManifestSimulation/IntelligenceEngine/HivemindDroneSystem.h>
#include <ManifestPersistence/Runtime_Database.h>

using namespace Manifest_Communication;

using Manifest_Utility::pow2;
using namespace Manifest_Math;

namespace Manifest_Core
{	
	struct ObjectRegistrationMessage
	{			
		Manifest_Simulation::Descriptor objectCapaibilities{ 0 };
		Manifest_Simulation::Descriptor objectAttributes{ 0 };	
		MFu64 objectID;
		MFbool addObject;
	}; 
	 
	struct AgentSatisfactionMessage
	{
		const MFu64 agentID;		
		const Manifest_Simulation::Descriptor system;
		const Manifest_Simulation::Descriptor type;
	};

	struct ObservableHivemindSystem
	{
	private:
		static constexpr MFu64 REGISTERATION{ 0 }; 
		static constexpr MFu64 SYSTEM_DESIRE{ REGISTERATION + 1 };
	public:
		enum class MessageTypes : MFu64
		{
			OBJECT_REGISTRATION = pow2(REGISTERATION),
			DESIRE_ASSIGNMENT = pow2(SYSTEM_DESIRE)
		};
	};

	using HivemindSystem = ObservableHivemindSystem;
	using HivemindSystemMessageType = HivemindSystem::MessageTypes;
	using HivemindSystemObservationToken = ObservationToken;
	using HivemindSystemEvent = Event;
	using HivemindSystemObserver = Observer;
	using HivemindSystemEventSpace = EventSpace;
}