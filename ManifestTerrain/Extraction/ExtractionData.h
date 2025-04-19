#pragma once
#include <array> 

#include <ManifestMath/Integer3D.h>
#include <ManifestTerrain/VoxelStructs.h>

namespace Manifest_Terrain
{
	struct ExtractionData
	{
		std::vector<Integer3D> vertices;
		std::vector<Triangle> triangles;
		MFu32 nVertices{0};
		MFu32 nTriangles{0};		
	};	

	struct PrimaryTerrain
	{
		ExtractionData extractionData;
		std::unordered_map<MFint32, MFu8> faceVertexIndexMap;
		std::unordered_map<MFint32, MFu8> sharedFaceVertexIndexMap;
	};
	struct TransitionTerrain
	{
		ExtractionData extractionData;
		std::vector<MFu32> halfResolutionVertexIndices;
	};
	constexpr MFsize MAX_PRIMARY_TRIANGLE_COUNT{ 200000 };
	constexpr MFsize MAX_PRIMARY_INDEX_COUNT{ 300000 };
	constexpr MFsize MAX_TRANSITION_TRIANGLE_COUNT{ 2000 };
	constexpr MFsize MAX_TRANSITION_INDEX_COUNT{ 3000 };
}