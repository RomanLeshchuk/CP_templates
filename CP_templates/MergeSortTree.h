#pragma once

#include <vector>
#include <set>
#include <cmath>
#include <cstdint>

#include "Treap.h"

template <typename T, typename KeyType, bool isUnique>
class MST
{
public:
    MST(const std::vector<T>& elems) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(elems.size())) },
        m_tree(m_baseSize << 1, Treap<KeyType>{})
    {
        this->build(elems, 1, 0, m_baseSize - 1);
    }

    std::vector<T> getRange(std::uint64_t l, std::uint64_t r) const
    {
        Treap<KeyType> treapRange = this->getRangeRecursive(1, 0, m_baseSize - 1, l, r);

        if (!treapRange.size())
        {
            return std::vector<T>{};
        }

        std::vector<KeyType> keys = treapRange.getRange(0, treapRange.size() - 1);

        std::vector<T> vals(keys.size());

        for (std::uint64_t i = 0; i < vals.size(); i++)
        {
            vals[i] = keys[i].key;
        }

        return vals;
    }

    std::uint64_t queryLessThan(std::uint64_t l, std::uint64_t r, const T& val) const
    {
        return this->queryLessThanRecursive(1, 0, m_baseSize - 1, l, r, val);
    }

    void update(std::uint64_t pos, const T& val)
    {
        this->updateRecursive(1, 0, m_baseSize - 1, pos, KeyType(val));
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
            if constexpr (isUnique)
            {
                if (!m_tree[startPos].count(elems[l]))
                {
                    m_tree[startPos].insert(elems[l]);
                }
            }
            else
            {
                m_tree[startPos].insert(elems[l]);
            }

            return;
        }

        std::uint64_t mid = (l + r) >> 1;

        this->build(elems, startPos << 1, l, mid);
        this->build(elems, (startPos << 1) + 1, mid + 1, r);
        
        bool swapped = false;
        if (m_tree[startPos << 1].size() < m_tree[(startPos << 1) + 1].size())
        {
            std::swap(m_tree[startPos << 1], m_tree[(startPos << 1) + 1]);
            swapped = true;
        }

        m_tree[startPos] = m_tree[startPos << 1];

        if (m_tree[(startPos << 1) + 1].size())
        {
            for (const KeyType& elem : m_tree[(startPos << 1) + 1].getRange(0, m_tree[(startPos << 1) + 1].size() - 1))
            {
                if constexpr (isUnique)
                {
                    if (!m_tree[startPos].count(elem))
                    {
                        m_tree[startPos].insert(elem);
                    }
                }
                else
                {
                    m_tree[startPos].insert(elem);
                }
            }
        }

        if (swapped)
        {
            std::swap(m_tree[startPos << 1], m_tree[(startPos << 1) + 1]);
        }
    }

    Treap<KeyType> getRangeRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r) const
    {
        if (l <= lRange && rRange <= r)
        {
            return m_tree[startPos];
        }

        if (rRange < l || r < lRange)
        {
            return Treap<KeyType>{};
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        Treap<KeyType> left = this->getRangeRecursive(startPos << 1, lRange, mid, l, r);
        Treap<KeyType> right = this->getRangeRecursive((startPos << 1) + 1, mid + 1, rRange, l, r);

        if (left.size() < right.size())
        {
            std::swap(left, right);
        }

        if (right.size())
        {
            for (const KeyType& elem : right.getRange(0, right.size() - 1))
            {
                if constexpr (isUnique)
                {
                    if (!left.count(elem))
                    {
                        left.insert(elem);
                    }
                }
                else
                {
                    left.insert(elem);
                }
            }
        }

        return left;
    }

    std::uint64_t queryLessThanRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r, const T& val) const
    {
        if (l <= lRange && rRange <= r)
        {
            if (val < m_tree[startPos].getKth(0).key)
            {
                return 0;
            }

            KeyType key = m_tree[startPos].getNearestSmaller(KeyType(val));

            return m_tree[startPos].getSmallestK(key) + (key.key != val);
        }

        if (rRange < l || r < lRange)
        {
            return 0;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        return (
            this->queryLessThanRecursive(startPos << 1, lRange, mid, l, r, val)
            + this->queryLessThanRecursive((startPos << 1) + 1, mid + 1, rRange, l, r, val)
        );
    }

    KeyType updateRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t pos, const KeyType& val)
    {
        if (lRange == rRange)
        {
            KeyType oldVal = m_tree[startPos].getKth(0);

            m_tree[startPos] = Treap<KeyType>{};
            m_tree[startPos].insert(val);

            return oldVal;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        KeyType oldVal;
        if (pos <= mid)
        {
            oldVal = this->updateRecursive(startPos << 1, lRange, mid, pos, val);
        }
        else
        {
            oldVal = this->updateRecursive((startPos << 1) + 1, mid + 1, rRange, pos, val);
        }

        m_tree[startPos].erase(oldVal);
        
        if constexpr (isUnique)
        {
            if (!m_tree[startPos].count(val))
            {
                m_tree[startPos].insert(val);
            }
        }
        else
        {
            m_tree[startPos].insert(val);
        }

        return oldVal;
    }

    std::uint64_t m_baseSize;
    std::vector<Treap<KeyType>> m_tree;
};

template <typename T>
struct MSTKey
{
    MSTKey() = default;

    MSTKey(const T& val) :
        key{ val }
    {
    }

    T key{};

    static MSTKey calc(const MSTKey& firstKey, const MSTKey& secondKey)
    {
        return MSTKey(firstKey.key);
    }

    static MSTKey calcMany(const MSTKey& key, std::uint64_t count)
    {
        return MSTKey(key.key);
    }
};

template <typename T>
using MergeSortTree = MST<T, MSTKey<T>, false>;

template <typename T>
using UniqueMergeSortTree = MST<T, MSTKey<T>, true>;
