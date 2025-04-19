#include "Terrain.h"

using namespace Manifest_Terrain;

Terrain::Terrain()
	: fileSystemObserver{ fileSystemObservationToken }, worldSystemObserver{ worldSystemObservationToken } {};

void Terrain::ObserveTerrainMessages()
{	
	//currently Binary_Terrain holds no relevant data
	if (fileSystemObserver.HasPendingMessages())
		fileSystemObserver.ProcessEvents(DONOTHINGANDRETURN);
	//check if a new map has been loaded
	if (worldSystemObserver.HasPendingMessages())
	{
		std::vector<MFu32> chunkIndexCopyStatus;
		worldSystemObserver.ProcessEvents(WorldUpdate, chunkIndexCopyStatus);
		runtimeDatabase->terrainExtractionTable->Push(TerrainUpdate, ExtractUpdatedSDFNodes, ProcessWorldConfiguration,*runtimeDatabase,worldConfigurationReadIndex, chunkIndexCopyStatus, terrainGeneration,terrainExtraction);
	}
}

void Manifest_Terrain::WorldUpdate(std::vector<Message>& messages, std::vector<MFu32>& chunkIndexCopyStatus)
{			
	auto dummy{ messages[0].GetMessageContent<MFu32>() };
	chunkIndexCopyStatus = std::move(messages[1].GetMessageContent<std::vector<MFu32>>());	
}

void Manifest_Terrain::ExtractUpdatedSDFNodes(const typename RCU< WorldConfiguration, WorldConfigurationDeleter>::Handle& handle, std::vector<MFu32>& chunkIndexCopyStatus, MFu32& terrainGeneration, TerrainExtraction(&terrainExtraction)[2])
{
	const auto& worldConfiguration{ *handle.handle };
	//calculate terrain metrics
	const auto base = BASE_CELLS_PER_BLOCK;
	const auto numBlocks = base << worldConfiguration.worldLOD;
	const auto blockSizeVoxelSpace = numBlocks << WORLD_INTERPOLATION;
	const auto blockSizeWorldSpace = blockSizeVoxelSpace * WORLD_SCALE;
	const auto wsz = worldConfiguration.worldGrid.maxWorld.y;
	const auto vsz = -static_cast<MFint32>(blockSizeVoxelSpace * worldConfiguration.zChunks);
	//update the reference to TerBrain::terrainGeneration
	auto readIndex{ terrainGeneration & RCU_MODULO };
	auto nextGeneration{ ++terrainGeneration };
	auto writeIndex{ nextGeneration & RCU_MODULO };
	terrainExtraction[writeIndex].worldGrid = worldConfiguration.worldGrid;
	terrainExtraction[writeIndex].maxExtractionLOD = worldConfiguration.worldLOD;	
	terrainExtraction[writeIndex].voxelSpaceZOffset = vsz;

	//copy and update current chunks into write buffer	
	const auto extractionChunks{ worldConfiguration.xChunks * worldConfiguration.zChunks };	
	//transforms coordinates from voxel space to world space
	const MFmat3 voxelToWorld{ 1,0,0,0,0,1,0,1,0};
	const MFpoint3 v2wTranslation{ 0,0,wsz + 0.0f };

	//for each lod check each extraction index copy status
	//if the chunk is copied, copy it. else, extract and update it
	TerrainExtractionData tempTerrainExtractionData;
	PrepareTempTerrainExtractionData(tempTerrainExtractionData);
	for (auto LOD{ 0 }; LOD <= worldConfiguration.worldLOD; ++LOD)
	{		
		MFu32 chunkIndex{ 0 };
		for (const auto& chunkCopy : chunkIndexCopyStatus)
		{		
			auto terrainIndex{ chunkIndex + (LOD * extractionChunks) };	
			const TerrainExtractionData& terrainExtractionData{ terrainExtraction[readIndex].data[terrainIndex] };
			if (chunkCopy)
				terrainExtraction[writeIndex].data[terrainIndex] = terrainExtractionData;
			else
			{				
				const auto xIndex = chunkIndex % worldConfiguration.worldGrid.width;
				const auto zIndex = chunkIndex / worldConfiguration.worldGrid.width;
				const Integer3D minCorner{ xIndex * numBlocks,zIndex * numBlocks,0 };
				const auto worldMin = static_cast<MFvec3>(minCorner << WORLD_INTERPOLATION) * WORLD_SCALE;
				const auto halfBlockSizeVoxelSpace = blockSizeVoxelSpace >> 1;
				const auto halfBlockSizeWorldSpace = halfBlockSizeVoxelSpace * WORLD_SCALE;	

				TerrainExtractionData& updatedTerrainExtractionData{ terrainExtraction[writeIndex].data[terrainIndex] };
				updatedTerrainExtractionData.extractionCorner = minCorner;
				//calculate voxel and world space min corners
				updatedTerrainExtractionData.vsMinCorner = voxelToWorld*static_cast<MFvec3>(minCorner);
				updatedTerrainExtractionData.wsMinCorner = (voxelToWorld * worldMin) + v2wTranslation;
				//calculate collider data
				updatedTerrainExtractionData.terrainCollider.halfLength = halfBlockSizeWorldSpace;
				updatedTerrainExtractionData.terrainCollider.center = updatedTerrainExtractionData.wsMinCorner + updatedTerrainExtractionData.terrainCollider.halfLength;

				terrainExtraction[writeIndex].updatedChunks.emplace_back(chunkIndex);				
				//extract new map information				
				ExtractTerrainData(minCorner, worldConfiguration.voxelMap, wsz, LOD,
					worldConfiguration.worldLOD, tempTerrainExtractionData);
				CopyTempTerrainExtractionData(tempTerrainExtractionData, updatedTerrainExtractionData);
			}
			++chunkIndex;
			ResetTempTerrainExtractionData(tempTerrainExtractionData);
		}
	}
	auto& updatedChunks{ terrainExtraction[writeIndex].updatedChunks };
	auto begin{ updatedChunks.begin() };
	auto end{ updatedChunks.end() };
	std::sort(begin,end);
	updatedChunks.erase(std::unique(begin, end),end);
	terrainExtraction[writeIndex].totalExtractedChunks = extractionChunks * (worldConfiguration.worldLOD + 1);
}

void Manifest_Terrain::ProcessWorldConfiguration(ExtractionFunction& extractionFunction, ManifestRuntimeDatabase& runtimeDatabase, MFu32& worldConfigurationReadIndex, std::vector<MFu32>& chunkIndexCopyStatus, MFu32& terrainGeneration, TerrainExtraction(&terrainExtraction)[2])
{
	runtimeDatabase.worldConfigurationTable->Pull(worldConfigurationReadIndex, extractionFunction, chunkIndexCopyStatus, terrainGeneration, terrainExtraction);	
}

TerrainExtraction* Manifest_Terrain::TerrainUpdate(ExtractionFunction& extractionFunction, ConfigurationFunction& configurationFunction, ManifestRuntimeDatabase& runtimeDatabase, MFu32& worldConfigurationReadIndex, std::vector<MFu32>& chunkIndexCopyStatus, MFu32& terrainGeneration, TerrainExtraction(&terrainExtraction)[2])
{
	configurationFunction(extractionFunction, runtimeDatabase, worldConfigurationReadIndex, chunkIndexCopyStatus, terrainGeneration, terrainExtraction);
	//return a reference to the newly prepared data
	return &terrainExtraction[terrainGeneration & RCU_MODULO];
}
///INITIALIZER
MFbool Terrain::Init(ThreadPool* _threadPool, ManifestRuntimeDatabase* _runtimeDatabase)
{
	threadPool = _threadPool;
	runtimeDatabase = _runtimeDatabase;
	constexpr MFsize reserve{ (1 + MAX_TERRAIN_LOD_SUPPORT) * MAX_TERAIN_CHUNK_SUPPORT };
	//create initial shared resources that will be uploaded into the RTDB
	terrainExtraction[0].data = std::vector<TerrainExtractionData>(reserve);
	terrainExtraction[1].data = std::vector<TerrainExtractionData>(reserve);

	return true;
}

MFbool Terrain::RegisterTerrainObservers(FileSystemEventSpace& fileSystemEventSpace, WorldSystemEventSpace& worldSystemEventSpace)
{
	if (!RegisterObserverToEventSpace(fileSystemObserver, fileSystemEventSpace))
		return false;
	if (!RegisterObserverToEventSpace(worldSystemObserver, worldSystemEventSpace))
		return false;
	return true;
}
void Manifest_Terrain::DONOTHINGANDRETURN(std::vector<Message>& messages)
{	
}