#include "DepthStructures.h"

using namespace Manifest_Core;

CascadedCameraView::CascadedCameraView(const Camera& cam)
{	
	cam.SplitCameraClip(zSplits);//create camera splits
	for (auto&& i = 0; i < numCascades; ++i)
		projSplits[i] = cam.SplitCameraProj(zSplits[i], zSplits[i + 1]);
}

