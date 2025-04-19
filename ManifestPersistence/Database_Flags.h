#pragma once

#include <ManifestUtility/Typenames.h>

using namespace Manifest_Utility;

namespace Manifest_Persistence
{
	constexpr MFu64 KEY_NOT_PRESENT{ std::numeric_limits<MFu64>::max() };
	constexpr MFu8 CHANNELS_NOT_PRESENT{ std::numeric_limits<MFu8>::max() };
	#define BUFFER_NOT_PRESENT nullptr
}