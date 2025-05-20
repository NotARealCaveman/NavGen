#pragma once
#include <vector>
#include <deque>
#include <unordered_set>
#include <cassert>

#include <ManifestUtility/Typenames.h>

using namespace Manifest_Utility;

namespace Manifest_Memory
{
	struct FlatKey
	{
		MFu64 ID;
		MFu32 index;
		inline MFbool operator==(const FlatKey& other) const { return ID == other.ID; };

		struct Hash
		{
			std::size_t operator()(const FlatKey& key) const
			{
				return std::hash<MFu64>()(key.ID);  // hash by ID
			}
		};
	};

	template <typename T>
	class FlatMap
	{
	public:
		std::unordered_set<FlatKey, FlatKey::Hash> keys;
		std::vector<T> values;
		std::deque<MFu32> freelist;

		MFbool contains(const MFu64 ID) const
		{
			FlatKey key{ ID,0 };
			return keys.contains(key);
		}

		void set(const MFu64 ID, T&& value)
		{
			if (contains(ID))
			{
				const auto key{ keys.find(FlatKey{ID,0}) };
				const MFu32 index{ key->index };
				values.at(index) = std::forward<T>(value);
			}
			else
				insert(std::make_pair(ID, std::forward<T>(value)));
		}

		void insert(std::pair<MFu64, T>&& insertion)
		{
			auto& [ID, value] { insertion };
			MFu32 index{ !freelist.empty() ? freelist.front() : static_cast<MFu32>(values.size()) };
			FlatKey key{ .ID { ID }, .index{ index } };
			if (index == values.size())
				values.emplace_back(std::move(value));
			else
			{
				freelist.pop_front();
				values.at(index) = std::move(value);
			}

			keys.insert(key);
		}

		void erase(const MFu64 ID)
		{
			if (!contains(ID))
				return;

			FlatKey key{ ID,0 };
			key = *keys.find(key);
			keys.erase(key);
			freelist.push_back(key.index);
		}

		T& at(const MFu64 ID)
		{
			assert(contains(ID));
			FlatKey key{ ID,0 };
			key = *keys.find(key);
			const MFu32 valueIndex{ key.index };
			return values.at(valueIndex);
		}

		const T& at(const MFu64 ID) const
		{
			assert(contains(ID));
			FlatKey key{ ID,0 };
			key = *keys.find(key);
			const MFu32 valueIndex{ key.index };
			return values.at(valueIndex);
		}

		typename std::vector<T>::iterator begin() { return values.begin(); }
		typename std::vector<T>::iterator end() { return values.end(); }
		typename std::vector<T>::const_iterator begin() const { return values.begin(); }
		typename std::vector<T>::const_iterator end() const { return values.end(); }
	};
}
