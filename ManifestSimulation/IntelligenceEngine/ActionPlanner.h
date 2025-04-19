#pragma once
#include <algorithm>
#include <ranges>
#include <unordered_map>
#include <cassert>

#include <ManifestMath/Point3.h>
#include <ManifestUtility/TypeAssist.h>

#include "Action.h"

using namespace Manifest_Math;

namespace Manifest_Simulation
{
	using ActionPlan = std::unordered_map<MFu64, std::vector<Action>>;
}	