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
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(size)) },
        m_roots{ new Node{} }
    {
        m_roots[0]->init(0, m_baseSize - 1, size - 1);
    }

    PersistentSegTree(const std::vector<T>& elems) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(elems.size())) },
        m_roots{ new Node{} }
    {
        m_roots[0]->init(0, m_baseSize - 1, elems.size() - 1, elems);
    }

    PersistentSegTree(const PersistentSegTree& tree) = delete;

    PersistentSegTree& operator=(const PersistentSegTree& tree) = delete;

    PersistentSegTree(PersistentSegTree&& tree) :
        m_baseSize{ tree.m_baseSize },
        m_roots{ std::move(tree.m_roots) }
    {
        tree.m_baseSize = 0;
        tree.m_roots.clear();
    }

    PersistentSegTree& operator=(PersistentSegTree&& tree)
    {
        if (this != &tree)
        {
            for (Node* root : m_roots)
            {
                delete root;
            }
            m_baseSize = tree.m_baseSize;
            m_roots = std::move(tree.m_roots);
            tree.m_roots.clear();
            tree.m_baseSize = 0;
        }

        return *this;
    }

    std::uint64_t getRootsSize() const
    {
        return m_roots.size();
    }

    T query(std::uint64_t root, std::uint64_t l, std::uint64_t r) const
    {
        return m_roots[root]->query(0, m_baseSize - 1, l, r);
    }

    std::uint64_t update(std::uint64_t root, const std::vector<std::pair<std::uint64_t, T>>& changes)
    {
        m_roots.push_back(m_roots[root]->update(0, m_baseSize - 1, changes, m_roots.size()));
		return m_roots.size() - 1;
    }

    std::int64_t leftBinSearch(const std::vector<std::uint64_t>& roots, std::int64_t l, std::int64_t r, std::function<bool(const std::vector<T>&)> func) const
    {
        std::vector<std::pair<std::uint64_t, std::vector<const Node*>>> partsRoots{};
        
        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            std::vector<std::pair<std::uint64_t, const Node*>> currRoots{};
            m_roots[roots[i]]->getParts(1, 0, m_baseSize - 1, l, r, currRoots);
            if (i == 0)
            {
                partsRoots.resize(currRoots.size(), { -1, std::vector<const Node*>(roots.size()) });
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
            prefNodes[0][i] = partsRoots[0].second[i]->data;
        }
        for (std::uint64_t i = 1; i < partsRoots.size(); i++)
        {
            for (std::uint64_t j = 0; j < roots.size(); j++)
            {
                prefNodes[i][j] = T::calc(prefNodes[i - 1][j], partsRoots[i].second[j]->data);
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

        std::pair<std::uint64_t, std::vector<const Node*>> currNode = partsRoots[partToBinSearch];
        std::vector<T> cumulativePref = partToBinSearch ? prefNodes[partToBinSearch - 1] : std::vector<T>(roots.size());

        while (currNode.first < m_baseSize)
        {
            std::vector<T> currVal(roots.size());
			for (std::uint64_t i = 0; i < roots.size(); i++)
			{
				currVal[i] = T::calc(cumulativePref[i], currNode.second[i]->lChild->data);
			}
            
            if (func(currVal))
            {
                std::vector<const Node*> currNodeRChilds(roots.size());
                for (std::uint64_t i = 0; i < roots.size(); i++)
                {
                    currNodeRChilds[i] = currNode.second[i]->rChild;
                }
                cumulativePref = currVal;
                currNode = { (currNode.first << 1) + 1, currNodeRChilds };
            }
            else
            {
                std::vector<const Node*> currNodeLChilds(roots.size());
                for (std::uint64_t i = 0; i < roots.size(); i++)
                {
                    currNodeLChilds[i] = currNode.second[i]->lChild;
                }
                currNode = { currNode.first << 1, currNodeLChilds };
            }
        }

        std::vector<T> currVal(roots.size());
        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            currVal[i] = T::calc(cumulativePref[i], currNode.second[i]->data);
        }

        if (func(currVal))
        {
            return currNode.first - m_baseSize;
        }

        return (std::int64_t)currNode.first - m_baseSize - 1;
    }

    std::int64_t rightBinSearch(const std::vector<std::uint64_t>& roots, std::int64_t l, std::int64_t r, std::function<bool(const std::vector<T>&)> func) const
    {
        std::vector<std::pair<std::uint64_t, std::vector<const Node*>>> partsRoots{};

        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            std::vector<std::pair<std::uint64_t, const Node*>> currRoots{};
            m_roots[roots[i]]->getParts(1, 0, m_baseSize - 1, l, r, currRoots);
            if (i == 0)
            {
                partsRoots.resize(currRoots.size(), { -1, std::vector<const Node*>(roots.size()) });
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
            suffNodes.back()[i] = partsRoots.back().second[i]->data;
        }
        for (std::int64_t i = partsRoots.size() - 2; i >= 0; i--)
        {
            for (std::uint64_t j = 0; j < roots.size(); j++)
            {
                suffNodes[i][j] = T::calc(partsRoots[i].second[j]->data, suffNodes[i + 1][j]);
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

        std::pair<std::uint64_t, std::vector<const Node*>> currNode = partsRoots[partToBinSearch];
        std::vector<T> cumulativeSuff = partToBinSearch + 1 != partsRoots.size() ? suffNodes[partToBinSearch + 1] : std::vector<T>(roots.size());

        while (currNode.first < m_baseSize)
        {
            std::vector<T> currVal(roots.size());
            for (std::uint64_t i = 0; i < roots.size(); i++)
            {
                currVal[i] = T::calc(currNode.second[i]->rChild->data, cumulativeSuff[i]);
            }

            if (func(currVal))
            {
                std::vector<const Node*> currNodeLChilds(roots.size());
                for (std::uint64_t i = 0; i < roots.size(); i++)
                {
                    currNodeLChilds[i] = currNode.second[i]->lChild;
                }
                cumulativeSuff = currVal;
                currNode = { currNode.first << 1, currNodeLChilds };
            }
            else
            {
                std::vector<const Node*> currNodeRChilds(roots.size());
                for (std::uint64_t i = 0; i < roots.size(); i++)
                {
                    currNodeRChilds[i] = currNode.second[i]->rChild;
                }
                currNode = { (currNode.first << 1) + 1, currNodeRChilds };
            }
        }

        std::vector<T> currVal(roots.size());
        for (std::uint64_t i = 0; i < roots.size(); i++)
        {
            currVal[i] = T::calc(currNode.second[i]->data, cumulativeSuff[i]);
        }

        if (func(currVal))
        {
			return currNode.first - m_baseSize;
        }

        return currNode.first - m_baseSize + 1;
    }

    ~PersistentSegTree()
    {
        for (Node* root : m_roots)
        {
            delete root;
        }
    }

private:
    class Node
    {
    public:
        Node() = default;

        void init(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t maxR)
        {
            if (maxR < lRange)
            {
                return;
            }

            if (lRange != rRange)
            {
                std::uint64_t mid = (lRange + rRange) >> 1;

                lChild = new Node{};
                lChild->init(lRange, mid, maxR);
                ownLeft = true;

                rChild = new Node{};
                rChild->init(mid + 1, rRange, maxR);
                ownRight = true;
            }
        }

        void init(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t maxR, const std::vector<T>& elems)
        {
            if (maxR < lRange)
            {
                return;
            }

            if (lRange != rRange)
            {
                std::uint64_t mid = (lRange + rRange) >> 1;

                lChild = new Node{};
                lChild->init(lRange, mid, maxR, elems);
                ownLeft = true;

                rChild = new Node{};
                rChild->init(mid + 1, rRange, maxR, elems);
                ownRight = true;

                data = T::calc(
                    lChild->data,
                    rChild->data
                );
            }
            else
            {
                data = elems[lRange];
            }
        }

        T query(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r) const
        {
            if (l <= lRange && rRange <= r)
            {
                return data;
            }

            if (rRange < l || r < lRange)
            {
                return T{};
            }

            std::uint64_t mid = (lRange + rRange) >> 1;

            return T::calc(
                lChild->query(lRange, mid, l, r),
                rChild->query(mid + 1, rRange, l, r)
            );
        }

        Node* update(std::uint64_t lRange, std::uint64_t rRange, const std::vector<std::pair<std::uint64_t, T>>& changes, std::uint64_t newOwner) const
        {
            if (lRange == rRange)
            {
                Node* newNode = new Node{};
                newNode->data = changes.back().second;

                return newNode;
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

            Node* newNode = new Node{};

            if (lChanges.size())
            {
                newNode->lChild = lChild->update(lRange, mid, lChanges, newOwner);
                newNode->ownLeft = true;
            }
            else
            {
                newNode->lChild = lChild;
            }

            if (rChanges.size())
            {
                newNode->rChild = rChild->update(mid + 1, rRange, rChanges, newOwner);
                newNode->ownRight = true;
            }
            else
            {
                newNode->rChild = rChild;
            }

            newNode->data = T::calc(
                newNode->lChild->data,
                newNode->rChild ? newNode->rChild->data : T{}
            );

            return newNode;
        }

        void getParts(std::uint64_t startPos, std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r,
                    std::vector<std::pair<std::uint64_t, const Node*>>& partsRootsIndexes) const
        {
            if (l <= lRange && rRange <= r)
            {
                partsRootsIndexes.push_back(std::pair<std::uint64_t, const Node*>{ startPos, this });
                return;
            }

            if (rRange < l || r < lRange)
            {
                return;
            }

            std::uint64_t mid = (lRange + rRange) >> 1;

            this->lChild->getParts(startPos << 1, lRange, mid, l, r, partsRootsIndexes);
            this->rChild->getParts((startPos << 1) + 1, mid + 1, rRange, l, r, partsRootsIndexes);
        }

        ~Node()
        {
            if (ownLeft)
            {
                delete lChild;
            }
            if (ownRight)
            {
                delete rChild;
            }
        }

        Node* lChild = nullptr;
        Node* rChild = nullptr;
        bool ownLeft = false;
        bool ownRight = false;
        T data{};
    };

    std::uint64_t m_baseSize;
    std::vector<Node*> m_roots{};
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