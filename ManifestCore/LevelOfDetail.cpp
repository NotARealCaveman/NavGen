#include "LevelOfDetail.h"

using namespace Manifest_Core;

//DETAILING METHODS
//startings from the first camera cascade, frustum culls each chunk's collider with the cascade frustum volume
//if visibile, the cascade level is used to record the chunk LOD
VisibilitySet Manifest_Core::CullInvisibleCells(const Grid2D& worldGrid, const std::vector<MFPolyhedron>& frustumCascades, const MFsize extractedChunks, std::array<TerrainMeshConfiguration, MAX_TERAIN_CHUNK_SUPPORT>& terrainMeshConfigurations)
{
	VisibilitySet result;			
	auto totalCells{ worldGrid.cells };
	for(auto p {0}; p < totalCells; ++p)
	{
		auto& meshConfiguration{ terrainMeshConfigurations[p] };
		meshConfiguration.currentLOD = 0;
		result.emplace_back(p);			
	}
	return result;

	for (auto chunkIndex{ 0 }; chunkIndex < extractedChunks; ++chunkIndex)
	{
		auto& meshConfiguration{ terrainMeshConfigurations[chunkIndex] };
		MFu8 cascadeLevel{0};
		for (const auto& frustumCascade : frustumCascades)
		{
			if (IsVisible(NUM_FRUSTUM_PLANES, frustumCascade.planes, meshConfiguration.meshCollider))
			{
				meshConfiguration.currentLOD = cascadeLevel;
				result.emplace_back(chunkIndex);
				break;
			}
			++cascadeLevel;
		}
	}
	return result;
}

//Original detail estimates selected by the frustum culling
//Enforces the lod rules that transitions can never be +1>, by starting at the bottom node in the visible set
//and working towards the upper most, by checking pervious neighbors of the cell should the transition rule be violated the node is sent to the Continuity check function which corrects it's lod and addresses fellow neighbors as needed
void Manifest_Core::SetDetailLevels(const VisibilitySet& visibilitySet, const Grid2D& worldGrid, std::array<TerrainMeshConfiguration, MAX_TERAIN_CHUNK_SUPPORT>& terrainMeshConfigurations, MFu8 lod_override)
{	
	
	for(const auto& chunkIndex : visibilitySet)
	{		
		auto& currentConfiguration = terrainMeshConfigurations[chunkIndex];	
		
		//track lowest neighbor lod		
		MFu8 minLOD = INVISIBLE_TERRAIN;
		const auto x = chunkIndex % worldGrid.width;
		const auto z = chunkIndex / worldGrid.width;
		for (auto face = 0; face < NUMBER_TRANSITION_FACES; ++face)
		{
			auto normal = FaceNormals[face];
			const MFint32 neighborZ = z + normal.y;
			const MFint32 neighborX = x + normal.x;
			//make sure neighbor is within grid			
			if ((0 <= neighborX && neighborX < worldGrid.width) && (0 <= neighborZ && neighborZ < worldGrid.width))
			{
				//get face neighbor of cell
				const auto neighborHash = neighborX + neighborZ * worldGrid.width;
				const auto& neighborConfiguration = terrainMeshConfigurations[neighborHash];
				minLOD = std::min(minLOD, neighborConfiguration.currentLOD);
			}
		}
		if (currentConfiguration.currentLOD > minLOD + 1)
			HistoricalContinuityCheck(worldGrid, chunkIndex, minLOD + 1, currentConfiguration.currentLOD, terrainMeshConfigurations);
	}	
}

//LOD RULE:= |LODcell-LODneighbor|<=1
//recurisvely checks previously checked cells to ensure the Lod transition rule is upheld
//this is done by retroactively enforcing that if a cell has been changed any of its previously checked neighbors must also be within the lod rule. if they are not then the continuity check is performed again
//this is repeated until either the lod rule has been satisfied for all neighbors of the affects cells or the cell has reached an invisible neighbor at which point there is no need to continue
void Manifest_Core::HistoricalContinuityCheck(const Grid2D& worldGrid, const MFu32& currentHash, const MFu8& targetLod, MFu8& currentLod, std::array<TerrainMeshConfiguration, MAX_TERAIN_CHUNK_SUPPORT>& terrainMeshConfigurations)
{
	//detransition the current lod as this cell requires an lod change and will reset its continuity	
	currentLod = targetLod;
	if (currentHash % worldGrid.width)//check the x component was not the edge of the grid
	{
		auto westHash = currentHash - 1;
		auto& westNeighbor = terrainMeshConfigurations[westHash];//move back one on the grid
		if (westNeighbor.currentLOD > currentLod + 1 && westNeighbor.currentLOD != INVISIBLE_TERRAIN)
			HistoricalContinuityCheck(worldGrid,westHash, currentLod + 1, westNeighbor.currentLOD, terrainMeshConfigurations);
	}
	if ((currentHash / worldGrid.width)==worldGrid.width)//check the z component was not the edge of the grid
	{
		const auto northHash = currentHash - worldGrid.width;
		auto& northNeighbor = terrainMeshConfigurations[northHash];//move down on on the grid
		if (northNeighbor.currentLOD > currentLod + 1 && northNeighbor.currentLOD != INVISIBLE_TERRAIN)
			HistoricalContinuityCheck(worldGrid, northHash, currentLod + 1, northNeighbor.currentLOD, terrainMeshConfigurations);
	}
}

static bool init = false;
//runs over the visible set and de/transitions as detected by the enforced lod rule
void Manifest_Core::DetectTerrainTransition(const VisibilitySet& visibilitySet, const Grid2D& worldGrid, const TerrainExtraction& terrainExtraction, const MFfloat voxelSpaceZOffset, TerrainMeshNodes& terrainMeshNodes)
{
	//REWORKING TRANSITION DETECTION	
	//loop through every cell determined visible and check the de/transition states with each of its neighbors
	//should a detransition be detected due to an lod change then this cell is "reset" and has its whole mesh restored
	//should a detransition be detected due to a changed lod state on a neighbor's face then that face of the mesh only is restored
	//if a transition is detected then the transitiong face is shifted and a transitioned flag is set. 
	//once the node is finishe checking it's neighbors it checks for the transitioned flag and if it is set it makes a check for shared corners
	//if there are shared corners present they are handled at this time.
	for (const auto& chunkIndex : visibilitySet)
	{
		auto& terrainConfiguration = terrainMeshNodes.terrainMeshConfigurations[chunkIndex];
		bool transitionFlag = false;
		auto previousLod = terrainConfiguration.configurationFlags >> 4;
		auto lodChange = previousLod != terrainConfiguration.currentLOD;
		//IF ENTIRE LOD LEVEL HAS CHANGED RESTORE OLD MESH
		if (lodChange)
		{
			DLOG({ CONSOLE_BG_GREEN }, "detransitioning faces from node: " , chunkIndex , "with previous lod: " , +previousLod , " and current lod: " , +terrainConfiguration.currentLOD);
			terrainConfiguration.configurationFlags &= 0xF0;//reset transition faces but preserve previous lod
		};
		const auto x = chunkIndex % worldGrid.width;
		const auto z = chunkIndex / worldGrid.width;
		for (auto face = 0; face < NUMBER_TRANSITION_FACES; ++face)
		{
			auto normal = FaceNormals[face];
			const MFu32 neighborX = x + normal.x;
			const MFu32 neighborZ = z + normal.y;
			auto faceBit = 1 << face;
			//check special cases, (0<=t<gW[< used as indexing is 0 based])
			//furutre optimization,hopefully, just clamp the neighbor value to width and run the loop as normal - should the neighbor cell be clamped to itself the de/transition checks all fail
			if ((0 <= neighborX && neighborX < worldGrid.width) && (0 <= neighborZ && neighborZ < worldGrid.width))
			{
				//get face neighbor of cell				
				const auto neighborHash = neighborX + neighborZ * worldGrid.width;
				const auto& neighborConfiguration = terrainMeshNodes.terrainMeshConfigurations[neighborHash];
				auto currentLOD = terrainConfiguration.currentLOD;
				auto neighborLod = neighborConfiguration.currentLOD;
				auto previousNeighborLod = neighborConfiguration.configurationFlags >> 4;
				//determine transition states
				auto currentDetransition = currentLOD <= neighborLod;
				auto previousTransition = previousLod > previousNeighborLod && !lodChange;
				auto currentTransition = currentLOD > neighborLod;
				//check detransition requirements			
				if (currentDetransition && previousTransition)
				{
					//reset the vertices 
					auto extractionDataIndex{ PrimaryExtractionDataIndex(chunkIndex,terrainMeshNodes.chunksPerLOD,currentLOD) };
					//contains primary and transition extraction data
					const auto& extractionData{ terrainExtraction.data[extractionDataIndex] };
					//reference to the gfx mesh buffer
					auto& primaryMeshBuffer{ terrainMeshNodes.primaryTerrainMeshBuffers[chunkIndex] };
					auto& primaryMeshData{ terrainMeshNodes.primaryTerrainMeshData[extractionDataIndex] };
					DLOG({ CONSOLE_BG_BLUE }, "Detransition detected on face " , +face , " of node: " , chunkIndex , " at lod: " , +terrainConfiguration.currentLOD);
					//detransition and remove transition bit from node
					DetransitionFace(primaryMeshData.faceTransitionIndices[face], primaryMeshData.bufferData, primaryMeshBuffer.meshVertexBuffer);
					//detransition shared corners of face if any
					const auto& transitionCombination = TransitionCombinationTable[terrainConfiguration.configurationFlags & 0x0F];
					extractionDataIndex = TransitionExtractionDataIndex(chunkIndex, terrainMeshNodes.chunksPerLOD, currentLOD);
					if (transitionCombination.transitionCount)
						DetransitionSharedCorners(transitionCombination, terrainConfiguration.currentLOD, primaryMeshData, terrainMeshNodes.transitionTerrainMeshData[extractionDataIndex], primaryMeshBuffer, terrainMeshNodes.transitionTerrainMeshBuffers[chunkIndex]);
					//remove face transition flag
					terrainConfiguration.configurationFlags &= ~(faceBit);
				}
				//check transition requirements
				else if (currentTransition && !previousTransition)
				{
					const auto extractionDataIndex{ PrimaryExtractionDataIndex(chunkIndex,terrainMeshNodes.chunksPerLOD,currentLOD) };
					//contains primary and transition extraction data
					const auto& extractionData{ terrainExtraction.data[extractionDataIndex] };
					//reference to the gfx mesh buffer
					auto& primaryMeshBuffer{ terrainMeshNodes.primaryTerrainMeshBuffers[chunkIndex] };
					auto& primaryMeshData{ terrainMeshNodes.primaryTerrainMeshData[extractionDataIndex] };
					DLOG({ CONSOLE_BG_MAGENTA }, "Transition detected on face " , +face , " of node: " , chunkIndex , " at lod: " , +terrainConfiguration.currentLOD);
					//transition primary axis of secondary indices 
					TransitionFace(face, primaryMeshData.faceTransitionIndices[face], primaryMeshData.bufferData, extractionData.vsMinCorner, terrainConfiguration.currentLOD, terrainExtraction.maxExtractionLOD, voxelSpaceZOffset, primaryMeshBuffer.meshVertexBuffer);
					//add transition bit to node
					terrainConfiguration.configurationFlags |= faceBit;
					//set flag to sigify there must be a check for shared corners
					transitionFlag = true;
				}
			}
		}
		//see if  transition flag was set during the face checks and look for transitioning corners		
		if (transitionFlag)
		{
			auto currentLOD = terrainConfiguration.currentLOD;
			//calculate index for extracted data
			auto extractionDataIndex{ PrimaryExtractionDataIndex(chunkIndex,terrainMeshNodes.chunksPerLOD,currentLOD) };
			//contains primary and transition extraction data
			const auto& extractionData{ terrainExtraction.data[extractionDataIndex] };
			//reference to the gfx mesh buffer
			auto& primaryMeshBuffer{ terrainMeshNodes.primaryTerrainMeshBuffers[chunkIndex] };
			auto& primaryMeshData{ terrainMeshNodes.primaryTerrainMeshData[extractionDataIndex] };
			TransitionCombination transitionCombination = TransitionCombinationTable[terrainConfiguration.configurationFlags & 0x0F];
			for (auto corner = 0; corner < transitionCombination.transitionCount; ++corner)
			{
				auto cornerData = transitionCombination.cornerCombinations[corner];
				MFu8 xFace; MFu8 xExtreme; MFu8 zFace; MFu8 zExtreme;
				ExtractTransitionData(cornerData, { xFace,xExtreme,zFace,zExtreme });
				DLOG({ CONSOLE_BG_CYAN }, "Shared corner[" , +cornerData , "] found on node, xFace: " , +xFace , " zFace : " , +zFace , " xExtreme : " , +xExtreme , " zExtreme : " , +zExtreme);
				//transition extreme main mesh vertices
				//x face
				const auto& extremeXIndices = primaryMeshData.extremalIndices[xFace][zExtreme];
				TransitionFace(xFace, extremeXIndices, primaryMeshData.bufferData, extractionData.vsMinCorner, currentLOD, terrainExtraction.maxExtractionLOD, voxelSpaceZOffset, primaryMeshBuffer.meshVertexBuffer);
				//z face
				const auto& extremeZIndices = primaryMeshData.extremalIndices[zFace][xExtreme];
				TransitionFace(zFace, extremeZIndices, primaryMeshData.bufferData, extractionData.vsMinCorner, currentLOD, terrainExtraction.maxExtractionLOD, voxelSpaceZOffset, primaryMeshBuffer.meshVertexBuffer);

				//transition extreme transition mesh vertices		
				extractionDataIndex = TransitionExtractionDataIndex(chunkIndex, terrainMeshNodes.chunksPerLOD, currentLOD);
				auto& transitionTerrainMeshData{ terrainMeshNodes.transitionTerrainMeshData[extractionDataIndex] };
				auto transitionCornerX = transitionTerrainMeshData[xFace];
				auto transitionCornerZ = transitionTerrainMeshData[zFace];
				//x face primary and complimentary			
				TransitionFace(xFace, transitionCornerX.extremalIndices[zExtreme], transitionCornerX.bufferData, extractionData.vsMinCorner, currentLOD, terrainExtraction.maxExtractionLOD, voxelSpaceZOffset, terrainMeshNodes.transitionTerrainMeshBuffers[chunkIndex][xFace].meshVertexBuffer);//primary
				TransitionFace(zFace, transitionCornerX.extremalIndices[zExtreme], transitionCornerX.bufferData, extractionData.vsMinCorner, currentLOD, terrainExtraction.maxExtractionLOD, voxelSpaceZOffset, terrainMeshNodes.transitionTerrainMeshBuffers[chunkIndex][xFace].meshVertexBuffer);//complimentary	
				//z face primary and complimentary
				TransitionFace(zFace, transitionCornerZ.extremalIndices[xExtreme], transitionCornerZ.bufferData, extractionData.vsMinCorner, currentLOD, terrainExtraction.maxExtractionLOD, voxelSpaceZOffset, terrainMeshNodes.transitionTerrainMeshBuffers[chunkIndex][zFace].meshVertexBuffer);//primary
				TransitionFace(xFace, transitionCornerZ.extremalIndices[xExtreme], transitionCornerZ.bufferData, extractionData.vsMinCorner, currentLOD, terrainExtraction.maxExtractionLOD, voxelSpaceZOffset, terrainMeshNodes.transitionTerrainMeshBuffers[chunkIndex][zFace].meshVertexBuffer);//complimentary	
			}
		}	
	}	
}


//transitions the primary vertices according to the secondary vertex data of the mesh node at the current lod for a given face.
void Manifest_Core::TransitionFace(const MFu8& face, const std::vector<MFu32>& transitionIndices, const std::vector<MFvec3>& interleavedVertexData, const MFpoint3& vsMinCorner, const MFu8& chunkLOD, const MFu8 worldLOD, const MFfloat voxelSpaceZOffset, MFfloat* meshBuffer)
{
	constexpr MFu32 stride{ 6 };
	//!!THIS MUST CHANGE IF USING wsY/vsZ TRANSITIONS!!
	const auto transitionCoord = TransitionCoord(face);
	const auto iScale = 1 / WORLD_SCALE;	
	for (const auto& faceIndex : transitionIndices)
	{
		auto vertexIndex{ faceIndex * 2 };
		//DLOG(32, "Transitioning vertex: " << vertexIndex << " " << interleavedVertexData[vertexIndex]);
		//get vertex in voxel space
		auto voxelVertex = interleavedVertexData[vertexIndex];
		voxelVertex *= iScale;
		voxelVertex[2] -= voxelSpaceZOffset;
		//DLOG(33, "voxel space vertex: " << voxelVertex);
		//get coord sample
		MFint32 coordSample{ static_cast<MFint32>(voxelVertex[transitionCoord]) };
		//remove interpolation scaling        
		auto baseCoord = coordSample >> WORLD_INTERPOLATION;
		//remove offset
		auto centeredCoord = baseCoord - vsMinCorner[transitionCoord];		
		centeredCoord = std::max((int)centeredCoord, 0);
		//calculate axis delta
		auto ad = CalculateAxisDelta(centeredCoord, chunkLOD,worldLOD);		
		//get max size a cell can be at the lod and add in transition width
		auto& bufferVerex{ (*reinterpret_cast<MFvec3*>(&meshBuffer[faceIndex* stride]))};
		//DLOG(35, "pre transition vertex: " << bufferVerex);
		bufferVerex[transitionCoord] += ad * BLOCK_SIZE_WORLD_SPACE;	
		//DLOG(36, "post transition vertex: " << bufferVerex);
	}
 }
//detransition the secondary vertices back to primary vertices for mesh node at current lod
//this is done by resetting the original vertices back using their voxel space coords
void Manifest_Core::DetransitionFace(const std::vector<MFu32>& transitionIndices, const std::vector<MFvec3>& interleavedVertexData,MFfloat* meshBuffer)
{
	static constexpr MFu32 stride{ 6 };
	for (const auto& faceIndex : transitionIndices)
	{
		const auto vertexIndex{ faceIndex * 2 };
		//get vertex in voxel space
		auto voxelVertex = interleavedVertexData[vertexIndex];
		//translate from voxel space to world space		
		*reinterpret_cast<MFvec3*>(&meshBuffer[faceIndex * stride]) = voxelVertex;
	}
}

//helper function to avoid chances of bugs when detransitioing
void Manifest_Core::DetransitionSharedCorners(const TransitionCombination transitionCombination, const MFu8 transitionLOD, const PrimaryTerrainMeshData& primaryTerrainData, const TransitionTerrainMeshData(&transitionTerrainData)[4], TerrainMeshBuffer& primaryMeshBuffer, TerrainMeshBuffer(&transitionMeshBegin)[4])
{
	MFu8 xFace; MFu8 xExtreme; MFu8 zFace; MFu8 zExtreme;
	for (auto corner = 0; corner < transitionCombination.transitionCount; ++corner)
	{
		//primary terrain mesh 
		ExtractTransitionData(transitionCombination.cornerCombinations[corner], { xFace,xExtreme,zFace,zExtreme });
		//detransion extremal primary vertices on x face of the zExtreme
		const std::vector<MFu32>& primaryXCornerIndices{ primaryTerrainData.extremalIndices[xFace][zExtreme] };	
		DetransitionFace(primaryXCornerIndices, primaryTerrainData.bufferData, primaryMeshBuffer.meshVertexBuffer);
		//detransion extremal primary vertices on z face of the xExtreme
		const std::vector<MFu32>& primaryZCornerIndices{ primaryTerrainData.extremalIndices[zFace][xExtreme] };
		DetransitionFace(primaryZCornerIndices, primaryTerrainData.bufferData, primaryMeshBuffer.meshVertexBuffer);
		//transition terrain mesh 
		//detranstition extremal primary and complimentary axes on x face of the zExtreme
		const std::vector<MFu32>& transitionCornerIndicesX{ transitionTerrainData[xFace].extremalIndices[zExtreme] };
		const std::vector<MFvec3>& transitionTriangleMeshX{ transitionTerrainData[xFace].bufferData };				
		MFfloat* transitionMeshBuffer{ transitionMeshBegin[xFace].meshVertexBuffer };
		DetransitionFace(transitionCornerIndicesX, transitionTriangleMeshX, transitionMeshBuffer);
		//detranstition extremal primary and complimentary axes on z face of the xExtreme
		const std::vector<MFu32>& transitionCornerIndicesZ{ transitionTerrainData[zFace].extremalIndices[xExtreme] };
		const std::vector<MFvec3>& transitionTriangleMeshZ{ transitionTerrainData[zFace].bufferData };
		transitionMeshBuffer = transitionMeshBegin[zFace].meshVertexBuffer;
		DetransitionFace(transitionCornerIndicesZ, transitionTriangleMeshZ, transitionMeshBuffer);
	}
}