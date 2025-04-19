#include "FileSystem.h"

using namespace Manifest_Core;

void Manifest_Core::ObersvableFileSystem::LoadGameDatabaseFile(const std::string& mbd, FileSystemEventSpace& eventSpace)
{
	std::ifstream bImport{ mbd, std::ios::in | std::ios::binary };
	ObersvableFileSystem::MessageTypes eventToken;
	//import database to scratch pad
	auto resourceDatabase = ImportGameResources(bImport);	
	FileSystemEvent event;	
	for(auto materialIndex{0}; materialIndex < resourceDatabase.binaryMaterialTable.header.totalEntries; ++materialIndex)
	{
		eventToken = eventToken | MessageTypes::MBD_MATERIAL;
		event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_MATERIAL), resourceDatabase.binaryMaterialTable[materialIndex]});
	}	
	for (auto texture{ 0 }; texture < resourceDatabase.binaryTextureTable.header.totalEntries; ++texture)
	{
		eventToken = eventToken | MessageTypes::MBD_TEXTURE;
		event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_TEXTURE), resourceDatabase.binaryTextureTable[texture]});
	}	
	for (auto objectIndex{ 0 }; objectIndex < resourceDatabase.binaryGeometryObjectTable.header.totalEntries; ++objectIndex)
	{			
		eventToken = eventToken | MessageTypes::MBD_GEOMETRYOBJECT;
		event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_GEOMETRYOBJECT), resourceDatabase.binaryGeometryObjectTable[objectIndex]});
	}
	for (auto nodeIndex{ 0 }; nodeIndex < resourceDatabase.binaryGeometryNodeTable.header.totalEntries; ++nodeIndex)
	{		
		eventToken = eventToken | MessageTypes::MBD_GEOMETRYNODE;
		event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_GEOMETRYNODE), resourceDatabase.binaryGeometryNodeTable[nodeIndex]});
	}
	for (auto meshIndex{ 0 }; meshIndex < resourceDatabase.binaryMeshTable.header.totalEntries; ++meshIndex)
	{
		eventToken = eventToken | MessageTypes::MBD_MESH;
		event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_MESH), resourceDatabase.binaryMeshTable[meshIndex]});
	}	
	//dynamic->static
	for (auto rigidBodyIndex{ 0 }; rigidBodyIndex < resourceDatabase.binaryRigidBodyTable.header.totalEntries; ++rigidBodyIndex)
	{
		eventToken = eventToken | MessageTypes::MBD_RIGIDBODY;
		event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_RIGIDBODY), resourceDatabase.binaryRigidBodyTable[rigidBodyIndex]});
	} 
	//dynamic->static
	for (auto colliderIndex{ 0 }; colliderIndex < resourceDatabase.binaryColliderTable.header.totalEntries; ++colliderIndex)
	{
		eventToken = eventToken | MessageTypes::MBD_COLLIDER;
		event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_COLLIDER), resourceDatabase.binaryColliderTable[colliderIndex]});
	} 
	event.eventToken = UnderlyingType(eventToken);
	eventSpace.NotifyRegisteredObservers(std::move(event));
}

void Manifest_Core::ObersvableFileSystem::LoadWorldFile(const std::string& mbd, FileSystemEventSpace& eventSpace)
{
	std::ifstream bImport{ mbd, std::ios::in | std::ios::binary };
	//import database to scratch pad
	auto terrainDatabase = ImportGameTerrain(bImport);
	FileSystemEvent event;
	//voxel map will be sent to the world system
	event.eventToken |= UnderlyingType(MessageTypes::MBD_VOXELMAP);
	event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_VOXELMAP), terrainDatabase.binaryVoxelMapTable[0]});	
	//terrain chunks will be sent to terrain system
	for (auto terrainIndex{ 0 }; terrainIndex < terrainDatabase.binaryTerrainTable.header.totalEntries; ++terrainIndex)
	{		
		event.eventToken |= UnderlyingType(MessageTypes::MBD_TERRAIN);
		event.messages.emplace_back(Message {UnderlyingType(MessageTypes::MBD_TERRAIN), terrainDatabase.binaryTerrainTable[terrainIndex]});
	}

	eventSpace.NotifyRegisteredObservers(std::move(event));
}