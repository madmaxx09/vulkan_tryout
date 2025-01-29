#pragma once



namespace wind
{
	template <typename T, typename... Rest>
	inline void hashCombine(std::size_t &seed, T const &v, Rest &&... rest) //hash function to encapsulate our vertex values into a single hash value this will be usefull to check if a vetex has already been encountered
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(int[]){0, (hashCombine(seed, std::forward<Rest>(rest)), 0)...};
	}
}