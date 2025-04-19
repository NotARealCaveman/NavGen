#include "World.h" 

using namespace Manifest_Core;

World::World()
	:fileSystemObserver{ fileSystemObservationToken } {};

void World::ObserveWorldMessages()
{
	if (fileSystemObserver.HasPendingMessages())
	{
		//push a new world config and record number of chunks in the map file
		fileSystemObserver.ProcessEvents(WorldConfigurationFileSystemMessage, *runtimeDatabase, worldConfigration,worldChunks);
		//notify terrain system worldChunks number of chunks loaded from the map file
		ObersvableWorldSystem::NotifyNewWorldMap
		(worldChunks, *worldSystemEventSpace);
	}
};

MFbool World::Init(ManifestRuntimeDatabase* _runtimeDatabase, WorldSystemEventSpace* _worldSystemEventSpace)
{
	runtimeDatabase = _runtimeDatabase;
	worldSystemEventSpace = _worldSystemEventSpace;
	return true;
}
MFbool World::RegisterWorldObservers(FileSystemEventSpace& fileSystemEventSpace)
{
	if (!RegisterObserverToEventSpace(fileSystemObserver, fileSystemEventSpace))
		return false;
	return true;
};

void  Manifest_Core::WorldConfigurationFileSystemMessage(std::vector<Message>& messages, ManifestRuntimeDatabase& runtimeDatabase, WorldConfiguration& worldConfiguration, MFu32& worldChunks)
{	
	//message is only a binary voxel map
	auto binaryVoxelMap{ messages[0].GetMessageContent<Binary_VoxelMap>() };		
	runtimeDatabase.worldConfigurationTable->Push(CreateNewWorldConfiguration, binaryVoxelMap,worldConfiguration);
	//notify terrain of new sdf for extraction
	worldChunks=  binaryVoxelMap.header.xChunks * binaryVoxelMap.header.zChunks;		
}

WorldConfiguration* Manifest_Core::CreateNewWorldConfiguration(Binary_VoxelMap& binaryVoxelMap, WorldConfiguration& worldConfiguration)
{
	//calculate terrain metrics
	const auto base = BASE_CELLS_PER_BLOCK;
	const auto numBlocks = base << binaryVoxelMap.header.worldLOD;
	const auto blockSizeVoxelSpace = numBlocks << WORLD_INTERPOLATION;
	const auto blockSizeWorldSpace = blockSizeVoxelSpace * WORLD_SCALE;

	//store world configuration - this will be uploaded to the database	
	worldConfiguration.xChunks = binaryVoxelMap.header.xChunks;
	worldConfiguration.zChunks = binaryVoxelMap.header.zChunks;
	worldConfiguration.worldGrid = Grid2D{ (float)blockSizeWorldSpace, 0, (float)blockSizeWorldSpace * worldConfiguration.xChunks, 0, -(float)blockSizeWorldSpace * worldConfiguration.zChunks };
	static_cast<MFint32>(blockSizeVoxelSpace * worldConfiguration.zChunks);
	auto& voxelMap = worldConfiguration.voxelMap;
	voxelMap.lod = worldConfiguration.worldLOD = binaryVoxelMap.header.worldLOD;
	voxelMap.mVoxels = binaryVoxelMap.header.mVoxels;
	voxelMap.nVoxels = binaryVoxelMap.header.nVoxels;
	voxelMap.hVoxels = binaryVoxelMap.header.hVoxels;
	voxelMap.field = new Voxel[binaryVoxelMap.header.payloadSize];
	memcpy(voxelMap.field, binaryVoxelMap.payload, binaryVoxelMap.header.payloadSize);

	return &worldConfiguration;
}