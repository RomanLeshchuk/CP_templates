#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>

template <typename T>
class DisjointSparseTable
{
public:
    DisjointSparseTable(const std::vector<T>& elems) :
        m_precomputedLogs(elems.size() << 1)
    {
        for (std::uint64_t i = 2; i < m_precomputedLogs.size(); ++i)
        {
            m_precomputedLogs[i] = m_precomputedLogs[i >> 1] + 1;
        }
        m_table.resize(m_precomputedLogs[elems.size()] + 1, std::vector<T>(elems.size()));

        for (std::uint64_t i = 0; i < m_table.size(); ++i)
        {
            for (std::uint64_t j = 0; j < elems.size(); j += (std::uint64_t)1 << (i + 1))
            {
                std::uint64_t mid = std::min(j + ((std::uint64_t)1 << i), elems.size());
                m_table[i][mid - 1] = elems[mid - 1];
                for (std::uint64_t k = mid; k > j + 1; --k)
                {
                    m_table[i][k - 2] = T::calc(elems[k - 2], m_table[i][k - 1]);
                }
                if (mid != elems.size())
                {
                    m_table[i][mid] = elems[mid];
                }
                std::uint64_t blockEnd = std::min(j + ((std::uint64_t)1 << (i + 1)), elems.size());
                for (std::uint64_t k = mid + 1; k < blockEnd; ++k)
                {
                    m_table[i][k] = T::calc(m_table[i][k - 1], elems[k]);
                }
            }
        }
    }

    T query(std::uint64_t l, std::uint64_t r) const
    {
        if (l == r)
        {
            return m_table[0][l];
        }
        return T::calc(
            m_table[m_precomputedLogs[l ^ r]][l],
            m_table[m_precomputedLogs[l ^ r]][r]
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

struct Sum
{
	std::int64_t val = 0;

	static Sum calc(const Sum& left, const Sum& right)
	{
		return Sum{ left.val + right.val };
	}
};