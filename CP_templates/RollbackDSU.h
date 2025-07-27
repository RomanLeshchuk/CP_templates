#pragma once

#include <vector>
#include <numeric>
#include <cstdint>
#include <algorithm>
#include <stack>

template <typename T>
class RollbackDSU
{
public:
	RollbackDSU(std::uint64_t size) :
		m_parent(size),
		m_rank(size),
		m_data(size)
	{
		std::iota(m_parent.begin(), m_parent.end(), 0);
	}

	RollbackDSU(const std::vector<T>& data) :
		m_parent(data.size()),
		m_rank(data.size()),
		m_data{ data }
	{
		std::iota(m_parent.begin(), m_parent.end(), 0);
	}

	std::uint64_t getRoot(std::uint64_t i)
	{
		return m_parent[i] == i ? i : this->getRoot(m_parent[i]);
	}

	bool join(std::uint64_t i, std::uint64_t j)
	{
		i = this->getRoot(i);
		j = this->getRoot(j);

		if (m_rank[i] < m_rank[j])
		{
			std::swap(i, j);
		}

		m_history.push({ j, m_parent[j], m_data[i], m_rank[i] == m_rank[j] });

		m_parent[j] = i;
		m_rank[i] += m_rank[i] == m_rank[j];
		m_data[i] = T::calc(m_data[i], m_data[j]);

		return true;
	}

	void updateIsolatedNode(std::uint64_t i, const T& val)
	{
		m_history.push({ i, i, m_data[i], false });
		m_data[i] = val;
	}

	T queryComponent(std::uint64_t i)
	{
		return m_data[getRoot(i)];
	}

	std::uint64_t getState()
	{
		return m_history.size();
	}

	void rollback(std::uint64_t state)
	{
		while (m_history.size() > state)
		{
			m_data[m_parent[m_history.top().node]] = m_history.top().newParentPrevData;
			m_rank[m_parent[m_history.top().node]] -= m_history.top().causedRankIncrease;
			m_parent[m_history.top().node] = m_history.top().prevParent;
			m_history.pop();
		}
	}

	std::uint64_t size()
	{
		return m_parent.size();
	}

private:
	struct Update
	{
		std::uint64_t node;
		std::uint64_t prevParent;
		T newParentPrevData;
		bool causedRankIncrease;
	};

	std::vector<std::uint64_t> m_parent;
	std::vector<std::uint64_t> m_rank;
	std::vector<T> m_data;
	std::stack<Update> m_history;
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