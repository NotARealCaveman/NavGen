#include "TransitionFlags.h"

using namespace Manifest_Terrain;

//returns the states that make up the current transitioning corner
void Manifest_Terrain::ExtractTransitionData(const SharedCornerData& sharedCornerData, TransitionData&& transitionData)
{
    transitionData.xFace = 2 * static_cast<MFbool>(sharedCornerData & 0x01);//face is either 0/2
    transitionData.xExtreme = static_cast<MFbool>(sharedCornerData & 0x02);
    transitionData.zFace = 1 + 2 * static_cast<MFbool>(sharedCornerData & 0x04);//face is either 1/3
    transitionData.zExtreme = static_cast<MFbool>(sharedCornerData & 0x08);
}

MFdouble Manifest_Terrain::CalculateAxisDelta(const MFint32 vertexCoord, const MFu8 chunkLOD, const MFu8 worldLOD)
{
	const MFint32 s = BASE_CELLS_PER_BLOCK << (worldLOD - chunkLOD);
	const MFint32 k = chunkLOD;
	const MFdouble wk = std::powf(2, k - 2);
	if (vertexCoord < std::powf(2, k))
		return (1 - std::powf(2, -k) * vertexCoord) * wk;
	else if (vertexCoord <= std::powf(2, k) * (s - 1))
		return 0;
	else
		return(s - 1 - std::powf(2, -k) * vertexCoord) * wk;
}