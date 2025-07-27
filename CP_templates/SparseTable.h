#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>

template <typename T>
class SparseTable
{
public:
	SparseTable(const std::vector<T>& elems) :
		m_precomputedLogs(elems.size()),
		m_table(std::log2(elems.size()) + 1, std::vector<T>(elems.size()))
	{
		for (std::uint64_t i = 0; i < elems.size(); ++i)
		{
			m_precomputedLogs[i] = std::log2(i + 1);
		}

		m_table[0] = elems;

		for (std::uint64_t i = 1; i < m_table.size(); ++i)
		{
			for (std::uint64_t j = 0; j + ((std::uint64_t)1 << i) <= elems.size(); ++j)
			{
				m_table[i][j] = T::calc(m_table[i - 1][j], m_table[i - 1][j + ((std::uint64_t)1 << (i - 1))]);
			}
		}
	}

	T query(std::uint64_t l, std::uint64_t r) const
	{
		return T::calc(
			m_table[m_precomputedLogs[r - l]][l],
			m_table[m_precomputedLogs[r - l]][r - ((std::uint64_t)1 << m_precomputedLogs[r - l]) + 1]
		);
	}

private:
	std::vector<std::uint64_t> m_precomputedLogs;
	std::vector<std::vector<T>> m_table;
};

struct Max
{
	std::int64_t val = std::numeric_limits<std::int64_t>::min();

	static Max calc(const Max& left, const Max& right)
	{
		return Max{ std::max(left.val, right.val) };
	}
};

struct Min
{
	std::int64_t val = std::numeric_limits<std::int64_t>::max();

	static Min calc(const Min& left, const Min& right)
	{
		return Min{ std::min(left.val, right.val) };
	}
};