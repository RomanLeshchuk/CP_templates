#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <memory>
#include <functional>

template <typename T>
class PersistentSegTree
{
public:
    PersistentSegTree(std::uint64_t size) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(size)) }
    {
        init(0, m_baseSize - 1, size - 1);
    }

    PersistentSegTree(const std::vector<T>& elems) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(elems.size())) }
    {
        init(0, m_baseSize - 1, elems.size() - 1, elems);
    }

    PersistentSegTree(const PersistentSegTree& tree) = default;

    PersistentSegTree& operator=(const PersistentSegTree& tree) = default;

    PersistentSegTree(PersistentSegTree&& tree) = default;

    PersistentSegTree& operator=(PersistentSegTree&& tree) = default;

    std::uint64_t getInitRoot() const
    {
        return 0;
    }

    T query(std::uint64_t root, std::uint64_t l, std::uint64_t r) const
    {
        return queryNode(root, 0, m_baseSize - 1, l, r);
    }

    std::uint64_t update(std::uint64_t root, const std::vector<std::pair<std::uint64_t, T>>& changes)
    {
        return updateNode(root, 0, m_baseSize - 1, changes);
    }

    std::int64_t leftBinSearch(const std::vector<std::uint64_t>& roots, std::int64_t l, std::int64_t r, std::function<bool(const std::vector<T>&)> func) const
    {
        std::vector<std::pair<std::uint64_t, std::vector<std::uint64_t>>> partsRoots{};

        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            std::vector<std::pair<std::uint64_t, std::uint64_t>> currRoots{};
            getParts(roots[i], 1, 0, m_baseSize - 1, l, r, currRoots);
            if (i == 0)
            {
                partsRoots.resize(currRoots.size(), { -1, std::vector<std::uint64_t>(roots.size()) });
            }
            for (std::uint64_t j = 0; j < currRoots.size(); j++)
            {
                if (i == 0)
                {
                    partsRoots[j].first = currRoots[j].first;
                }
                partsRoots[j].second[i] = currRoots[j].second;
            }
        }

        std::vector<std::vector<T>> prefNodes(partsRoots.size(), std::vector<T>(roots.size()));
        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            prefNodes[0][i] = m_nodes[partsRoots[0].second[i]].data;
        }
        for (std::uint64_t i = 1; i < partsRoots.size(); i++)
        {
            for (std::uint64_t j = 0; j < roots.size(); j++)
            {
                prefNodes[i][j] = T::calc(prefNodes[i - 1][j], m_nodes[partsRoots[i].second[j]].data);
            }
        }

        std::uint64_t partToBinSearch = 0;
        for (std::uint64_t i = 0; i < partsRoots.size() - 1; i++)
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

        std::pair<std::uint64_t, std::vector<std::uint64_t>> currNode = partsRoots[partToBinSearch];
        std::vector<T> cumulativePref = partToBinSearch ? prefNodes[partToBinSearch - 1] : std::vector<T>(roots.size());

        while (currNode.first < m_baseSize)
        {
            std::vector<T> currVal(roots.size());
            for (std::uint64_t i = 0; i < roots.size(); i++)
            {
                currVal[i] = T::calc(cumulativePref[i], m_nodes[m_nodes[currNode.second[i]].lChild].data);
            }

            if (func(currVal))
            {
                std::vector<std::uint64_t> currNodeRChilds(roots.size());
                for (std::uint64_t i = 0; i < roots.size(); i++)
                {
                    currNodeRChilds[i] = m_nodes[currNode.second[i]].rChild;
                }
                cumulativePref = currVal;
                currNode = { (currNode.first << 1) + 1, currNodeRChilds };
            }
            else
            {
                std::vector<std::uint64_t> currNodeLChilds(roots.size());
                for (std::uint64_t i = 0; i < roots.size(); i++)
                {
                    currNodeLChilds[i] = m_nodes[currNode.second[i]].lChild;
                }
                currNode = { currNode.first << 1, currNodeLChilds };
            }
        }

        std::vector<T> currVal(roots.size());
        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            currVal[i] = T::calc(cumulativePref[i], m_nodes[currNode.second[i]].data);
        }

        if (func(currVal))
        {
            return currNode.first - m_baseSize;
        }

        return (std::int64_t)currNode.first - m_baseSize - 1;
    }

    std::int64_t rightBinSearch(const std::vector<std::uint64_t>& roots, std::int64_t l, std::int64_t r, std::function<bool(const std::vector<T>&)> func) const
    {
        std::vector<std::pair<std::uint64_t, std::vector<std::uint64_t>>> partsRoots{};

        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            std::vector<std::pair<std::uint64_t, std::uint64_t>> currRoots{};
            getParts(roots[i], 1, 0, m_baseSize - 1, l, r, currRoots);
            if (i == 0)
            {
                partsRoots.resize(currRoots.size(), { -1, std::vector<std::uint64_t>(roots.size()) });
            }
            for (std::uint64_t j = 0; j < currRoots.size(); j++)
            {
                if (i == 0)
                {
                    partsRoots[j].first = currRoots[j].first;
                }
                partsRoots[j].second[i] = currRoots[j].second;
            }
        }

        std::vector<std::vector<T>> suffNodes(partsRoots.size(), std::vector<T>(roots.size()));
        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            suffNodes.back()[i] = m_nodes[partsRoots.back().second[i]].data;
        }
        for (std::int64_t i = partsRoots.size() - 2; i >= 0; i--)
        {
            for (std::uint64_t j = 0; j < roots.size(); j++)
            {
                suffNodes[i][j] = T::calc(m_nodes[partsRoots[i].second[j]].data, suffNodes[i + 1][j]);
            }
        }

        std::uint64_t partToBinSearch = partsRoots.size() - 1;
        for (std::uint64_t i = partsRoots.size() - 1; i > 0; i--)
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

        std::pair<std::uint64_t, std::vector<std::uint64_t>> currNode = partsRoots[partToBinSearch];
        std::vector<T> cumulativeSuff = partToBinSearch + 1 != partsRoots.size() ? suffNodes[partToBinSearch + 1] : std::vector<T>(roots.size());

        while (currNode.first < m_baseSize)
        {
            std::vector<T> currVal(roots.size());
            for (std::uint64_t i = 0; i < roots.size(); i++)
            {
                currVal[i] = T::calc(m_nodes[m_nodes[currNode.second[i]].rChild].data, cumulativeSuff[i]);
            }

            if (func(currVal))
            {
                std::vector<std::uint64_t> currNodeLChilds(roots.size());
                for (std::uint64_t i = 0; i < roots.size(); i++)
                {
                    currNodeLChilds[i] = m_nodes[currNode.second[i]].lChild;
                }
                cumulativeSuff = currVal;
                currNode = { currNode.first << 1, currNodeLChilds };
            }
            else
            {
                std::vector<std::uint64_t> currNodeRChilds(roots.size());
                for (std::uint64_t i = 0; i < roots.size(); i++)
                {
                    currNodeRChilds[i] = m_nodes[currNode.second[i]].rChild;
                }
                currNode = { (currNode.first << 1) + 1, currNodeRChilds };
            }
        }

        std::vector<T> currVal(roots.size());
        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            currVal[i] = T::calc(m_nodes[currNode.second[i]].data, cumulativeSuff[i]);
        }

        if (func(currVal))
        {
            return currNode.first - m_baseSize;
        }

        return currNode.first - m_baseSize + 1;
    }

private:
    struct Node
    {
        std::uint64_t lChild = std::numeric_limits<std::uint64_t>::max();
        std::uint64_t rChild = std::numeric_limits<std::uint64_t>::max();
        T data{};
    };

    std::uint64_t init(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t maxR)
    {
        std::uint64_t curr = m_nodes.size();
        m_nodes.push_back(Node{});

        if (maxR < lRange)
        {
            return curr;
        }

        if (lRange != rRange)
        {
            std::uint64_t mid = (lRange + rRange) >> 1;

            std::uint64_t l = init(lRange, mid, maxR);
            std::uint64_t r = init(mid + 1, rRange, maxR);

            m_nodes[curr].lChild = l;
            m_nodes[curr].rChild = r;
        }
        return curr;
    }

    std::uint64_t init(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t maxR, const std::vector<T>& elems)
    {
        std::uint64_t curr = m_nodes.size();
        m_nodes.push_back(Node{});

        if (maxR < lRange)
        {
            return curr;
        }

        if (lRange != rRange)
        {
            std::uint64_t mid = (lRange + rRange) >> 1;

            std::uint64_t l = init(lRange, mid, maxR, elems);
            std::uint64_t r = init(mid + 1, rRange, maxR, elems);

            m_nodes[curr].lChild = l;
            m_nodes[curr].rChild = r;

            m_nodes[curr].data = T::calc(
                m_nodes[l].data,
                m_nodes[r].data
            );
        }
        else
        {
            m_nodes[curr].data = elems[lRange];
        }
        return curr;
    }

    T queryNode(std::uint64_t node, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r) const
    {
        if (l <= lRange && rRange <= r)
        {
            return m_nodes[node].data;
        }

        if (rRange < l || r < lRange)
        {
            return T{};
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        return T::calc(
            queryNode(m_nodes[node].lChild, lRange, mid, l, r),
            queryNode(m_nodes[node].rChild, mid + 1, rRange, l, r)
        );
    }

    std::uint64_t updateNode(std::uint64_t node, std::uint64_t lRange, std::uint64_t rRange, const std::vector<std::pair<std::uint64_t, T>>& changes)
    {
        std::uint64_t newNodeIdx = m_nodes.size();
        m_nodes.push_back(Node{});

        if (lRange == rRange)
        {
            m_nodes[newNodeIdx].data = changes.back().second;
            return newNodeIdx;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        std::vector<std::pair<std::uint64_t, T>> lChanges{};
        std::vector<std::pair<std::uint64_t, T>> rChanges{};

        for (const std::pair<std::uint64_t, T>& change : changes)
        {
            if (change.first <= mid)
            {
                lChanges.push_back(change);
            }
            else
            {
                rChanges.push_back(change);
            }
        }

        if (lChanges.size())
        {
            std::uint64_t l = updateNode(m_nodes[node].lChild, lRange, mid, lChanges);
            m_nodes[newNodeIdx].lChild = l;
        }
        else
        {
            m_nodes[newNodeIdx].lChild = m_nodes[node].lChild;
        }

        if (rChanges.size())
        {
            std::uint64_t r = updateNode(m_nodes[node].rChild, mid + 1, rRange, rChanges);
            m_nodes[newNodeIdx].rChild = r;
        }
        else
        {
            m_nodes[newNodeIdx].rChild = m_nodes[node].rChild;
        }

        m_nodes[newNodeIdx].data = T::calc(
            m_nodes[m_nodes[newNodeIdx].lChild].data,
            m_nodes[m_nodes[newNodeIdx].rChild].data
        );

        return newNodeIdx;
    }

    void getParts(std::uint64_t node, std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r,
        std::vector<std::pair<std::uint64_t, std::uint64_t>>& partsRootsIndexes) const
    {
        if (l <= lRange && rRange <= r)
        {
            partsRootsIndexes.push_back(std::pair<std::uint64_t, std::uint64_t>{ startPos, node });
            return;
        }

        if (rRange < l || r < lRange)
        {
            return;
        }

        std::uint64_t mid = (lRange + rRange) >> 1;

        getParts(m_nodes[node].lChild, startPos << 1, lRange, mid, l, r, partsRootsIndexes);
        getParts(m_nodes[node].rChild, (startPos << 1) + 1, mid + 1, rRange, l, r, partsRootsIndexes);
    }

    std::uint64_t m_baseSize;
    std::vector<Node> m_nodes{};
};

struct Sum
{
    std::int64_t val = 0;

    static Sum calc(const Sum& left, const Sum& right)
    {
        return Sum{ left.val + right.val };
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

struct Max
{
    std::int64_t val = std::numeric_limits<std::int64_t>::min();

    static Max calc(const Max& left, const Max& right)
    {
        return Max{ std::max(left.val, right.val) };
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
