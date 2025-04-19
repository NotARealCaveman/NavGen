#pragma once
#include <ManifestCore/InputSupport.h>
#include <ManifestCommunication/EventSpace.h>
#include <ManifestGraphics/GraphicsResource.h>
#include <ManifestPersistence/DatabaseTypes.h>
#include <ManifestSimulation/SimulationEffects.h>
#include <ManifestSimulation/EntityEngine/EntitySpawner.h>
#include <ManifestSimulation/CollisionEngine/Collider.h>
#include <ManifestMemory/Utility/FunctionBinder.h>
#include <ManifestUtility/TypeAssist.h>
#include <ManifestUtility/DebugLogger.h>

using namespace Manifest_Communication;
using Manifest_Experimental::GraphicsNode;
using Manifest_Core::InputCode;
using Manifest_Memory::FunctionBinder;
using Manifest_Persistence::PrimaryKey;
using Manifest_Simulation::EffectApplicator, Manifest_Simulation::EntitySpawnParams, Manifest_Simulation::Collider;
using Manifest_Utility::UnderlyingType;
	
namespace Manifest_Core
{  
	struct GraphicsAttachment
	{
		GraphicsNode graphicsNode;
		PrimaryKey simulationID;
	};

	struct EffectsAttachment
	{
		EffectApplicator effectApplicator;
		MFbool effectsEntity;
	};
	
	using RemovedIDs = std::vector<PrimaryKey>;
	 
	struct ObservableFrameworkSystem
	{	
	private:		
		static constexpr MFu16 ADD_ENTITY{ 0 };
		static constexpr MFu16 REMOVE_ENTITY{ ADD_ENTITY + 1 };
		static constexpr MFu16 ADD_RIGIDBODY{ REMOVE_ENTITY + 1 };
		static constexpr MFu16 REMOVE_RIGIDBODY{ ADD_RIGIDBODY + 1 };
		static constexpr MFu16 ADD_COLLIDER{ REMOVE_RIGIDBODY + 1 };
		static constexpr MFu16 ADD_GRAHPICS{ ADD_COLLIDER + 1 };
		static constexpr MFu16 ATTACH_SIMULATION_EFFECT{ ADD_GRAHPICS + 1 };
		static constexpr MFu16 ANGENT_ACTION{ ATTACH_SIMULATION_EFFECT + 1 };

	public:	
		enum class MessageTypes : MFu16
		{			
			ADD_ENTITY = pow2(ADD_ENTITY),
			REMOVE_ENTITY = pow2(REMOVE_ENTITY),
			ADD_RIGIDBODY = pow2(ADD_RIGIDBODY),
			REMOVE_RIGIDBODY = pow2(REMOVE_RIGIDBODY),
			ADD_COLLIDER = pow2(ADD_COLLIDER),
			ATTACH_GRAPHICS = pow2(ADD_GRAHPICS),
			ATTACH_SIMULATION_EFFECT = pow2(ATTACH_SIMULATION_EFFECT),
			SEND_AGENT_ACTION = pow2(ANGENT_ACTION)
		};				
		static void AddCollisionVolume(Event& event,Collider* collider);
		static void AttachGraphicsResource(Event& event, GraphicsAttachment graphicsAttachment);			
	};

	using FrameworkSystem = ObservableFrameworkSystem;
	using FrameworkSystemMessageType = FrameworkSystem::MessageTypes;
	using FrameworkSystemObservationToken = ObservationToken;
	using FrameworkSystemEvent = Event;
	using FrameworkSystemObserver = Observer;
	using FrameworkSystemEventSpace = EventSpace;	
}