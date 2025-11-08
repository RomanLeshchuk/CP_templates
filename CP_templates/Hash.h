#pragma once

#include <cstdint>
#include <vector>

template <typename Container>
class Hash
{
public:
	Hash(const Container& container, std::uint64_t mul = 137, std::uint64_t mod = 1000000007) :
		m_mod{ mod },
		m_muls(container.size()),
		m_invMuls(container.size()),
		m_hashes(container.size())
	{
		m_muls[0] = 1;
		for (std::uint64_t i = 1; i < container.size(); ++i)
		{
			m_muls[i] = m_muls[i - 1] * mul % m_mod;
		}
		
		m_invMuls.back() = 1;
		for (std::uint64_t power = m_mod - 2, base = m_muls.back(); power; base = base * base % m_mod, power >>= 1)
		{
			if (power & 1)
			{
				m_invMuls.back() = m_invMuls.back() * base % m_mod;
			}
		}

		for (std::uint64_t i = container.size() - 1; i > 0; --i)
		{
			m_invMuls[i - 1] = m_invMuls[i] * mul % m_mod;
		}

		m_hashes[0] = container[0] % m_mod;
		for (std::uint64_t i = 1; i < container.size(); ++i)
		{
			m_hashes[i] = (m_hashes[i - 1] + (container[i] % m_mod) * m_muls[i]) % m_mod;
		}
	}

	std::uint64_t getHash(std::uint64_t l, std::uint64_t r) const
	{
		if (l == 0)
		{
			return m_hashes[r];
		}
		return (m_hashes[r] - m_hashes[l - 1] + m_mod) * m_invMuls[l] % m_mod;
	}

	std::uint64_t size() const
	{
		return m_hashes.size();
	}

private:
	std::uint64_t m_mod;
	std::vector<std::uint64_t> m_muls;
	std::vector<std::uint64_t> m_invMuls;
	std::vector<std::uint64_t> m_hashes;
};