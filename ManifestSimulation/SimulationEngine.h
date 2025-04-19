#pragma once
#include <vector>

#include <ManifestCore/Systems/FileSystem.h>
#include <ManifestCore/Systems/GraphicsSystem.h>
#include <ManifestCore/Systems/FrameworkSystem.h>
#include <ManifestCore/PhysicsTimer.h>
#include <ManifestCore/EngineModule.h>
#include <ManifestMath/Quickhull.h>
#include <ManifestMemory/Utility/ThreadPool.h>


#include "EntityEngine/EntityEngine.h"
#include "PhysicsEngine/PhysicsEngine.h"
#include "CollisionEngine/CollisionEngine.h"
#include "IntelligenceEngine/IntelligenceEngine.h"
#include "SimulationState.h"
#include "SimulationEffects.h"

using namespace Manifest_Communication;
using namespace Manifest_Math;
using namespace Manifest_Memory;
using Manifest_Core::EngineModule;

namespace Manifest_Simulation
{		
	class SimulationEngine : public EngineModule
	{
		public:
			Timer performanceTimer;
		public:
			SimulationEngine();
			~SimulationEngine();
			//framework integration
			MFbool Init(HivemindSystemEventSpace& hivemindSystemEventSpace);			
			MFbool RegisterSimulationObservers(FileSystemEventSpace& fileSystemEventSpace, FrameworkSystemEventSpace& frameworkSystemEventSpace, HivemindSystemEventSpace& hivemindSystemEventSpace);
			void Shutdown();				
			void ObserveSimulationMessages();
			//update simulation
			void StepSimulation(const MFu32 currentPollingGeneration);		
			void Pull() override;
			void Push() override;
			const MFbool IsEntityObject(const MFu64 simulationID);
			const MFbool IsPhysicsObject(const MFu64 simulationID);
			inline void SetTimeStep(const Timer::Duration& _timeStep) { timeStep = _timeStep; };
			inline const Timer::Duration& GetTimeStep() const { return timeStep; };

			Timer::Duration accumlatedTime { 0ms };
			Timer::Duration timeStep{ 13ms };//~75hz
			//set to game data in initializer
			ThreadPool* pool;
			GraphicsSystemEventSpace* graphicsSystemEventSpace;
			FrameworkSystemEventSpace* frameworkSystemEventSpace;
			HivemindSystemEventSpace* hivemindSystemEventSpace;
			ManifestRuntimeDatabase* runtimeDatabase;
			//event observer
			FileSystemObserver fileSystemObserver;
			FrameworkSystemObserver frameworkSystemObserver;
			//subengines
			EntityEngine entityEngine;
			PhysicsEngine physicsEngine;
			CollisionEngine collisionEngine;			
			IntelligenceEngine intelligenceEngine;
			//used to map data systems to goal indices
			FlatMap<DataSystem*> dataSystems;

			SimulationEffectApplicators simulationEffectApplicators;			

			MFu32 terrainExtractionReadIndex;
			MFu32 actionPlanReadIndex;						
			private:
				xoshiro256ss_state idGenerator;
				MFu32 terrainGeneration{ DEFAULT_GENERATION };
			//file system message handling
			static constexpr FileSystemObservationToken fileSystemObservationToken  { UnderlyingType(FileSystemMessageType::MBD_RIGIDBODY |	  FileSystemMessageType::MBD_COLLIDER) };
			//framework system message handling
			static constexpr FrameworkSystemObservationToken frameworkSystemObservationToken
			{
				UnderlyingType(FrameworkSystemMessageType::ADD_COLLIDER | FrameworkSystemMessageType::ADD_ENTITY | FrameworkSystemMessageType::REMOVE_ENTITY | FrameworkSystemMessageType::ADD_RIGIDBODY | FrameworkSystemMessageType::REMOVE_RIGIDBODY | FrameworkSystemMessageType::ATTACH_SIMULATION_EFFECT)
			};
	};

	SimulationState* CopySimulationStates(const PhysicsEngine& physicsEngine, const EntityEngine& entityEngine);

	void ConfigureTerrainForCollision(const typename Manifest_Memory::RCU<TerrainExtraction, TerrainExtractionDeleter>::Handle& handle, CollisionEngine& collisionEngine, PhysicsEngine& physicsEngine, MFu32& previousGeneration);

	void PhysicsFileSystemProcessingFunction(std::vector<Message>& messages, GraphicsSystemEventSpace& graphicsSystemEventSpace, PhysicsEngine& physicsEngine, CollisionEngine& collisionEngine);
	void PhysicsFrameworkSystemProcessingFunction(std::vector<Message>& messages, SimulationEngine& simulationEngine);
}