#pragma once

#include <vector>
#include <numeric>
#include <cstdint>
#include <algorithm>

template <typename T>
class DSU
{
public:
	DSU(std::uint64_t size) :
		m_parent(size),
		m_rank(size),
		m_data(size)
	{
		std::iota(m_parent.begin(), m_parent.end(), 0);
	}

	DSU(const std::vector<T>& data) :
		m_parent(data.size()),
		m_rank(data.size()),
		m_data{ data }
	{
		std::iota(m_parent.begin(), m_parent.end(), 0);
	}

	std::uint64_t getRoot(std::uint64_t i)
	{
		return m_parent[i] == i ? i : m_parent[i] = this->getRoot(m_parent[i]);
	}

	void join(std::uint64_t i, std::uint64_t j)
	{
		i = this->getRoot(i);
		j = this->getRoot(j);

		if (m_rank[i] < m_rank[j])
		{
			std::swap(i, j);
		}

		m_parent[j] = i;
		m_rank[i] += m_rank[i] == m_rank[j];
		m_data[i] = T::calc(m_data[i], m_data[j]);
	}

	void updateIsolatedNode(std::uint64_t i, const T& val)
	{
		m_data[i] = val;
	}

	T queryComponent(std::uint64_t i)
	{
		return m_data[getRoot(i)];
	}

	std::uint64_t size()
	{
		return m_parent.size();
	}

private:
	std::vector<std::uint64_t> m_parent;
	std::vector<std::uint64_t> m_rank;
	std::vector<T> m_data;
};

struct Empty
{
	static Empty calc(const Empty& firstEmpty, const Empty& secondEmpty)
	{
		return Empty{};
	}
};

struct Max
{
	std::int64_t val = std::numeric_limits<std::int64_t>::min();

	static Max calc(const Max& firstMax, const Max& secondMax)
	{
		return Max{ std::max(firstMax.val, secondMax.val) };
	}
};

struct Min
{
	std::int64_t val = std::numeric_limits<std::int64_t>::max();

	static Min calc(const Min& firstMin, const Min& secondMin)
	{
		return Min{ std::min(firstMin.val, secondMin.val) };
	}
};

struct Sum
{
	std::int64_t val = 0;

	static Sum calc(const Sum& firstSum, const Sum& secondSum)
	{
		return Sum{ firstSum.val + secondSum.val };
	}
};