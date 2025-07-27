#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <memory>

template <typename T>
class LazyImplicitTreap
{
public:
    LazyImplicitTreap() = default;

    LazyImplicitTreap(const LazyImplicitTreap& lazyImplicitTreap) :
        m_root{ lazyImplicitTreap.m_root ? new Node(*lazyImplicitTreap.m_root) : nullptr }
    {
    }

    LazyImplicitTreap& operator=(const LazyImplicitTreap& lazyImplicitTreap)
    {
        if (this != &lazyImplicitTreap)
        {
            m_root = lazyImplicitTreap.m_root ? new Node(*lazyImplicitTreap.m_root) : nullptr;
        }

        return *this;
    }

    LazyImplicitTreap(LazyImplicitTreap&& lazyImplicitTreap) :
        m_root{ lazyImplicitTreap.m_root }
    {
        lazyImplicitTreap.m_root = nullptr;
    }

    LazyImplicitTreap& operator=(LazyImplicitTreap&& lazyImplicitTreap)
    {
        if (this != &lazyImplicitTreap)
        {
            m_root = lazyImplicitTreap.m_root;
            lazyImplicitTreap.m_root = nullptr;
        }

        return *this;
    }

    LazyImplicitTreap(const std::vector<T>& arr)
    {
        for (std::uint64_t i = 0; i < arr.size(); i++)
        {
            this->insert(i, arr[i]);
        }
    }

    LazyImplicitTreap split(std::uint64_t pos)
    {
        m_root = this->insert(m_root, 0, pos, new Node{});

        this->propagate(m_root->left);

        LazyImplicitTreap res{};
        res.m_root = m_root->right;

        m_root->right = nullptr;
        m_root->count = (m_root->left ? m_root->left->count : 0) + m_root->weight;

        this->erase(pos);

        return res;
    }

    void merge(LazyImplicitTreap&& lazyImplicitTreap)
    {
        this->propagate(m_root);
        lazyImplicitTreap.propagate(lazyImplicitTreap.m_root);

        Node* tempRoot = new Node{};
        tempRoot->left = m_root;
        tempRoot->right = lazyImplicitTreap.m_root;
        tempRoot->count = (tempRoot->left ? tempRoot->left->count : 0) + (tempRoot->right ? tempRoot->right->count : 0) + tempRoot->weight;

        std::uint64_t pos = this->size();
        m_root = tempRoot;
        this->erase(pos);

        lazyImplicitTreap.m_root = nullptr;
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

    LazyImplicitTreap eraseRange(std::uint64_t l, std::uint64_t r)
    {
        LazyImplicitTreap endPart = this->split(r + 1);
        LazyImplicitTreap midPart = this->split(l);

        this->merge(std::move(endPart));

        return midPart;
    }

    void insertRange(std::uint64_t pos, LazyImplicitTreap&& lazyImplicitTreap)
    {
        LazyImplicitTreap endPart = this->split(pos);

        this->merge(std::move(lazyImplicitTreap));
        this->merge(std::move(endPart));
    }

    std::vector<T> getRange(std::uint64_t l, std::uint64_t r)
    {
        LazyImplicitTreap range = this->eraseRange(l, r);

        std::vector<T> res{};
        this->getRange(range.m_root, res);

        this->insertRange(l, std::move(range));

        return res;
    }

    T get(std::uint64_t pos)
    {
        return pos < this->size() ? this->get(m_root, pos)->val : T{};
    }

    void updateRangeBy(std::uint64_t l, std::uint64_t r, const T& val)
    {
        LazyImplicitTreap replaceRange = this->eraseRange(l, r);

        replaceRange.m_root->lazyType = 1;
        replaceRange.m_root->lazy = val;

        this->insertRange(l, std::move(replaceRange));
    }

    void updateRangeReplace(std::uint64_t l, std::uint64_t r, const T& val)
    {
        LazyImplicitTreap replaceRange = this->eraseRange(l, r);

        replaceRange.m_root->lazyType = 2;
        replaceRange.m_root->lazy = val;

        this->insertRange(l, std::move(replaceRange));
    }

    void reverseRange(std::uint64_t l, std::uint64_t r)
    {
        LazyImplicitTreap reverseRange = this->eraseRange(l, r);

        reverseRange.m_root->lazyReverse = !reverseRange.m_root->lazyReverse;

        this->insertRange(l, std::move(reverseRange));
    }

    void cloneRange(std::uint64_t l, std::uint64_t r, std::uint64_t count)
    {
        LazyImplicitTreap cloneRange = this->eraseRange(l, r);

        if (count)
        {
            cloneRange.m_root->lazyClone = count;
            this->insertRange(l, std::move(cloneRange));
        }
    }

    T getRangeQuery(std::uint64_t l, std::uint64_t r)
    {
        LazyImplicitTreap queryRange = this->eraseRange(l, r);

        T res = queryRange.m_root->val;

        this->insertRange(l, std::move(queryRange));

        return res;
    }

    std::uint64_t size()
    {
        if (!m_root)
        {
            return 0;
        }

        this->propagate(m_root);

        return m_root->count;
    }

    ~LazyImplicitTreap()
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
            priority{ (std::uint64_t)std::rand() }
        {
        }

        Node(const Node& node) :
            val{ node.val },
            lazy{ node.lazy },
            lazyType{ node.lazyType },
            priority{ node.priority },
            count{ node.count },
            weight{ node.weight },
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
        T lazy{};
        std::uint8_t lazyType = 0;
        bool lazyReverse = false;
        std::uint64_t lazyClone = 1;
        std::uint64_t priority;
        std::uint64_t count = 1;
        std::uint64_t weight = 1;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    void propagate(Node* node)
    {
        if (!node)
        {
            return;
        }

        if (node->lazyReverse)
        {
            if (node->left)
            {
                node->left->lazyReverse = !node->left->lazyReverse;
            }
            if (node->right)
            {
                node->right->lazyReverse = !node->right->lazyReverse;
            }

            std::swap(node->left, node->right);
            node->val = T::reverse(node->val);
            node->lazyReverse = false;
        }

        if (node->lazyClone != 1)
        {
            node->count *= node->lazyClone;
            node->weight *= node->lazyClone;
            node->val = T::calcManyVal(node->val, node->lazyClone);

            if (node->left)
            {
                node->left->lazyClone *= node->lazyClone;
            }
            if (node->right)
            {
                node->right->lazyClone *= node->lazyClone;
            }

            node->lazyClone = 1;
        }

        if (!node->lazyType)
        {
            return;
        }

        if (node->lazyType == 1)
        {
            node->val = T::calcLazy(node->val, T::calcMany(node->lazy, node->count));
        }
        else
        {
            node->val = T::calcMany(node->lazy, node->count);
        }

        if (node->left)
        {
            if (node->lazyType == 1 && node->left->lazyType)
            {
                node->left->lazy = T::calcLazy(node->left->lazy, node->lazy);
            }
            else
            {
                node->left->lazy = node->lazy;
                node->left->lazyType = node->lazyType;
            }
        }

        if (node->right)
        {
            if (node->lazyType == 1 && node->right->lazyType)
            {
                node->right->lazy = T::calcLazy(node->right->lazy, node->lazy);
            }
            else
            {
                node->right->lazy = node->lazy;
                node->right->lazyType = node->lazyType;
            }
        }

        node->lazyType = 0;
    }

    Node* rightRotate(Node* y)
    {
        this->propagate(y);
        this->propagate(y->left);
        this->propagate(y->left->left);
        this->propagate(y->left->right);
        this->propagate(y->right);

        Node* x = y->left;
        Node* t1 = x->left;
        Node* t2 = x->right;
        Node* t3 = y->right;

        x->right = y;
        y->left = t2;

        y->val = T::calcLeft(T::calcRight(t2 ? t2->val : T{}, T::calcMany(y->val, y->weight)), t3 ? t3->val : T{});
        y->count = (t2 ? t2->count : 0) + (t3 ? t3->count : 0) + y->weight;

        x->val = T::calcLeft(T::calcRight(t1 ? t1->val : T{}, T::calcMany(x->val, x->weight)), y->val);
        x->count = (t1 ? t1->count : 0) + y->count + x->weight;

        return x;
    }

    Node* leftRotate(Node* x)
    {
        this->propagate(x);
        this->propagate(x->right);
        this->propagate(x->left);
        this->propagate(x->right->left);
        this->propagate(x->right->right);

        Node* y = x->right;
        Node* t1 = x->left;
        Node* t2 = y->left;
        Node* t3 = y->right;

        y->left = x;
        x->right = t2;

        x->val = T::calcLeft(T::calcRight(t1 ? t1->val : T{}, T::calcMany(x->val, x->weight)), t2 ? t2->val : T{});
        x->count = (t1 ? t1->count : 0) + (t2 ? t2->count : 0) + x->weight;

        y->val = T::calcLeft(T::calcRight(x->val, T::calcMany(y->val, y->weight)), t3 ? t3->val : T{});
        y->count = x->count + (t3 ? t3->count : 0) + y->weight;

        return y;
    }

    Node* insert(Node* node, std::uint64_t k, std::uint64_t pos, Node* val)
    {
        this->propagate(node);

        if (node)
        {
            this->propagate(node->left);
            this->propagate(node->right);
        }

        if (!node)
        {
            node = val;
            this->propagate(node);
        }
        else if (pos <= k + (node->left ? node->left->count : 0))
        {
            node->left = this->insert(node->left, k, pos, val);
            node->count += val->weight;
            node->val = T::calcRight(val->val, node->val);

            if (node->left->priority > node->priority)
            {
                node = this->rightRotate(node);
            }
        }
        else if (pos >= k + (node->left ? node->left->count : 0) + node->weight)
        {
            node->right = this->insert(node->right, k + (node->left ? node->left->count : 0) + node->weight, pos, val);
            node->count += val->weight;
            node->val = T::calcLeft(node->val, val->val);

            if (node->right->priority > node->priority)
            {
                node = this->leftRotate(node);
            }
        }
        else
        {
            std::uint64_t rightCount = k + (node->left ? node->left->count : 0) + node->weight - pos;
            node->count -= rightCount;
            node->weight -= rightCount;
            node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T::calcMany(node->val, node->weight)), node->right ? node->right->val : T{});

            Node* insertNode = new Node(node->val);
            insertNode->count = rightCount;
            insertNode->weight = rightCount;
            insertNode->val = T::calcMany(insertNode->val, insertNode->weight);

            node = this->insert(node, k, pos, insertNode);
            node = this->insert(node, k, pos, val);
        }

        return node;
    }

    Node* erase(Node* node, std::uint64_t pos)
    {
        this->propagate(node);
        this->propagate(node->left);
        this->propagate(node->right);

        if (pos < (node->left ? node->left->count : 0))
        {
            node->left = this->erase(node->left, pos);
            node->count--;
            node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T::calcMany(node->val, node->weight)), node->right ? node->right->val : T{});

            return node;
        }
        if (pos >= (node->left ? node->left->count : 0) + node->weight)
        {
            node->right = this->erase(node->right, pos - (node->left ? node->left->count : 0) - node->weight);
            node->count--;
            node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T::calcMany(node->val, node->weight)), node->right ? node->right->val : T{});

            return node;
        }
        if (node->weight != 1)
        {
            node->count--;
            node->weight--;
            node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T::calcMany(node->val, node->weight)), node->right ? node->right->val : T{});

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
            node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T::calcMany(node->val, node->weight)), node->right ? node->right->val : T{});

            return node;
        }

        node = this->rightRotate(node);

        node->right = this->erase(node->right, pos - (node->left ? node->left->count : 0) - node->weight);
        node->count--;
        node->val = T::calcLeft(T::calcRight(node->left ? node->left->val : T{}, T::calcMany(node->val, node->weight)), node->right ? node->right->val : T{});

        return node;
    }

    void getRange(Node* node, std::vector<T>& res)
    {
        if (!node)
        {
            return;
        }

        this->propagate(node);

        this->getRange(node->left, res);
        res.insert(res.end(), node->weight, node->val);
        this->getRange(node->right, res);
    }

    Node* get(Node* node, std::uint64_t pos)
    {
        this->propagate(node);
        this->propagate(node->left);

        if (pos < (node->left ? node->left->count : 0))
        {
            return this->get(node->left, pos);
        }

        if (node->right && pos >= (node->left ? node->left->count : 0) + node->weight)
        {
            return this->get(node->right, pos - (node->left ? node->left->count : 0) - node->weight);
        }

        return node;
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

    static Key calcLazy(const Key& key, const Key& lazy)
    {
        return key;
    }

    static Key calcMany(const Key& key, std::uint64_t count)
    {
        return key;
    }

    static Key calcManyVal(const Key& key, std::uint64_t count)
    {
        return key;
    }

    static Key reverse(const Key& key)
    {
        return key;
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

    static Min calcLazy(const Min& min, const Min& lazy)
    {
        return Min(min.key + lazy.key, min.min + lazy.min);
    }

    static Min calcMany(const Min& min, std::uint64_t count)
    {
        return Min(min.key);
    }

    static Min calcManyVal(const Min& min, std::uint64_t count)
    {
        return Min(min.key, min.min);
    }

    static Min reverse(const Min& min)
    {
        return min;
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

    static Max calcLazy(const Max& max, const Max& lazy)
    {
        return Max(max.key + lazy.key, max.max + lazy.max);
    }

    static Max calcMany(const Max& max, std::uint64_t count)
    {
        return Max(max.key);
    }

    static Max calcManyVal(const Max& max, std::uint64_t count)
    {
        return Max(max.key, max.max);
    }

    static Max reverse(const Max& max)
    {
        return max;
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

    static Sum calcLazy(const Sum& sum, const Sum& lazy)
    {
        return Sum(sum.key + lazy.key, sum.sum + lazy.sum);
    }

    static Sum calcMany(const Sum& sum, std::uint64_t count)
    {
        return Sum(sum.key, sum.key * (std::int64_t)count);
    }

    static Sum calcManyVal(const Sum& sum, std::uint64_t count)
    {
        return Sum(sum.key, sum.sum * (std::int64_t)count);
    }

    static Sum reverse(const Sum& sum)
    {
        return sum;
    }
};