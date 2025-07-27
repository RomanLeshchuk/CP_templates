#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <memory>

template <typename T, bool isUnique>
class ConstMST
{
public:
    ConstMST(const std::vector<T>& elems) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(elems.size())) },
        m_tree(m_baseSize << 1, std::vector<T>{})
    {
        this->build(elems, 1, 0, m_baseSize - 1);
    }

    std::vector<T> queryRange(std::uint64_t l, std::uint64_t r) const
    {
        return this->queryRangeRecursive(1, 0, m_baseSize - 1, l, r);
    }

    std::uint64_t queryCountLess(std::uint64_t l, std::uint64_t r, const T& val) const
    {
        return this->queryCountLessRecursive(1, 0, m_baseSize - 1, l, r, val);
    }

private:
    void build(const std::vector<T>& elems, std::uint64_t startPos, std::uint64_t l, std::uint64_t r)
    {
        if (l >= elems.size())
        {
            return;
        }

        if (l == r)
        {
            m_tree[startPos] = { elems[l] };
            return;
        }

        std::uint64_t mid = (l + r) >> 1;

        this->build(elems, startPos << 1, l, mid);
        this->build(elems, (startPos << 1) + 1, mid + 1, r);

        std::merge(
            m_tree[startPos << 1].begin(), m_tree[startPos << 1].end(),
            m_tree[(startPos << 1) + 1].begin(), m_tree[(startPos << 1) + 1].end(),
            std::back_inserter(m_tree[startPos])
        );

        if (isUnique && !result.empty())
        {
            std::vector<T> uniqueResult{};

            for (std::uint64_t i = 1; i < result.size(); i++)
            {
                if (result[i] != result[i - 1])
                {
                    uniqueResult.push_back(m_tree[startPos][i]);
                }
            }

            m_tree[startPos] = std::move()
        }

        if constexpr (isUnique)
        {
            std::merge(
                m_tree[startPos << 1].begin(), m_tree[startPos << 1].end(),
                m_tree[(startPos << 1) + 1].begin(), m_tree[(startPos << 1) + 1].end(),
                std::back_inserter(m_tree[startPos])
            );
        }
    }

    std::vector<T> queryRangeRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r) const
    {
        if (l <= lRange && rRange <= r)
        {
            return m_tree[startPos];
        }

        if (rRange < l || r < lRange)
        {
            return std::vector<T>{};
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        std::vector<T> left = this->queryRangeRecursive(startPos << 1, lRange, mid, l, r);
        std::vector<T> right = this->queryRangeRecursive((startPos << 1) + 1, mid + 1, rRange, l, r);
        
        std::vector<T> result{};

        std::merge(
            left.begin(), left.end(),
            right.begin(), right.end(),
            std::back_inserter(result)
        );

        if (isUnique && !result.empty())
        {
            std::vector<T> uniqueResult{};

            for (std::uint64_t i = 1; i < result.size(); i++)
            {
                if (result[i] != result[i - 1])
                {
                    uniqueResult.push_back(result[i]);
                }
            }

            return uniqueResult;
        }

        return result;
    }

    std::uint64_t queryCountLessRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r, const T& val) const
    {
        if (l <= lRange && rRange <= r)
        {
            return std::lower_bound(m_tree[startPos].begin(), m_tree[startPos].end()) - m_tree[startPos].begin();
        }

        if (rRange < l || r < lRange)
        {
            return 0;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        return (
            this->queryCountLessRecursive(startPos << 1, lRange, mid, l, r, val)
            + this->queryCountLessRecursive((startPos << 1) + 1, mid + 1, rRange, l, r)
        );
    }

    std::uint64_t m_baseSize;
    std::vector<std::vector<T>> m_tree;
};

template <typename T>
using ConstMergeSortTree = ConstMST<T, false>;

template <typename T>
using UniqueConstMergeSortTree = ConstMST<T, true>;
