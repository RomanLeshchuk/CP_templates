#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>

template <typename T>
class LazySegTree
{
public:
    LazySegTree(std::uint64_t size) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(size)) },
        m_tree(m_baseSize << 1, T{}),
        m_lazy(m_baseSize << 1, T{}),
        m_lazyType(m_baseSize << 1, 0)
    {
    }

    LazySegTree(const std::vector<T>& elems) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(elems.size())) },
        m_tree(m_baseSize << 1, T{}),
        m_lazy(m_baseSize << 1, T{}),
        m_lazyType(m_baseSize << 1, 0)
    {
        for (std::uint64_t i = 0; i < elems.size(); i++)
        {
            m_tree[m_baseSize + i] = elems[i];
        }
        for (std::uint64_t i = m_baseSize - 1; i > 0; i--)
        {
            m_tree[i] = T::calc(m_tree[i << 1], m_tree[(i << 1) + 1]);
        }
    }

    T query(std::uint64_t l, std::uint64_t r)
    {
        return this->queryRecursive(1, 0, m_baseSize - 1, l, r);
    }

    void updateBy(std::uint64_t l, std::uint64_t r, const T& val)
    {
        this->updateByRecursive(1, 0, m_baseSize - 1, l, r, val);
    }

    void updateReplace(std::uint64_t l, std::uint64_t r, const T& val)
    {
        this->updateReplaceRecursive(1, 0, m_baseSize - 1, l, r, val);
    }

private:
    void propagate(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange)
    {
        if (!m_lazyType[startPos])
        {
            return;
        }

        if (m_lazyType[startPos] == 1)
        {
            m_tree[startPos] = T::calcLazy(m_tree[startPos], T::calcMany(m_lazy[startPos], rRange - lRange + 1));
        }
        else
        {
            m_tree[startPos] = T::calcMany(m_lazy[startPos], rRange - lRange + 1);
        }

        if (lRange != rRange)
        {
            if (m_lazyType[startPos] == 1 && m_lazyType[startPos << 1])
            {
                m_lazy[startPos << 1] = T::calcLazy(m_lazy[startPos << 1], m_lazy[startPos]);
            }
            else
            {
                m_lazy[startPos << 1] = m_lazy[startPos];
                m_lazyType[startPos << 1] = m_lazyType[startPos];
            }

            if (m_lazyType[startPos] == 1 && m_lazyType[(startPos << 1) + 1])
            {
                m_lazy[(startPos << 1) + 1] = T::calcLazy(m_lazy[(startPos << 1) + 1], m_lazy[startPos]);
            }
            else
            {
                m_lazy[(startPos << 1) + 1] = m_lazy[startPos];
                m_lazyType[(startPos << 1) + 1] = m_lazyType[startPos];
            }
        }

        m_lazyType[startPos] = 0;
    }

    T queryRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r)
    {
        if (rRange < l || r < lRange)
        {
            return T{};
        }

        this->propagate(startPos, lRange, rRange);

        if (l <= lRange && rRange <= r)
        {
            return m_tree[startPos];
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        return T::calc(
            this->queryRecursive(startPos << 1, lRange, mid, l, r),
            this->queryRecursive((startPos << 1) + 1, mid + 1, rRange, l, r)
        );
    }

    void updateByRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r, const T& val)
    {
        this->propagate(startPos, lRange, rRange);

        if (l <= lRange && rRange <= r)
        {
            m_lazyType[startPos] = 1;
            m_lazy[startPos] = val;

            this->propagate(startPos, lRange, rRange);

            return;
        }

        if (rRange < l || r < lRange)
        {
            return;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        this->updateByRecursive(startPos << 1, lRange, mid, l, r, val);
        this->updateByRecursive((startPos << 1) + 1, mid + 1, rRange, l, r, val);

        m_tree[startPos] = T::calc(m_tree[startPos << 1], m_tree[(startPos << 1) + 1]);
    }

    void updateReplaceRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r, const T& val)
    {
        if (l <= lRange && rRange <= r)
        {
            m_lazyType[startPos] = 2;
            m_lazy[startPos] = val;

            this->propagate(startPos, lRange, rRange);

            return;
        }

        this->propagate(startPos, lRange, rRange);

        if (rRange < l || r < lRange)
        {
            return;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        this->updateReplaceRecursive(startPos << 1, lRange, mid, l, r, val);
        this->updateReplaceRecursive((startPos << 1) + 1, mid + 1, rRange, l, r, val);

        m_tree[startPos] = T::calc(m_tree[startPos << 1], m_tree[(startPos << 1) + 1]);
    }

    std::uint64_t m_baseSize;
    std::vector<T> m_tree;
    std::vector<T> m_lazy;
    std::vector<std::uint8_t> m_lazyType;
};

struct Min
{
    std::int64_t val = std::numeric_limits<std::int64_t>::max();

    static Min calc(const Min& left, const Min& right)
    {
        return Min{ std::min(left.val, right.val) };
    }

    static Min calcMany(const Min& min, std::uint64_t count)
    {
        return Min{ min.val };
    }

    static Min calcLazy(const Min& min, const Min& lazy)
    {
        return Min{ min.val + lazy.val };
    }
};

struct Max
{
    std::int64_t val = std::numeric_limits<std::int64_t>::min();

    static Max calc(const Max& left, const Max& right)
    {
        return Max{ std::max(left.val, right.val) };
    }

    static Max calcMany(const Max& max, std::uint64_t count)
    {
        return Max{ max.val };
    }

    static Max calcLazy(const Max& max, const Max& lazy)
    {
        return Max{ max.val + lazy.val };
    }
};

struct Sum
{
    std::int64_t val = 0;

    static Sum calc(const Sum& left, const Sum& right)
    {
        return Sum{ left.val + right.val };
    }

    static Sum calcMany(const Sum& sum, std::uint64_t count)
    {
        return Sum{ sum.val * (std::int64_t)count };
    }

    static Sum calcLazy(const Sum& sum, const Sum& lazy)
    {
        return Sum{ sum.val + lazy.val };
    }
};