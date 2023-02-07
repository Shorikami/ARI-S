#ifndef UUID_HPP
#define UUID_HPP

#include <cstdint>
#include <random>
#include <unordered_map>

namespace ARIS
{
	static std::random_device s_Random;
	static std::mt19937_64 s_Engine(s_Random());
	static std::uniform_int_distribution<uint64_t> s_UniformDist;

	class UUID
	{
	public:
		UUID()
			: m_UUID(s_UniformDist(s_Engine))
		{
		}

		UUID(uint64_t id)
			: m_UUID(id)
		{
		}

		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<ARIS::UUID>
	{
		size_t operator()(const ARIS::UUID& id) const
		{
			return (uint64_t)id;
		}
	};
}

#endif