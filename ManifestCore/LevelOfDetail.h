#pragma once

#include <ManifestTerrain/TerrainFlags.h>
#include <ManifestTerrain/TransitionFlags.h>
#include <ManifestTerrain/TerrainExtraction.h>
#include <ManifestMath/Visibility.h>
#include <ManifestGraphics/Meshs/TerrainMesh.h>

using namespace Manifest_Math;
using namespace Manifest_Terrain;
using namespace Manifest_Graphics;

namespace Manifest_Core
{   
    //DETAILING PRIMARY METHODS1
    //first pass - create visible set and remove/detransition invisible nodes as needed
    VisibilitySet CullInvisibleCells(const Grid2D& worldGrid, const std::vector<MFPolyhedron>& frustumCascades,const MFsize extractedChunks, std::array<TerrainMeshConfiguration,MAX_TERAIN_CHUNK_SUPPORT>& terrainMeshConfigurations);
    //second pass - finalize lod 
    void SetDetailLevels(const VisibilitySet& visibilitySet, const Grid2D& worldGrid, std::array<TerrainMeshConfiguration, MAX_TERAIN_CHUNK_SUPPORT>& terrainMeshConfigurations, MFu8 lod_override);
    //enforces the lod rule
    void HistoricalContinuityCheck(const Grid2D& worldGrid, const MFu32& currentHash, const MFu8& targetLod, MFu8& currentLod, std::array<TerrainMeshConfiguration, MAX_TERAIN_CHUNK_SUPPORT>& terrainMeshConfigurations);
    //detects and de/transtions on the visible set
    void DetectTerrainTransition(const VisibilitySet& visibilitySet, const Grid2D& worldGrid,const TerrainExtraction& terrainExtraction, const MFfloat voxelSpaceZOffset, TerrainMeshNodes& terrainMeshNodes);
    ///DETAILING SECONDARY METHODS
    //transitions the selected secondary indices by the normal defined by the face variable
    void TransitionFace(const MFu8& face, const std::vector<MFu32>& transitionIndices, const std::vector<MFvec3>& interleavedVertexData, const MFpoint3& vsMinCorner, const MFu8& chunkLOD, const MFu8 worldLOD, const MFfloat voxelSpaceZOffset, MFfloat* meshBuffer);
    //resets the secondary vertices to their primary positions 
    void DetransitionFace(const std::vector<MFu32>& transitionIndices, const std::vector<MFvec3>& interleavedVertexData,MFfloat* meshBuffer);
   //restores a transitioned corner to its proper state 
    void DetransitionSharedCorners(const TransitionCombination transitionCombination, const MFu8 transitionLOD, const PrimaryTerrainMeshData& primaryTerrainData, const TransitionTerrainMeshData(&transitionTerrainData)[4], TerrainMeshBuffer& primaryMeshBuffer, TerrainMeshBuffer(& transitionMeshBegin)[4]);
}