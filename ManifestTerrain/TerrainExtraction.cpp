#include "TerrainExtraction.h"

using namespace Manifest_Terrain;

void Manifest_Terrain::PrepareTempTerrainExtractionData(TerrainExtractionData& tempTerrainExtractionData)
{
	auto& tempPrimaryExtractionData{ tempTerrainExtractionData.primaryTerrain.extractionData };

	tempPrimaryExtractionData.triangles.resize(MAX_PRIMARY_TRIANGLE_COUNT);	
	tempPrimaryExtractionData.vertices.resize(MAX_PRIMARY_INDEX_COUNT);
	for (auto face{ 0 }; face < NUMBER_TRANSITION_FACES; ++face)
	{
		auto& tempTransitionExtractionData{ tempTerrainExtractionData.transitionTerrain[face].extractionData };
		
		tempTransitionExtractionData.triangles.resize(MAX_PRIMARY_TRIANGLE_COUNT);		
		tempTransitionExtractionData.vertices.resize(MAX_PRIMARY_INDEX_COUNT);
	};
}

void Manifest_Terrain::CopyTempTerrainExtractionData(const TerrainExtractionData& tempTerrainExtractionData, TerrainExtractionData& finalTerrainExtractionData)
{	
	const auto& tempPrimaryExtractionData{ tempTerrainExtractionData.primaryTerrain.extractionData };
	auto& finalPrimaryExtractionData{ finalTerrainExtractionData.primaryTerrain.extractionData };

	finalPrimaryExtractionData.nTriangles = tempPrimaryExtractionData.nTriangles;
	finalPrimaryExtractionData.nVertices = tempPrimaryExtractionData.nVertices;
	finalPrimaryExtractionData.triangles.insert(finalPrimaryExtractionData.triangles.begin(), tempPrimaryExtractionData.triangles.begin(), tempPrimaryExtractionData.triangles.begin() + tempPrimaryExtractionData.nTriangles);
	finalPrimaryExtractionData.vertices.insert(finalPrimaryExtractionData.vertices.begin(), tempPrimaryExtractionData.vertices.begin(), tempPrimaryExtractionData.vertices.begin() + tempPrimaryExtractionData.nVertices);
	finalTerrainExtractionData.primaryTerrain.faceVertexIndexMap=
	 	tempTerrainExtractionData.primaryTerrain.faceVertexIndexMap;
	finalTerrainExtractionData.primaryTerrain.sharedFaceVertexIndexMap = tempTerrainExtractionData.primaryTerrain.sharedFaceVertexIndexMap;
	for (auto face{ 0 }; face < NUMBER_TRANSITION_FACES; ++face)
	{
		const auto& tempTransitionExtractionData{ tempTerrainExtractionData.transitionTerrain[face].extractionData };
		auto& finalTransitionExtractionData{ finalTerrainExtractionData.transitionTerrain[face].extractionData};

		finalTransitionExtractionData.nTriangles = tempTransitionExtractionData.nTriangles;
		finalTransitionExtractionData.nVertices = tempTransitionExtractionData.nVertices;
		finalTransitionExtractionData.triangles.insert(finalTransitionExtractionData.triangles.begin(), tempTransitionExtractionData.triangles.begin(), tempTransitionExtractionData.triangles.begin() + tempTransitionExtractionData.nTriangles);
		finalTransitionExtractionData.vertices.insert(finalTransitionExtractionData.vertices.begin(), tempTransitionExtractionData.vertices.begin(), tempTransitionExtractionData.vertices.begin() + tempTransitionExtractionData.nVertices);
		finalTerrainExtractionData.transitionTerrain[face].halfResolutionVertexIndices =
			tempTerrainExtractionData.transitionTerrain[face].halfResolutionVertexIndices;
	}
}

void Manifest_Terrain::ResetTempTerrainExtractionData(TerrainExtractionData& tempTerrainExtractionData)
{
	auto& tempPrimaryTerrain{ tempTerrainExtractionData.primaryTerrain };
	auto& tempPrimaryExtractionData{ tempTerrainExtractionData.primaryTerrain.extractionData };
	constexpr auto triangleStride{ sizeof(decltype(tempPrimaryExtractionData.triangles[0])) };
	constexpr auto vertexStride{ sizeof(decltype(tempPrimaryExtractionData.vertices[0])) };

	memset(tempPrimaryExtractionData.triangles.data(), 0, triangleStride * tempPrimaryExtractionData.triangles.size());
	memset(tempPrimaryExtractionData.vertices.data(), 0, triangleStride * tempPrimaryExtractionData.vertices.size());
	tempPrimaryExtractionData.nVertices = 0;
	tempPrimaryExtractionData.nTriangles = 0;	
	tempPrimaryTerrain.faceVertexIndexMap.clear();
	tempPrimaryTerrain.sharedFaceVertexIndexMap.clear();
	for (auto face{ 0 }; face < NUMBER_TRANSITION_FACES; ++face)
	{
		auto& tempTransitionTerrain{ tempTerrainExtractionData.transitionTerrain[face] };
		auto& tempTransitionExtractionData{ tempTransitionTerrain.extractionData };

		memset(tempTransitionExtractionData.triangles.data(), 0, triangleStride * tempTransitionExtractionData.triangles.size());
		memset(tempTransitionExtractionData.vertices.data(), 0, triangleStride * tempTransitionExtractionData.vertices.size());
		tempTransitionExtractionData.nVertices = 0;
		tempTransitionExtractionData.nTriangles = 0;
		tempTransitionTerrain.halfResolutionVertexIndices.clear();
	};
}


void Manifest_Terrain::ExtractTerrainData(const Integer3D& minCorner, const VoxelMap& map, const MFdouble worldSpaceZoffest, const MFu8 extractionLOD, const MFu8 worldLOD, TerrainExtractionData& extractionData)
{
	//extract voxel data into new block
	ExtractPrimaryTerrainData(map.field, minCorner, map.nVoxels, map.mVoxels, extractionLOD, worldLOD, extractionData.primaryTerrain);
	//extract transitions if lod is present
	if (extractionLOD)
		ExtractTransitionTerrainData(map.field, map.nVoxels, map.mVoxels, extractionLOD, worldLOD, minCorner, extractionData.transitionTerrain);
}
