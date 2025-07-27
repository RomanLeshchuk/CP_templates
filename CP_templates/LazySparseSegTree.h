#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <memory>

template <typename T>
class LazySparseSegTree
{
public:
    LazySparseSegTree(std::uint64_t size) :
        m_baseSize{ (std::uint64_t)1 << (std::uint64_t)std::ceil(std::log2(size)) }
    {
    }

    LazySparseSegTree(const LazySparseSegTree& tree) :
        m_baseSize{ tree.m_baseSize },
        m_root{ tree.m_root ? new Node(*tree.m_root) : nullptr }
    {
    }

    LazySparseSegTree& operator=(const LazySparseSegTree& tree)
    {
        if (this != &tree)
        {
            m_baseSize = tree.m_baseSize;
            m_root = tree.m_root ? new Node(*tree.m_root) : nullptr;
        }

        return *this;
    }

    LazySparseSegTree(LazySparseSegTree&& tree) :
        m_baseSize{ tree.m_baseSize },
        m_root{ tree.m_root }
    {
        tree.m_baseSize = 0;
        tree.m_root = nullptr;
    }

    LazySparseSegTree& operator=(LazySparseSegTree&& tree)
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

    T query(std::uint64_t l, std::uint64_t r)
    {
        return m_root ? m_root->query(0, m_baseSize - 1, l, r) : T{};
    }

    void updateBy(std::uint64_t l, std::uint64_t r, const T& val)
    {
        if (!m_root)
        {
            m_root = new Node{};
        }
        m_root->updateByRecursive(0, m_baseSize - 1, l, r, val);
    }

    void updateReplace(std::uint64_t l, std::uint64_t r, const T& val)
    {
        if (!m_root)
        {
            m_root = new Node{};
        }
        m_root->updateReplaceRecursive(0, m_baseSize - 1, l, r, val);
    }

    ~LazySparseSegTree()
    {
        if (m_root)
        {
            delete m_root;
        }
    }

private:
    class Node
    {
    public:
        Node() = default;

        Node(const Node& node) :
            m_lChild{ node.m_lChild ? new Node(*node.m_lChild) : nullptr },
            m_rChild{ node.m_rChild ? new Node(*node.m_rChild) : nullptr },
            m_data{ node.m_data },
            m_lazy{ node.m_lazy },
            m_lazyType{ node.m_lazyType }
        {
        }

        void propagate(std::uint64_t lRange, std::uint64_t rRange)
        {
            if (!m_lazyType)
            {
                return;
            }

            if (m_lazyType == 1)
            {
                m_data = T::calcLazy(m_data, T::calcMany(m_lazy, rRange - lRange + 1));
            }
            else
            {
                m_data = T::calcMany(m_lazy, rRange - lRange + 1);
            }

            if (lRange != rRange)
            {
                if (!m_lChild)
                {
                    m_lChild = new Node{};
                }
                if (!m_rChild)
                {
                    m_rChild = new Node{};
                }

                if (m_lazyType == 1 && m_lChild->m_lazyType)
                {
                    m_lChild->m_lazy = T::calcLazy(m_lChild->m_lazy, m_lazy);
                }
                else
                {
                    m_lChild->m_lazy = m_lazy;
                    m_lChild->m_lazyType = m_lazyType;
                }

                if (m_lazyType == 1 && m_rChild->m_lazyType)
                {
                    m_rChild->m_lazy = T::calcLazy(m_rChild->m_lazy, m_lazy);
                }
                else
                {
                    m_rChild->m_lazy = m_lazy;
                    m_rChild->m_lazyType = m_lazyType;
                }
            }

            m_lazyType = 0;
        }

        T query(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r)
        {
            this->propagate(lRange, rRange);

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

        void updateByRecursive(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r, const T& val)
        {
            this->propagate(lRange, rRange);

            if (l <= lRange && rRange <= r)
            {
                this->m_lazyType = 1;
                this->m_lazy = val;

                this->propagate(lRange, rRange);

                return;
            }

            if (rRange < l || r < lRange)
            {
                return;
            }

            std::uint64_t mid = (lRange + rRange) >> 1;


            if (!m_lChild)
            {
                m_lChild = new Node{};
            }
            if (!m_rChild)
            {
                m_rChild = new Node{};
            }

            m_lChild->updateByRecursive(lRange, mid, l, r, val);
            m_rChild->updateByRecursive(mid + 1, rRange, l, r, val);

            m_data = T::calc(m_lChild->m_data, m_rChild->m_data);
        }

        void updateReplaceRecursive(std::uint64_t lRange, std::uint64_t rRange, std::uint64_t l, std::uint64_t r, const T& val)
        {
            this->propagate(lRange, rRange);

            if (l <= lRange && rRange <= r)
            {
                this->m_lazyType = 2;
                this->m_lazy = val;

                this->propagate(lRange, rRange);

                return;
            }

            if (rRange < l || r < lRange)
            {
                return;
            }

            std::uint64_t mid = (lRange + rRange) >> 1;

            if (!m_lChild)
            {
                m_lChild = new Node{};
            }
            if (!m_rChild)
            {
                m_rChild = new Node{};
            }

            m_lChild->updateReplaceRecursive(lRange, mid, l, r, val);
            m_rChild->updateReplaceRecursive(mid + 1, rRange, l, r, val);

            m_data = T::calc(m_lChild->m_data, m_rChild->m_data);
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
        T m_lazy{};
        std::uint8_t m_lazyType = 0;
    };

    std::uint64_t m_baseSize;
    Node* m_root = nullptr;
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