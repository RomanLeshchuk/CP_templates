#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <functional>

template <typename T>
class SegTree
{
public:
    SegTree(std::uint64_t size) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(size)) },
        m_tree(m_baseSize << 1, T{})
    {
    }

    SegTree(const std::vector<T>& elems) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(elems.size())) },
        m_tree(m_baseSize << 1, T{})
    {
        for (std::uint64_t i = 0; i < elems.size(); i++)
        {
            m_tree[m_baseSize + i] = elems[i];
        }
        for (std::uint64_t i = m_baseSize - 1; i > 0; i--)
        {
            m_tree[i] = T::calc(
                m_tree[i << 1],
                m_tree[(i << 1) + 1]
            );
        }
    }

    T query(std::uint64_t l, std::uint64_t r) const
    {
        return this->queryRecursive(1, 0, m_baseSize - 1, l, r);
    }

    void update(std::uint64_t pos, const T& val)
    {
        this->updateRecursive(1, 0, m_baseSize - 1, pos, val);
    }

    std::int64_t leftBinSearch(std::int64_t l, std::int64_t r, std::function<bool(const T&)> func) const
    {
        std::vector<std::uint64_t> partsRootsIndexes{};
		this->getPartsRecursive(1, 0, m_baseSize - 1, l, r, partsRootsIndexes);

        std::vector<T> prefNodes(partsRootsIndexes.size());
        prefNodes[0] = m_tree[partsRootsIndexes[0]];
        for (std::uint64_t i = 1; i < partsRootsIndexes.size(); i++)
        {
			prefNodes[i] = T::calc(prefNodes[i - 1], m_tree[partsRootsIndexes[i]]);
        }

        std::uint64_t partToBinSearch = 0;
        for (std::uint64_t i = 0; i < partsRootsIndexes.size() - 1; i++)
        {
            if (func(prefNodes[i]))
            {
                partToBinSearch++;
            }
            else
            {
                break;
            }
        }

		std::uint64_t pos = partsRootsIndexes[partToBinSearch];
        T cumulativePref = partToBinSearch ? prefNodes[partToBinSearch - 1] : T{};

        while (pos < m_baseSize)
        {
            T currVal = T::calc(cumulativePref, m_tree[pos << 1]);
            if (func(currVal))
            {
                cumulativePref = currVal;
				pos = (pos << 1) + 1;
			}
            else
            {
                pos <<= 1;
            }
        }

        if (func(T::calc(cumulativePref, m_tree[pos])))
        {
            return pos - m_baseSize;
        }

        return (std::int64_t)pos - m_baseSize - 1;
    }

    std::int64_t rightBinSearch(std::int64_t l, std::int64_t r, std::function<bool(const T&)> func) const
    {
        std::vector<std::uint64_t> partsRootsIndexes{};
        this->getPartsRecursive(1, 0, m_baseSize - 1, l, r, partsRootsIndexes);

        std::vector<T> suffNodes(partsRootsIndexes.size());
        suffNodes.back() = m_tree[partsRootsIndexes.back()];
        for (std::int64_t i = partsRootsIndexes.size() - 2; i >= 0; i--)
        {
            suffNodes[i] = T::calc(m_tree[partsRootsIndexes[i]], suffNodes[i + 1]);
        }

        std::uint64_t partToBinSearch = partsRootsIndexes.size() - 1;
        for (std::uint64_t i = partsRootsIndexes.size() - 1; i > 0; i--)
        {
            if (func(suffNodes[i]))
            {
                partToBinSearch--;
            }
            else
            {
                break;
            }
        }

        std::uint64_t pos = partsRootsIndexes[partToBinSearch];
        T cumulativeSuff = partToBinSearch + 1 != partsRootsIndexes.size() ? suffNodes[partToBinSearch + 1] : T{};

        while (pos < m_baseSize)
        {
            T currVal = T::calc(m_tree[(pos << 1) + 1], cumulativeSuff);
            if (func(currVal))
            {
                cumulativeSuff = currVal;
                pos <<= 1;
            }
            else
            {
                pos = (pos << 1) + 1;
            }
        }

        if (func(T::calc(m_tree[pos], cumulativeSuff)))
        {
            return pos - m_baseSize;
        }

        return pos - m_baseSize + 1;
    }

    T getElem(std::uint64_t pos) const
    {
        return m_tree[m_baseSize + pos];
    }

    template <typename U>
    friend class SegTree2d;

private:
    T queryRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r) const
    {
        if (l <= lRange && rRange <= r)
        {
            return m_tree[startPos];
        }

        if (rRange < l || r < lRange)
        {
            return T{};
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        return T::calc(
            this->queryRecursive(startPos << 1, lRange, mid, l, r),
            this->queryRecursive((startPos << 1) + 1, mid + 1, rRange, l, r)
        );
    }

    void updateRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t pos, const T& val)
    {
        if (lRange == rRange)
        {
            m_tree[startPos] = val;
            return;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        if (pos <= mid)
        {
            this->updateRecursive(startPos << 1, lRange, mid, pos, val);
        }
        else
        {
            this->updateRecursive((startPos << 1) + 1, mid + 1, rRange, pos, val);
        }

        m_tree[startPos] = T::calc(
            m_tree[startPos << 1],
            m_tree[(startPos << 1) + 1]
        );
    }

    void getPartsRecursive(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r, std::vector<std::uint64_t>& partsRootsIndexes) const
    {
        if (l <= lRange && rRange <= r)
        {
			partsRootsIndexes.push_back(startPos);
            return;
        }

        if (rRange < l || r < lRange)
        {
            return;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        this->getPartsRecursive(startPos << 1, lRange, mid, l, r, partsRootsIndexes);
        this->getPartsRecursive((startPos << 1) + 1, mid + 1, rRange, l, r, partsRootsIndexes);
    }

    std::uint64_t m_baseSize;
    std::vector<T> m_tree;
};

template <typename T>
class SegTree2d
{
public:
    SegTree2d(std::uint64_t sizeI, std::uint64_t sizeJ) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(sizeI)) },
        m_tree(m_baseSize << 1, SegTree<T>(sizeJ))
    {
    }

    SegTree2d(const std::vector<std::vector<T>>& elems) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(elems.size())) },
        m_tree(m_baseSize << 1, SegTree<T>(elems.empty() ? 0 : elems[0].size()))
    {
        for (std::uint64_t i = 0; i < elems.size(); i++)
        {
            m_tree[m_baseSize + i] = SegTree<T>(elems[i]);
        }
        for (std::uint64_t i = m_baseSize - 1; i > 0; i--)
        {
            for (std::uint64_t j = 1; j < m_tree[i].m_tree.size(); j++)
            {
                m_tree[i].m_tree[j] = T::calc(
                    m_tree[i << 1].m_tree[j],
                    m_tree[(i << 1) + 1].m_tree[j]
                );
            }
        }
    }

    T query(std::uint64_t lI, std::uint64_t rI, std::uint64_t lJ, std::uint64_t rJ) const
    {
        return this->queryRecursive(1, 0, m_baseSize - 1, lI, rI, lJ, rJ);
    }

    void update(std::uint64_t posI, std::uint64_t posJ, const T& val)
    {
        this->updateRecursive(1, 0, m_baseSize - 1, posI, posJ, val);
    }

    T getElem(std::uint64_t posI, std::uint64_t posJ) const
    {
        return m_tree[m_baseSize + posI].getElem(posJ);
    }

private:
    T queryRecursive(
        std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange,
        std::uint64_t lI, std::uint64_t rI, std::uint64_t lJ, std::uint64_t rJ) const
    {
        if (lI <= lRange && rRange <= rI)
        {
            return m_tree[startPos].query(lJ, rJ);
        }

        if (rRange < lI || rI < lRange)
        {
            return T{};
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        return T::calc(
            this->queryRecursive(startPos << 1, lRange, mid, lI, rI, lJ, rJ),
            this->queryRecursive((startPos << 1) + 1, mid + 1, rRange, lI, rI, lJ, rJ)
        );
    }

    void updateRecursive(
        std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange,
        std::uint64_t posI, std::uint64_t posJ, const T& val)
    {
        if (lRange == rRange)
        {
            m_tree[startPos].update(posJ, val);
            return;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        if (posI <= mid)
        {
            this->updateRecursive(startPos << 1, lRange, mid, posI, posJ, val);
        }
        else
        {
            this->updateRecursive((startPos << 1) + 1, mid + 1, rRange, posI, posJ, val);
        }

        m_tree[startPos].update(posJ, T::calc(
            m_tree[startPos << 1].getElem(posJ),
            m_tree[(startPos << 1) + 1].getElem(posJ)
        ));
    }

    std::uint64_t m_baseSize;
    std::vector<SegTree<T>> m_tree;
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

struct Gcd
{
    std::uint64_t val = 0;

    static Gcd calc(const Gcd& left, const Gcd& right)
    {
        std::uint64_t a = left.val, b = right.val;
        if (b > a)
        {
            std::swap(a, b);
        }

        if (!a && !b)
        {
            return Gcd{ 0 };
        }
        if (!b)
        {
            return Gcd{ a };
        }

        while (b)
        {
            a %= b;
            std::swap(a, b);
        }

        return Gcd{ a };
    }
};

struct GreatestSum
{
    GreatestSum() = default;

    GreatestSum(std::int64_t initVal) :
        val{ std::max(initVal, (std::int64_t)0) },
        greatestPref{ std::max(initVal, (std::int64_t)0) },
        greatestSuff{ std::max(initVal, (std::int64_t)0) },
        sum{ initVal }
    {
    }

    std::int64_t val = 0;

    std::int64_t greatestPref = 0;
    std::int64_t greatestSuff = 0;
    std::int64_t sum = 0;

    static GreatestSum calc(const GreatestSum& left, const GreatestSum& right)
    {
        GreatestSum res{};

        res.sum = left.sum + right.sum;
        res.greatestPref = std::max(left.greatestPref, left.sum + right.greatestPref);
        res.greatestSuff = std::max(right.greatestSuff, right.sum + left.greatestSuff);

        res.val = std::max({
            left.val,
            right.val,
            left.greatestSuff + right.greatestPref
        });

        return res;
    }
};