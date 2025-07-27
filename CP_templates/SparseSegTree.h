#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <memory>

template <typename T>
class SparseSegTree
{
public:
    SparseSegTree(std::uint64_t size) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(size)) }
    {
    }

    SparseSegTree(const SparseSegTree& tree) :
        m_baseSize{ tree.m_baseSize },
        m_root{ tree.m_root ? new Node(*tree.m_root) : nullptr }
    {
    }

    SparseSegTree& operator=(const SparseSegTree& tree)
    {
        if (this != &tree)
        {
            m_baseSize = tree.m_baseSize;
            m_root = tree.m_root ? new Node(*tree.m_root) : nullptr;
        }

        return *this;
    }

    SparseSegTree(SparseSegTree&& tree) :
        m_baseSize{ tree.m_baseSize },
        m_root{ tree.m_root }
    {
        tree.m_baseSize = 0;
        tree.m_root = nullptr;
    }

    SparseSegTree& operator=(SparseSegTree&& tree)
    {
        if (this != &tree)
        {
            m_baseSize = tree.m_baseSize;
            m_root = tree.m_root;
            tree.m_baseSize = 0;
            tree.m_root = nullptr;
        }

        return *this;
    }

    T query(std::uint64_t l, std::uint64_t r) const
    {
        return m_root ? m_root->query(0, m_baseSize - 1, l, r) : T{};
    }

    void update(std::uint64_t pos, const T& val)
    {
        if (!m_root)
        {
            m_root = new Node{};
        }
        m_root->update(0, m_baseSize - 1, pos, val);
    }

    ~SparseSegTree()
    {
        if (m_root)
        {
            delete m_root;
        }
    }

    template <typename U>
    friend class SparseSegTree2d;

private:
    class Node
    {
    public:
        Node() = default;

        Node(const Node& node) :
            m_lChild{ node.m_lChild ? new Node(*node.m_lChild) : nullptr },
            m_rChild{ node.m_rChild ? new Node(*node.m_rChild) : nullptr },
            m_data{ node.m_data }
        {
        }

        T query(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r) const
        {
            if (l <= lRange && rRange <= r)
            {
                return m_data;
            }

            if (rRange < l || r < lRange)
            {
                return T{};
            }

            std::uint64_t mid = (lRange + rRange) >> 1;

            return T::calc(
                m_lChild ? m_lChild->query(lRange, mid, l, r) : T{},
                m_rChild ? m_rChild->query(mid + 1, rRange, l, r) : T{}
            );
        }

        void update(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t pos, const T& val)
        {
            if (lRange == rRange)
            {
                m_data = val;

                return;
            }

            std::uint64_t mid = (lRange + rRange) >> 1;

            if (pos <= mid)
            {
                if (!m_lChild)
                {
                    m_lChild = new Node{};
                }
                m_lChild->update(lRange, mid, pos, val);
            }
            else
            {
                if (!m_rChild)
                {
                    m_rChild = new Node{};
                }
                m_rChild->update(mid + 1, rRange, pos, val);
            }

            m_data = T::calc(
                m_lChild ? m_lChild->m_data : T{},
                m_rChild ? m_rChild->m_data : T{}
            );
        }

        ~Node()
        {
            if (m_lChild)
            {
                delete m_lChild;
            }
            if (m_rChild)
            {
                delete m_rChild;
            }
        }

    private:
        Node* m_lChild = nullptr;
        Node* m_rChild = nullptr;
        T m_data{};
    };

    std::uint64_t m_baseSize;
    Node* m_root = nullptr;
};

template <typename T>
class SparseSegTree2d
{
public:
    SparseSegTree2d(std::uint64_t sizeI, std::uint64_t sizeJ) :
        m_baseSizeI{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(sizeI)) },
        m_baseSizeJ{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(sizeJ)) }
    {
    }

    SparseSegTree2d(const SparseSegTree2d& tree) :
        m_baseSizeI{ tree.m_baseSizeI },
        m_baseSizeJ{ tree.m_baseSizeJ },
        m_root{ tree.m_root ? new Node2d(*tree.m_root) : nullptr }
    {
    }

    SparseSegTree2d& operator=(const SparseSegTree2d& tree)
    {
        if (this != &tree)
        {
            m_baseSizeI = tree.m_baseSizeI;
            m_baseSizeJ = tree.m_baseSizeJ;
            m_root = tree.m_root ? new Node2d(*tree.m_root) : nullptr;
        }

        return *this;
    }

    SparseSegTree2d(SparseSegTree2d&& tree) :
        m_baseSizeI{ tree.m_baseSizeI },
        m_baseSizeJ{ tree.m_baseSizeJ },
        m_root{ tree.m_root }
    {
        tree.m_baseSizeI = 0;
        tree.m_baseSizeJ = 0;
        tree.m_root = nullptr;
    }

    SparseSegTree2d& operator=(SparseSegTree2d&& tree)
    {
        if (this != &tree)
        {
            m_baseSizeI = tree.m_baseSizeI;
            m_baseSizeJ = tree.m_baseSizeJ;
            m_root = tree.m_root;
            tree.m_baseSizeI = 0;
            tree.m_baseSizeJ = 0;
            tree.m_root = nullptr;
        }

        return *this;
    }

    T query(std::uint64_t lI, std::uint64_t rI, std::uint64_t lJ, std::uint64_t rJ) const
    {
        return m_root ? m_root->query(0, m_baseSizeI - 1, lI, rI, lJ, rJ) : T{};
    }

    void update(std::uint64_t posI, std::uint64_t posJ, const T& val)
    {
        if (!m_root)
        {
            m_root = new Node2d(m_baseSizeJ);
        }
        m_root->update(0, m_baseSizeI - 1, posI, posJ, val);
    }

    ~SparseSegTree2d()
    {
        if (m_root)
        {
            delete m_root;
        }
    }

private:
    class Node2d
    {
    public:
        Node2d(std::uint64_t baseSizeJ) :
            m_data(baseSizeJ)
        {
        }

        Node2d(const Node2d& node) :
            m_lChild{ node.m_lChild ? new Node2d(*node.m_lChild) : nullptr },
            m_rChild{ node.m_rChild ? new Node2d(*node.m_rChild) : nullptr },
            m_data{ node.m_data }
        {
        }

        T query(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t lI, std::uint64_t rI, std::uint64_t lJ, std::uint64_t rJ) const
        {
            if (lI <= lRange && rRange <= rI)
            {
                return m_data.query(lJ, rJ);
            }

            if (rRange < lI || rI < lRange)
            {
                return T{};
            }

            std::uint64_t mid = (lRange + rRange) >> 1;

            return T::calc(
                m_lChild ? m_lChild->query(lRange, mid, lI, rI, lJ, rJ) : T{},
                m_rChild ? m_rChild->query(mid + 1, rRange, lI, rI, lJ, rJ) : T{}
            );
        }

        void update(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t posI, std::uint64_t posJ, const T& val)
        {
            if (lRange == rRange)
            {
                m_data.update(posJ, val);

                return;
            }

            std::uint64_t mid = (lRange + rRange) >> 1;

            if (posI <= mid)
            {
                if (!m_lChild)
                {
                    m_lChild = new Node2d(m_data.m_baseSize);
                }
                m_lChild->update(lRange, mid, posI, posJ, val);
            }
            else
            {
                if (!m_rChild)
                {
                    m_rChild = new Node2d(m_data.m_baseSize);
                }
                m_rChild->update(mid + 1, rRange, posI, posJ, val);
            }

            m_data.update(posJ, T::calc(
                m_lChild ? m_lChild->m_data.query(posJ, posJ) : T{},
                m_rChild ? m_rChild->m_data.query(posJ, posJ) : T{}
            ));
        }

        ~Node2d()
        {
            if (m_lChild)
            {
                delete m_lChild;
            }
            if (m_rChild)
            {
                delete m_rChild;
            }
        }

    private:
        Node2d* m_lChild = nullptr;
        Node2d* m_rChild = nullptr;
        SparseSegTree<T> m_data;
    };

    std::uint64_t m_baseSizeI;
    std::uint64_t m_baseSizeJ;
    Node2d* m_root = nullptr;
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