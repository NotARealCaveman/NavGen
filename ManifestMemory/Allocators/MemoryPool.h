#pragma once
#include <memory_resource>

#include <vector>
#include <array>
#include <optional>
#include <type_traits>


namespace Manifest_Memory
{
	class MemoryPool : public std::pmr::unsynchronized_pool_resource
	{
		//todo fill out heuristic information
	};
}