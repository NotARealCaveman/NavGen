#pragma once
#include "FlatMap.h"

namespace Manifest_Memory
{
	template<typename T>
	class SparseMap
	{
	public:
		std::unordered_set<FlatKey, FlatKey::Hash> symbols;
		std::vector<FlatMap<T>> maps;

		MFbool contains(const MFu64 representation) const
		{
			return symbols.contains(FlatKey{ .ID { representation }, .index { 0 } });
		}

		void insert(const MFu64 representation)
		{
			if (symbols.contains(FlatKey{ .ID { representation }, .index { 0 } }))
				return;

			symbols.insert(FlatKey{ .ID {representation },.index{static_cast<MFu32>(maps.size())} });
			maps.emplace_back();
		}

		FlatMap<T>& at(const MFu64 representation)
		{
			assert(contains(representation));

			const auto symbolIterator{ symbols.find(FlatKey{.ID{representation}, .index{0}}) };

			return maps.at(symbolIterator->index);
		}

		const FlatMap<T>& at(const MFu64 representation) const
		{
			assert(contains(representation));

			const auto symbolIterator{ symbols.find(FlatKey{.ID{representation}, .index{0}}) };

			return maps.at(symbolIterator->index);
		}
	};
}