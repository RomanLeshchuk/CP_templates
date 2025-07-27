#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <memory>

template <typename T>
class ImplicitTreap
{
public:
    ImplicitTreap() = default;

    ImplicitTreap(const ImplicitTreap& implicitTreap) :
        m_root{ implicitTreap.m_root ? new Node(*implicitTreap.m_root) : nullptr }
    {
    }

    ImplicitTreap& operator=(const ImplicitTreap& implicitTreap)
    {
        if (this != &implicitTreap)
        {
            m_root = implicitTreap.m_root ? new Node(*implicitTreap.m_root) : nullptr;
        }

        return *this;
    }

    ImplicitTreap(ImplicitTreap&& implicitTreap) :
        m_root{ implicitTreap.m_root }
    {
        implicitTreap.m_root = nullptr;
    }

    ImplicitTreap& operator=(ImplicitTreap&& implicitTreap)
    {
        if (this != &implicitTreap)
        {
            m_root = implicitTreap.m_root;
            implicitTreap.m_root = nullptr;
        }

        return *this;
    }

    ImplicitTreap(const std::vector<T>& arr)
    {
        for (std::uint64_t i = 0; i < arr.size(); i++)
        {
            this->insert(i, arr[i]);
        }
    }

    ImplicitTreap split(std::uint64_t pos)
    {
        m_root = this->insert(m_root, 0, pos, new Node{});

        ImplicitTreap res{};
        res.m_root = m_root->right;

        m_root->right = nullptr;
        m_root->count = (m_root->left ? m_root->left->count : 0) + 1;

        this->erase(pos);

        return res;
    }

    void merge(ImplicitTreap&& implicitTreap)
    {
        Node* tempRoot = new Node{};
        tempRoot->left = m_root;
        tempRoot->right = implicitTreap.m_root;
        tempRoot->count = (tempRoot->left ? tempRoot->left->count : 0) + (tempRoot->right ? tempRoot->right->count : 0) + 1;

        std::uint64_t pos = this->size();
        m_root = tempRoot;
        this->erase(pos);

        implicitTreap.m_root = nullptr;
    }

    void insert(std::uint64_t pos, const T& val)
    {
        m_root = this->insert(m_root, 0, pos, new Node(val));
    }

    void erase(std::uint64_t pos)
    {
        if (!m_root)
        {
            return;
        }

        m_root = this->erase(m_root, std::min(pos, m_root->count - 1));
    }

    std::vector<T> getRange(std::uint64_t l, std::uint64_t r)
    {
        ImplicitTreap range = this->eraseRange(l, r);

        std::vector<T> res{};
        this->getRange(range.m_root, res);

        this->insertRange(l, std::move(range));

        return res;
    }

    T get(std::uint64_t pos) const
    {
        return pos < this->size() ? this->get(m_root, pos)->val : T{};
    }

    void update(std::uint64_t pos, const T& val)
    {
        if (pos < this->size())
        {
            this->update(m_root, pos, val);
        }
    }

    T getRangeQuery(std::uint64_t l, std::uint64_t r)
    {
        ImplicitTreap queryRange = this->eraseRange(l, r);

        T res = queryRange.m_root->val;

        this->insertRange(l, std::move(queryRange));

        return res;
    }

    ImplicitTreap eraseRange(std::uint64_t l, std::uint64_t r)
    {
        ImplicitTreap endPart = this->split(r + 1);
        ImplicitTreap midPart = this->split(l);

        this->merge(std::move(endPart));

        return midPart;
    }

    void insertRange(std::uint64_t pos, ImplicitTreap&& implicitTreap)
    {
        ImplicitTreap endPart = this->split(pos);

        this->merge(std::move(implicitTreap));
        this->merge(std::move(endPart));
    }

    std::uint64_t size() const
    {
        return m_root ? m_root->count : (std::uint64_t)0;
    }

    ~ImplicitTreap()
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
        Node() :
            val{},
            priority{ std::numeric_limits<std::uint64_t>::max() }
        {
        }

        Node(const T& nodeVal) :
            val{ nodeVal },
            priority{ (std::uint64_t)rand() }
        {
        }

        Node(const Node& node) :
            val{ node.val },
            priority{ node.priority },
            count{ node.count },
            left{ node.left ? new Node(*node.left) : nullptr },
            right{ node.right ? new Node(*node.right) : nullptr }
        {
        }

        ~Node()
        {
            if (left)
            {
                delete left;
            }
            if (right)
            {
                delete right;
            }
        }

        T val;
        std::uint64_t priority;
        std::uint64_t count = 1;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* rightRotate(Node* y)
    {
        Node* x = y->left;
        Node* t1 = x->left;
        Node* t2 = x->right;
        Node* t3 = y->right;

        x->right = y;
        y->left = t2;

        y->val = T::calcLeft(T::calcRight(t2 ? t2->val : T{}, T(y->val.key)), t3 ? t3->val : T{});
        y->count = (t2 ? t2->count : 0) + (t3 ? t3->count : 0) + 1;

        x->val = T::calcLeft(T::calcRight(t1 ? t1->val : T{}, T(x->val.key)), y->val);
        x->count = (t1 ? t1->count : 0) + y->count + 1;

        return x;
    }

    Node* leftRotate(Node* x)
    {
        Node* y = x->right;
        Node* t1 = x->left;
        Node* t2 = y->left;
        Node* t3 = y->right;

        y->left = x;
        x->right = t2;

        x->val = T::calcLeft(T::calcRight(t1 ? t1->val : T{}, T(x->val.key)), t2 ? t2->val : T{});
        x->count = (t1 ? t1->count : 0) + (t2 ? t2->count : 0) + 1;

        y->val = T::calcLeft(T::calcRight(x->val, T(y->val.key)), t3 ? t3->val : T{});
        y->count = x->count + (t3 ? t3->count : 0) + 1;

        return y;
    }

    Node* insert(Node* node, std::uint64_t k, std::uint64_t pos, Node* val)
    {
        if (!node)
        {
            node = val;
        }
        else if (pos <= k + (node->left ? node->left->count : 0))
        {
            node->left = this->insert(node->left, k, pos, val);
            node->count++;
            node->val = T::calcRight(val->val, node->val);

            if (node->left->priority > node->priority)
            {
                node = this->rightRotate(node);
            }
        }
        else
        {
            node->right = this->insert(node->right, k + (node->left ? node->left->count : 0) + 1, pos, val);
            node->count++;
            node->val = T::calcLeft(node->val, val->val);

            if (node->right->priority > node->priority)
            {
                node = this->leftRotate(node);
            }
        }

        return node;
    }

    Node* erase(Node* node, std::uint64_t pos)
    {
        if (pos < (node->left ? node->left->count : 0))
        {
            node->left = this->erase(node->left, pos);
            node->count--;
            node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T(node->val.key)), node->right ? node->right->val : T{});

            return node;
        }
        if (pos > (node->left ? node->left->count : 0))
        {
            node->right = this->erase(node->right, pos - (node->left ? node->left->count : 0) - 1);
            node->count--;
            node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T(node->val.key)), node->right ? node->right->val : T{});

            return node;
        }
        if (!node->left)
        {
            Node* temp = node->right;
            node->right = nullptr;
            delete node;

            return temp;
        }
        if (!node->right)
        {
            Node* temp = node->left;
            node->left = nullptr;
            delete node;

            return temp;
        }
        if (node->left->priority < node->right->priority)
        {
            node = this->leftRotate(node);

            node->left = this->erase(node->left, pos);
            node->count--;
            node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T(node->val.key)), node->right ? node->right->val : T{});

            return node;
        }

        node = this->rightRotate(node);

        node->right = this->erase(node->right, pos - (node->left ? node->left->count : 0) - 1);
        node->count--;
        node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T(node->val.key)), node->right ? node->right->val : T{});

        return node;
    }

    void getRange(Node* node, std::vector<T>& res) const
    {
        if (!node)
        {
            return;
        }

        this->getRange(node->left, res);
        res.push_back(node->val);
        this->getRange(node->right, res);
    }

    Node* get(Node* node, std::uint64_t pos) const
    {
        if (node->left && pos < node->left->count)
        {
            return this->get(node->left, pos);
        }

        if (node->right && pos > (node->left ? node->left->count : 0))
        {
            return this->get(node->right, pos - (node->left ? node->left->count : 0) - 1);
        }

        return node;
    }

    void update(Node* node, std::uint64_t pos, const T& val)
    {
        if (node->left && pos < node->left->count)
        {
            this->update(node->left, pos, val);
        }
        else if (node->right && pos > (node->left ? node->left->count : 0))
        {
            this->update(node->right, pos - (node->left ? node->left->count : 0) - 1, val);
        }
        else
        {
            node->val = val;
        }

        node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T(node->val.key)), node->right ? node->right->val : T{});
    }

    Node* m_root = nullptr;
};

struct Key
{
    Key() = default;

    Key(std::int64_t val) :
        key{ val }
    {
    }

    std::int64_t key = 0;

    static Key calcLeft(const Key& firstKey, const Key& secondKey)
    {
        return firstKey;
    }

    static Key calcRight(const Key& firstKey, const Key& secondKey)
    {
        return secondKey;
    }
};

struct Min
{
    Min() = default;

    Min(std::int64_t val) :
        key{ val },
        min{ val }
    {
    }

    Min(std::int64_t key, std::int64_t min) :
        key{ key },
        min{ min }
    {
    }

    std::int64_t key = std::numeric_limits<std::int64_t>::max();

    std::int64_t min = std::numeric_limits<std::int64_t>::max();

    static Min calcLeft(const Min& firstMin, const Min& secondMin)
    {
        return Min(firstMin.key, std::min(firstMin.min, secondMin.min));
    }

    static Min calcRight(const Min& firstMin, const Min& secondMin)
    {
        return Min(secondMin.key, std::min(firstMin.min, secondMin.min));
    }
};

struct Max
{
    Max() = default;

    Max(std::int64_t val) :
        key{ val },
        max{ val }
    {
    }

    Max(std::int64_t key, std::int64_t max) :
        key{ key },
        max{ max }
    {
    }

    std::int64_t key = std::numeric_limits<std::int64_t>::min();

    std::int64_t max = std::numeric_limits<std::int64_t>::min();

    static Max calcLeft(const Max& firstMax, const Max& secondMax)
    {
        return Max(firstMax.key, std::max(firstMax.max, secondMax.max));
    }

    static Max calcRight(const Max& firstMax, const Max& secondMax)
    {
        return Max(secondMax.key, std::max(firstMax.max, secondMax.max));
    }
};

struct Sum
{
    Sum() = default;

    Sum(std::int64_t val) :
        key{ val },
        sum{ val }
    {
    }

    Sum(std::int64_t key, std::int64_t sum) :
        key{ key },
        sum{ sum }
    {
    }

    std::int64_t key = 0;

    std::int64_t sum = 0;

    static Sum calcLeft(const Sum& firstSum, const Sum& secondSum)
    {
        return Sum(firstSum.key, firstSum.sum + secondSum.sum);
    }

    static Sum calcRight(const Sum& firstSum, const Sum& secondSum)
    {
        return Sum(secondSum.key, firstSum.sum + secondSum.sum);
    }
};