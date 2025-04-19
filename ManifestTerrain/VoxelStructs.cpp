#include "VoxelStructs.h"

using namespace Manifest_Terrain;

VoxelBlock::~VoxelBlock()
{
	if(IS_EXTRACTED_BLOCK)
		VoxelBlockDeleter{}(this);
}

void VoxelBlockDeleter::operator()(VoxelBlock* voxelBlock)
{
	//clean up block info
	for (MFu32 face = 0; face < NUMBER_TRANSITION_FACES; ++face)
	{
		delete[]  voxelBlock->meshTransitionVertices[face];
		delete[]  voxelBlock->meshTransitionTriangles[face];
	}
	delete voxelBlock->meshVertices;
	delete voxelBlock->meshTriangles;
}