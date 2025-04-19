#include "SimulationEngine.h"

using namespace Manifest_Simulation;


SimulationEngine::SimulationEngine()
	: fileSystemObserver{ fileSystemObservationToken }, frameworkSystemObserver{ frameworkSystemObservationToken}
{
};

MFbool SimulationEngine::Init(HivemindSystemEventSpace& hivemindSystemEventSpace)
{
	if (!physicsEngine.Init(&idGenerator))
		return false;
	if (!entityEngine.Init(hivemindSystemEventSpace,&idGenerator))
		return false;
	if(!intelligenceEngine.Init(runtimeDatabase->terrainExtractionTable, runtimeDatabase->actionPlanTable))
		return false;  
	//map path finder words to dictionary

	return true;
} 

MFbool SimulationEngine::RegisterSimulationObservers(FileSystemEventSpace& fileSystemEventSpace, FrameworkSystemEventSpace& frameworkSystemEventSpace, HivemindSystemEventSpace& hivemindSystemEventSpace)
{
	this->frameworkSystemEventSpace = &frameworkSystemEventSpace;
	intelligenceEngine.RegisterHivemindToEventSpace(hivemindSystemEventSpace);
	if(!RegisterObserverToEventSpace(fileSystemObserver, fileSystemEventSpace))
		return false;
	if (!RegisterObserverToEventSpace(frameworkSystemObserver, frameworkSystemEventSpace))
		return false;
	return true;
}

void SimulationEngine::Shutdown()
{
	physicsEngine.Terminate();	
	entityEngine.Terminate();	
	intelligenceEngine.Terminate();
}

void SimulationEngine::ObserveSimulationMessages()
{	
	if (fileSystemObserver.HasPendingMessages())
		fileSystemObserver.ProcessEvents(PhysicsFileSystemProcessingFunction, *graphicsSystemEventSpace, physicsEngine, collisionEngine);
	if (frameworkSystemObserver.HasPendingMessages())
		frameworkSystemObserver.ProcessEvents(PhysicsFrameworkSystemProcessingFunction,*this);

}

void SimulationEngine::Pull()
{
	//Pulls placed at begin - see sub stepping comment at end of function
	///check for terrain updates, will probably just move this to a terrain update message for the framework but time shall tell
	runtimeDatabase->terrainExtractionTable->Pull(terrainExtractionReadIndex, ConfigureTerrainForCollision, collisionEngine, physicsEngine, terrainGeneration);
	//get and execute agent actions - SIMULATION CAN ONLY EXECUTE PATHFINDING ACTIONS. GAME SPECIFIC SYSTEMS MUST BE PROCESSED IN THE GAME LOOP
	runtimeDatabase->actionPlanTable->Pull(actionPlanReadIndex, [this](const typename Manifest_Memory::RCU<ActionPlan, std::default_delete<ActionPlan>>::Handle& handle)->void
	{
		const ActionPlan& actionPlan{ *handle.handle };
		EntityManager& entityManager{ entityEngine.entityManager };
		 std::ranges::for_each(actionPlan, [&actionPlan, &entityManager](const auto& plan) mutable -> void
		{
			const auto& [agentID, actions] {plan};			
			if (entityManager.GetEntityIndex(agentID) == DataSystem::ID_NOT_MAPPED)
				return;			
			std::ranges::for_each(actions, [&, agentID, requisiteSatisfied = true](const Action& action) mutable->void
			{			
				//this could be better but fine for now
				if (!requisiteSatisfied) 
					return;  

				ForwardFunction(action, agentID);
			});
		});
	});
}

void SimulationEngine::StepSimulation(const MFu32 currentPollingGeneration)
{			
	const MFfloat dt{ static_cast<MFfloat>(GetTimeStep().count()) };
	//clear last frame collisions - populate buffers for new step
	collisionEngine.ResetCollisions();
	//step simulation	
	UpdateEffects (dt,simulationEffectApplicators.physicsEffectApplicators);
	UpdateEffects(dt, simulationEffectApplicators.entityEffectApplicators);
	physicsEngine.IntegratePhysicsForces(dt);	
	entityEngine.UpdateEntities(dt, currentPollingGeneration);	
	collisionEngine.UpdateColliders(physicsEngine,entityEngine);
	collisionEngine.DetectCollisions();
	collisionEngine.PreparePhysicsCollisions(physicsEngine);	
	collisionEngine.ResolvePhysicsCollisions(dt);
	collisionEngine.ProcessEntityCollisions(entityEngine);
	collisionEngine.ResolveEntityCollisions(dt, entityEngine);	
	physicsEngine.IntegratePhysicsVelocities(dt);	
	entityEngine.IntegrateEntities(dt);	
	
}

void SimulationEngine::Push()
{	
	runtimeDatabase->simulationStateTable->Push(CopySimulationStates, physicsEngine, entityEngine);	 
}

const MFbool SimulationEngine::IsEntityObject(const MFu64 simulationID)
{
	return entityEngine.entityManager.GetEntityIndex(simulationID) != DataSystem::ID_NOT_MAPPED;
}

const MFbool SimulationEngine::IsPhysicsObject(const MFu64 simulationID)
{
	return physicsEngine.IsBodyMapped(simulationID);
}

SimulationState* Manifest_Simulation::CopySimulationStates(const PhysicsEngine& physicsEngine, const EntityEngine& entityEngine)
{
	const EntityManager& entityManager{ entityEngine.entityManager };
	const EntityPhysicsData& entityData{ entityManager.entityData };
	const RigidBodyData& rigidBodies{ physicsEngine.rigidBodies};
	const MFsize totalBodies{ rigidBodies.size() };
	const MFsize totalEntities{ entityManager.entityData.size() };
	const MFsize totalStates{ totalBodies + totalEntities };
	SimulationState* result{ new SimulationState };
	result->nodeID = new Manifest_Persistence::PrimaryKey[totalStates];
	result->orientation = new MFquaternion[totalStates];
	result->position = new MFpoint3[totalStates];	
	result->previousPosition = new MFpoint3[totalStates];
	result->scale = new MFvec3[totalStates];
	result->stateCount = totalStates;

	//cache simulated states from physics objects and then entities	
	rigidBodies.CopySimulationDataForRendering(result->nodeID, result->orientation, result->position,result->previousPosition, result->scale, totalBodies,0);
	entityData.CopySimulationDataForRendering(result->nodeID, result->orientation, result->position, result->previousPosition, result->scale, totalEntities, totalBodies);

	return result;
}

void Manifest_Simulation::PhysicsFileSystemProcessingFunction(std::vector<Message>& messages, GraphicsSystemEventSpace& graphicsSystemEventSpace, PhysicsEngine& physicsEngine, CollisionEngine& collisionEngine)
{
	RigidBodyData& rigidBodies{ physicsEngine.rigidBodies}; 
	DLOG({ CONSOLE_BG_GREEN }, "total messages:", messages.size());
	for (Message& message : messages)
	{
		DLOG({ CONSOLE_BG_MAGENTA }, "processing message:", UnderlyingType(FileSystemMessageType::MBD_RIGIDBODY));
		switch (message.messageToken)
		{
		case UnderlyingType(FileSystemMessageType::MBD_RIGIDBODY):
		{
			auto rigidBodyParams = message.GetMessageContent<Binary_RigidBody>();			
			const Binary_RigidBody::Entry_Header& header{ rigidBodyParams.header };

			if (!header.bodyCount)
				break;
									
			Byte* const& payload{ rigidBodyParams.payload };
			const MFu32 offset{ rigidBodies.ReserveSpaceForBodies(header.bodyCount) };			
			MFquaternion* orientation{ const_cast<MFquaternion*>(&rigidBodies.GetOrientation(offset)) };
			memcpy(orientation, payload, header.positionOffset);
			//normalize orientations
			std::transform(&orientation[offset], &orientation[offset + header.bodyCount], &orientation[offset], [&](MFquaternion& orientation) {return Normalize(orientation); });
			MFpoint3* position{ const_cast<MFpoint3*>(&rigidBodies.GetPosition(offset)) };
			memcpy(&position[offset], &payload[header.positionOffset], header.linearAccelarationOffset - header.positionOffset);
			MFvec3* linearAcceleration{ const_cast<MFvec3*>(&rigidBodies.GetLinearAccelaration(offset)) };
			memcpy(&linearAcceleration[offset], &payload[header.linearAccelarationOffset], header.linearVelocityOffset - header.linearAccelarationOffset);
			MFvec3* linearVelocity{ const_cast<MFvec3*>(&rigidBodies.GetLinearVelocity(offset)) };
			memcpy(&linearVelocity[offset], &payload[header.linearVelocityOffset], header.angularVelocityOffset - header.linearVelocityOffset);
			MFvec3* angularVelocity{ const_cast<MFvec3*>(&rigidBodies.GetAngularVelocity(offset)) };
			memcpy(&angularVelocity[offset], &payload[header.angularVelocityOffset], header.scaleOffset- header.angularVelocityOffset);
			MFvec3* scale{ const_cast<MFvec3*>(&rigidBodies.GetScale(offset)) };
			memcpy(&scale[offset], &payload[header.scaleOffset], header.appliedForceOffset - header.scaleOffset);
			MFvec3* appliedForce{ const_cast<MFvec3*>(&rigidBodies.GetForce(offset)) };
			memcpy(&appliedForce[offset], &payload[header.appliedForceOffset], header.appliedTorqueOffset - header.appliedForceOffset);
			MFvec3* appliedTorque{ const_cast<MFvec3*>(&rigidBodies.GetTorque(offset)) };
			memcpy(&appliedTorque[offset], &payload[header.appliedTorqueOffset], header.iMassOffset - header.appliedTorqueOffset);
			MFfloat* iMass{ const_cast<MFfloat*>(&rigidBodies.GetIMass(offset)) };
			memcpy(&iMass[offset], &payload[header.iMassOffset], header.linearDampingOffset - header.iMassOffset);
			MFfloat* linearDamping{ const_cast<MFfloat*>(&rigidBodies.GetLinearDamping(offset)) };
			memcpy(&linearDamping[offset], &payload[header.linearDampingOffset], header.angularDampingOffset - header.linearDampingOffset);
			MFfloat* angularDamping{ const_cast<MFfloat*>(&rigidBodies.GetAngularDamping(offset)) };
			memcpy(&angularDamping[offset], &payload[header.angularDampingOffset], header.objectIDOffset - header.angularDampingOffset);
			MFu64* ID{ const_cast<MFu64*>(&rigidBodies.GetID(offset)) };
			memcpy(&ID[offset], &payload[header.objectIDOffset], header.dynamicOffset - header.objectIDOffset);
			//set dynamic status dummy data for static
			MFbool* isDynamic{ const_cast<MFbool*>(&rigidBodies.GetDynamic(offset)) };
			memcpy(&isDynamic[offset], &payload[header.dynamicOffset], header.payloadSize - header.dynamicOffset);
			MFpoint3* prevPosition{ const_cast<MFpoint3*>(&rigidBodies.GetPreviousPosition(offset)) };
			memset(&prevPosition[offset], 0, sizeof(MFpoint3) * header.bodyCount);
			MFvec3* prevAcceleration{ const_cast<MFvec3*>(&rigidBodies.GetPreviousAccelaration(offset)) };
			memset(&prevAcceleration[offset], 0, sizeof(MFvec3) * header.bodyCount);
			MFquaternion* prevOrientation{ const_cast<MFquaternion*>(&rigidBodies.GetPreviousOrientation(offset)) };
			memset(&prevOrientation[offset], 0, sizeof(MFquaternion) * header.bodyCount);			;
			for (auto body{ 0 }; body < header.bodyCount; ++body)
			{		
				//this is an issue - cannot get body id because buffer doesn't know about data existing
				const MFu32 bodyIndex{ body + offset };
				rigidBodies.AddBody(rigidBodies.GetID(bodyIndex), bodyIndex, rigidBodies.GetDynamic(bodyIndex));
				const MFmat3 rotation{ orientation[bodyIndex].GetRotation() };
				rigidBodies.SetWorldSpace(BodyWorldSpace(rotation, rigidBodies.GetPosition(bodyIndex)), bodyIndex);
				rigidBodies.SetWorldITensor(InteriaWorldSpace(rotation, rigidBodies.GetLocalITensor(bodyIndex)), bodyIndex);
			}
			break;
		}
		//NOT THE BIGGEST FAN OF THIS BUT RIGID BODY MESSAGES MUST COME BEFORE COLLIDER MESSAGES. COLLIDERS ASSUME A RIGID BODY EXISTS FOR INDEXING
		case UnderlyingType(FileSystemMessageType::MBD_COLLIDER):
		{
			Binary_Collider bcollider = message.GetMessageContent<Binary_Collider>();
			Collider* collider{ new Collider };			
			switch (bcollider.header.colliderType)
			{
			case COLLIDER_GEOMETRY::SPHERE:
			{
				BoundingSphere sphere;
				//need positions for center to  handle static colliders
				sphere.radius = *reinterpret_cast<const MFfloat*>(bcollider.payload);
				sphere.radius= *reinterpret_cast<const MFfloat*>(bcollider.payload);
				const MFu32 sphereBodyIndex{ physicsEngine.GetBodyIndex(bcollider.header.objectID)};
				sphere.center = rigidBodies.GetPosition(sphereBodyIndex);
				collider->SetCollider(sphere);
				rigidBodies.SetLocalITensor(SphereITensor(sphere.radius, rigidBodies.GetIMass(sphereBodyIndex)), sphereBodyIndex);
				break;
			}
			case COLLIDER_GEOMETRY::CAPSULE:
			{
				Capsule capsule;
				const MFu32 capsuleBodyIndex{ physicsEngine.GetBodyIndex(bcollider.header.objectID)};
				const MFmat3 bodyRotation{ rigidBodies.GetOrientation(capsuleBodyIndex).GetRotation() };
				capsule.worldSpace = BodyWorldSpace(bodyRotation, rigidBodies.GetPosition(capsuleBodyIndex));
				ptrdiff_t payloadOffset{ 0 };
				capsule.radius = *reinterpret_cast<const MFfloat*>(bcollider.payload);				
				payloadOffset += 1;
				capsule.pointsLocal[0] = *reinterpret_cast<const MFpoint3*>(&bcollider.payload[payloadOffset]);
				payloadOffset += 3;
				capsule.pointsLocal[1] = *reinterpret_cast<const MFpoint3*>(&bcollider.payload[payloadOffset]);
				capsule.pointsWorld[0] = capsule.worldSpace * capsule.pointsLocal[0];
				capsule.pointsWorld[1] = capsule.worldSpace * capsule.pointsLocal[1];
				capsule.halfLength = Magnitude(capsule.pointsLocal[1] - capsule.pointsLocal[0]) * 0.5f;
				collider->SetCollider(capsule);
				rigidBodies.SetLocalITensor(CapsuleITensor(capsule.halfLength * 2, capsule.radius, rigidBodies.GetIMass(capsuleBodyIndex), rigidBodies.GetIMass(capsuleBodyIndex)), capsuleBodyIndex);
				rigidBodies.SetWorldITensor(InteriaWorldSpace(bodyRotation, rigidBodies.GetLocalITensor(capsuleBodyIndex)), capsuleBodyIndex);
				break;
			}
			case COLLIDER_GEOMETRY::CONVEXHULL:
			{
				constexpr auto iVec3{ 1.0 / sizeof(MFvec3) };
				std::vector<MFpoint3> pointCloud(bcollider.header.payloadSize * iVec3);
				memcpy(pointCloud.data(), bcollider.payload, bcollider.header.payloadSize);
				ConvexHull convexHull;
				convexHull.mesh = QuickHull(pointCloud);
				const MFu32 convexBodyIndex
				{ physicsEngine.GetBodyIndex(bcollider.header.objectID) };
				const MFmat3 bodyRotation{ rigidBodies.GetOrientation(convexBodyIndex).GetRotation() };
				convexHull.worldSpace = BodyWorldSpace(bodyRotation, rigidBodies.GetPosition(convexBodyIndex));
				convexHull.scale = rigidBodies.GetScale(convexBodyIndex);
				collider->SetCollider(convexHull);
				convexHull.BuildFinalHullFaces();
				//todo provide generic tensor calcualtion for hull			
				rigidBodies.SetLocalITensor(CuboidITensor({ 1 }, rigidBodies.GetIMass(convexBodyIndex)), convexBodyIndex);
				rigidBodies.SetWorldITensor(InteriaWorldSpace(bodyRotation, rigidBodies.GetLocalITensor(convexBodyIndex)), convexBodyIndex);
				break;
			}
			}
			collider->physicsID = bcollider.header.objectID;
			if(bcollider.header.dynamic)
				collisionEngine.InsertCollider(COLLIDER_CLASSIFICATION::DYNAMIC, collider);
			else
				collisionEngine.InsertCollider(COLLIDER_CLASSIFICATION::STATIC, collider);
			break;
		}
		default:
			break;
		}
	}
}

void Manifest_Simulation::PhysicsFrameworkSystemProcessingFunction(std::vector<Message>& messages, SimulationEngine& simulationEngine)
{
	for (Message& message : messages)
	{
		switch (message.messageToken)
		{
			case UnderlyingType(FrameworkSystemMessageType::ADD_ENTITY):
			{ 
				//todo: rewrite entity addition with memory/spawner overhaul
				break;
			}			
			case UnderlyingType(FrameworkSystemMessageType::REMOVE_ENTITY):
			{
				RemovedIDs removedEntityIDs{ message.GetMessageContent<RemovedIDs>() };
				 simulationEngine.entityEngine.entityManager.RemoveEntities(removedEntityIDs);

				break;
			}
			case UnderlyingType(FrameworkSystemMessageType::ADD_COLLIDER):
			{
				Collider* collider{ message.GetMessageContent<Collider*>() };
				if(simulationEngine.physicsEngine.IsBodyMapped(collider->physicsID))
					simulationEngine.collisionEngine.InsertCollider(COLLIDER_CLASSIFICATION::DYNAMIC, collider);
				else
					simulationEngine.collisionEngine.InsertCollider(COLLIDER_CLASSIFICATION::ENTITY,collider);
				break;
			}
			case UnderlyingType(FrameworkSystemMessageType::ATTACH_SIMULATION_EFFECT):
			{
				EffectsAttachment effectsAttachment{ message.GetMessageContent<EffectsAttachment>() };
				if (effectsAttachment.effectsEntity)
					AttachEffect(simulationEngine.simulationEffectApplicators.entityEffectApplicators, std::move(effectsAttachment.effectApplicator));
				else
					AttachEffect(simulationEngine.simulationEffectApplicators.physicsEffectApplicators, std::move(effectsAttachment.effectApplicator));
				break;
			}
		}
	}
}


void Manifest_Simulation::ConfigureTerrainForCollision(const typename Manifest_Memory::RCU<TerrainExtraction, TerrainExtractionDeleter>::Handle& handle, CollisionEngine& collisionEngine, PhysicsEngine& physicsEngine, MFu32& previousGeneration)
{
	const TerrainExtraction& terrainExtraction{ *handle.handle };
	if (previousGeneration == handle.generation)
		return;

	previousGeneration = handle.generation;

	if (!terrainExtraction.totalExtractedChunks)
		return;

	MFsize chunksPerLOD{ terrainExtraction.totalExtractedChunks / (terrainExtraction.maxExtractionLOD + 1) };

	DLOG({ CONSOLE_MAGENTA }, "It happened boys");
	const MFu8 TARGET_LOD{ 0 };
	const TerrainExtractionData& data{ terrainExtraction.data[chunksPerLOD * TARGET_LOD] };

	const PrimaryTerrain& primaryTerrain{ data.primaryTerrain };
	const ExtractionData& extractionData{ primaryTerrain.extractionData };

	MFtransform voxelToWorld
	{
		WORLD_SCALE,0.0f,0.0f,
		0.0f,0.0f,WORLD_SCALE,
		0.0f,WORLD_SCALE,0.0f,
		0.0f,0.0f,(MFfloat)(terrainExtraction.voxelSpaceZOffset * WORLD_SCALE)
	};
	MFpoint3 minVertex
	{
		std::numeric_limits<MFfloat>::max()
	},
		maxVertex
	{
		std::numeric_limits<MFfloat>::min()
	};
	Collider* collider{ new Collider };
	collider->SetCollider<MeshCollider>({});
	MeshCollider& meshCollider{ collider->GetColliderGeometry<MeshCollider>() };
	meshCollider.faces.reserve(extractionData.nTriangles);
	MFu32 largestTriangleIndex{ NULL };
	MFfloat largestArea{ -std::numeric_limits<MFfloat>::infinity() };
	for (auto i{ 0 }; i < chunksPerLOD; ++i)
	{
		const TerrainExtractionData& data{ terrainExtraction.data[(chunksPerLOD * TARGET_LOD)+i] };

		const PrimaryTerrain& primaryTerrain{ data.primaryTerrain };
		const ExtractionData& extractionData{ primaryTerrain.extractionData };
		for (const Triangle& triangle : extractionData.triangles)
		{
			auto a = triangle.vertexIndex[0];
			auto b = triangle.vertexIndex[1];
			auto c = triangle.vertexIndex[2];
			MFvec3 surfaceNormal{ static_cast<MFvec3>(ValidationNormal(extractionData.vertices[a], extractionData.vertices[b], extractionData.vertices[c], data.extractionCorner)) };
			//calulate 4*Area of triangle
			const MFfloat quadrupleTriangleArea{ MagnitudeSquared(surfaceNormal) };
			if (!quadrupleTriangleArea)
				continue;

			MFtriangle conversion
			{
				voxelToWorld * static_cast<MFpoint3>(static_cast<const MFvec3>(extractionData.vertices[triangle.vertexIndex[2]])),
				voxelToWorld * static_cast<MFpoint3>(static_cast<const MFvec3>(extractionData.vertices[triangle.vertexIndex[1]])),
				voxelToWorld * static_cast<MFpoint3>(static_cast<const MFvec3>(extractionData.vertices[triangle.vertexIndex[0]]))
			};
			//z is reversed
			for (const MFpoint3& vertex : conversion.vertices)
			{
				minVertex = Min(minVertex, vertex);
				maxVertex = Max(maxVertex, vertex);
			}
			//cache largest triangle to create min subdivision length in octree
			if (largestArea < quadrupleTriangleArea)
			{
				largestArea = quadrupleTriangleArea;
				largestTriangleIndex = meshCollider.faces.size();
			}
			meshCollider.faces.emplace_back(conversion);
		}
	}

	BuildOctreeFromTriangles(meshCollider.faces, minVertex, maxVertex, largestTriangleIndex, meshCollider.octree);
	int total{ 0 };
	int nodeIndex{ 0 };
	std::map<int, int> sizeMap;
	for (const auto& node : meshCollider.octree.nodes)
	{
		DLOG({ CONSOLE_CYAN }, "node:", nodeIndex, "has", node.triangles.size(), "triangles");
		++nodeIndex;
		total += node.triangles.size();
		if (sizeMap.find(node.triangles.size()) == sizeMap.end())
			sizeMap[node.triangles.size()] = 1;
		else
			++sizeMap[node.triangles.size()];
	}
	for (const auto& [key, value] : sizeMap)
		DLOG({ CONSOLE_CYAN }, "triangle cont:", key, "has", value, "nodes");
	collisionEngine.InsertCollider(COLLIDER_CLASSIFICATION::TERRAIN, std::move(collider));
	const PrimaryKey terrainID{ collider->physicsID };
	RigidBodyParameters terrainParams;
	ZeroBody(terrainParams);
	physicsEngine.rigidBodies.AddBody(terrainParams, terrainID, false);

	previousGeneration = handle.generation;
}

SimulationEngine::~SimulationEngine()
{
}