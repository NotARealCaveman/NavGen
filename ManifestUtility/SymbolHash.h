#pragma once
#include "typenames.h"

namespace Manifest_Utility
{
	//returns the hash of the symbol in all upper case
	class SymbolHash
	{
	public:		
		SymbolHash() = default;
		inline constexpr SymbolHash(const MFu64 _hash) : hash{ _hash } {};
		constexpr SymbolHash(const char* symbol) : hash{ HashFnv1a(symbol) } {}		
		constexpr SymbolHash operator=(const SymbolHash& other) 
		{
			if (this == &other)
				return *this;

			hash = other.hash;

			return *this;
		}

		constexpr operator MFu64() const { return hash; }
	private:
		static constexpr MFu64 FNV1A_PRIME = 1099511628211ull;
		static constexpr MFu64 FNV1A_BASIS = 14695981039346656037ull;

		static constexpr char ToUpperChar(char c) {
			return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
		}

		static constexpr MFu64 HashFnv1a(const char* str, MFu64 hash = FNV1A_BASIS) {
			return (*str) ? HashFnv1a(str + 1, (hash ^ static_cast<MFu64>(ToUpperChar(*str))) * FNV1A_PRIME) : hash;
		}

		MFu64 hash{ 0 };
	};
}