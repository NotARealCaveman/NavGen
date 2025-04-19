#pragma once
#include <memory_resource>
#include <vector>
#include <array>
#include <optional>
#include <type_traits>

namespace Manifest_Memory
{ 
	class MemoryArena : public std::pmr::monotonic_buffer_resource
	{
	public:
		MemoryArena() = default;	
	};
}