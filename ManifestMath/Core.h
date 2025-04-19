#pragma once
#include <iostream>
#include <iomanip>

#include "Typenames.h"

namespace Manifest_Math
{
	constexpr MFfloat Pi = 3.1415927f; 
	inline MFfloat ClampRange(const MFfloat& lower, const MFfloat& a, const MFfloat& upper)
	{
		return std::fminf(std::fmaxf(lower, a), upper);
	}
	inline MFfloat Clamp_EPSILON(const MFfloat f, const MFfloat EPSILON, const MFfloat clampValue)
	{
		return std::fabsf(f) < EPSILON ? clampValue : f;
	}
}