#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <memory>

template <typename T>
class Treap
{
public:
    Treap() = default;

    Treap(const Treap& treap) :
        m_root{ treap.m_root ? new Node(*treap.m_root) : nullptr }
    {
    }

    Treap& operator=(const Treap& treap)
    {
        if (this != &treap)
        {
            delete m_root;
            m_root = treap.m_root ? new Node(*treap.m_root) : nullptr;
        }

        return *this;
    }

    Treap(Treap&& treap) :
        m_root{ treap.m_root }
    {
        treap.m_root = nullptr;
    }

    Treap& operator=(Treap&& treap)
    {
        if (this != &treap)
        {
            delete m_root;
            m_root = treap.m_root;
            treap.m_root = nullptr;
        }

        return *this;
    }

    void insert(const T& val, std::uint64_t count = 1)
    {
        if (!count)
        {
            return;
        }

        m_root = this->insert(m_root, val, count);
    }

    std::uint64_t count(const T& val) const
    {
        return this->count(m_root, val);
    }

    std::uint64_t erase(const T& val, std::uint64_t count = 1)
    {
        if (!count)
        {
            return 0;
        }

        std::pair<Node*, std::uint64_t> data = this->erase(m_root, val, count);
        m_root = data.first;

        return data.second;
    }

    std::vector<T> getRange(std::uint64_t l, std::uint64_t r) const
    {
        std::vector<T> res{};

        this->getRange(m_root, 0, res, l, r);

        return res;
    }

    T getKth(std::uint64_t k) const
    {
        return k < this->size() ? this->getKth(m_root, k)->val : T{};
    }

    T getRangeQuery(std::uint64_t l, std::uint64_t r) const
    {
        T res{};

        this->getRangeQuery(m_root, 0, res, l, r);

        return res;
    }

    std::uint64_t size() const
    {
        return m_root ? m_root->count : (std::uint64_t)0;
    }

    T getNearestSmaller(const T& val) const
    {
        Node* res = nullptr;
        this->getNearestSmaller(m_root, val, res);

        return res ? res->val : T{};
    }

    T getNearestBigger(const T& val) const
    {
        Node* res = nullptr;
        this->getNearestBigger(m_root, val, res);

        return res ? res->val : T{};
    }

    std::uint64_t getSmallestK(const T& val) const
    {
        std::uint64_t counter = 0;
        Node* res = this->getSmallestK(m_root, val, counter);

        return res ? counter : this->size();
    }

    std::uint64_t getBiggestK(const T& val) const
    {
        std::uint64_t counter = 0;
        Node* res = this->getBiggestK(m_root, val, counter);

        return res ? counter : this->size();
    }

    ~Treap()
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
        Node(const T& nodeVal, std::uint64_t nodeWeight) :
            val{ T::calcMany(nodeVal, nodeWeight) },
            weight{ nodeWeight },
            count{ nodeWeight },
            priority{ (std::uint64_t)std::rand() }
        {
        }

        Node(const Node& node) :
            val{ node.val },
            priority{ node.priority },
            weight{ node.weight },
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
        std::uint64_t weight;
        std::uint64_t count;
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

        y->val = T::calc(T::calcMany(y->val, y->weight), T::calc(t2 ? t2->val : T{}, t3 ? t3->val : T{}));
        y->count = (t2 ? t2->count : 0) + (t3 ? t3->count : 0) + y->weight;

        x->val = T::calc(T::calcMany(x->val, x->weight), T::calc(t1 ? t1->val : T{}, y->val));
        x->count = (t1 ? t1->count : 0) + y->count + x->weight;

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

        x->val = T::calc(T::calcMany(x->val, x->weight), T::calc(t1 ? t1->val : T{}, t2 ? t2->val : T{}));
        x->count = (t1 ? t1->count : 0) + (t2 ? t2->count : 0) + x->weight;

        y->val = T::calc(T::calcMany(y->val, y->weight), T::calc(x->val, t3 ? t3->val : T{}));
        y->count = x->count + (t3 ? t3->count : 0) + y->weight;

        return y;
    }

    Node* insert(Node* node, const T& val, std::uint64_t count)
    {
        if (!node)
        {
            node = new Node(val, count);
        }
        else if (val.key < node->val.key)
        {
            node->left = this->insert(node->left, val, count);
            node->count += count;
            node->val = T::calc(node->val, T::calcMany(val, count));

            if (node->left->priority > node->priority)
            {
                node = this->rightRotate(node);
            }
        }
        else if (val.key == node->val.key)
        {
            node->count += count;
            node->weight += count;
            node->val = T::calc(node->val, T::calcMany(val, count));
        }
        else
        {
            node->right = this->insert(node->right, val, count);
            node->count += count;
            node->val = T::calc(node->val, T::calcMany(val, count));

            if (node->right->priority > node->priority)
            {
                node = this->leftRotate(node);
            }
        }

        return node;
    }

    std::uint64_t count(Node* node, const T& val) const
    {
        if (!node)
        {
            return 0;
        }
        if (node->val.key == val.key)
        {
            return node->weight;
        }
        if (node->val.key < val.key)
        {
            return this->count(node->right, val);
        }

        return this->count(node->left, val);
    }

    std::pair<Node*, std::uint64_t> erase(Node* node, const T& val, std::uint64_t count)
    {
        if (!node)
        {
            return std::pair<Node*, std::uint64_t>{ nullptr, 0 };
        }
        if (val.key < node->val.key)
        {
            std::pair<Node*, std::uint64_t> data = this->erase(node->left, val, count);
            node->left = data.first;
            node->count -= data.second;
            node->val = T::calc(T::calcMany(node->val, node->weight), T::calc(node->left ? node->left->val : T{}, node->right ? node->right->val : T{}));

            return std::pair<Node*, std::uint64_t>{ node, data.second };
        }
        if (val.key > node->val.key)
        {
            std::pair<Node*, std::uint64_t> data = this->erase(node->right, val, count);
            node->right = data.first;
            node->count -= data.second;
            node->val = T::calc(T::calcMany(node->val, node->weight), T::calc(node->left ? node->left->val : T{}, node->right ? node->right->val : T{}));

            return std::pair<Node*, std::uint64_t>{ node, data.second };
        }
        if (count < node->weight)
        {
            node->weight -= count;
            node->count -= count;
            node->val = T::calc(T::calcMany(node->val, node->weight), T::calc(node->left ? node->left->val : T{}, node->right ? node->right->val : T{}));

            return std::pair<Node*, std::uint64_t>{ node, count };
        }
        if (!node->left)
        {
            Node* temp = node->right;
            node->right = nullptr;
            std::uint64_t nodeWeight = node->weight;
            delete node;

            return std::pair<Node*, std::uint64_t>{ temp, nodeWeight };
        }
        if (!node->right)
        {
            Node* temp = node->left;
            node->left = nullptr;
            std::uint64_t nodeWeight = node->weight;
            delete node;

            return std::pair<Node*, std::uint64_t>{ temp, nodeWeight };
        }
        if (node->left->priority < node->right->priority)
        {
            node = this->leftRotate(node);

            std::pair<Node*, std::uint64_t> data = this->erase(node->left, val, count);
            node->left = data.first;
            node->count -= data.second;
            node->val = T::calc(T::calcMany(node->val, node->weight), T::calc(node->left ? node->left->val : T{}, node->right ? node->right->val : T{}));

            return std::pair<Node*, std::uint64_t>{ node, data.second };
        }

        node = this->rightRotate(node);

        std::pair<Node*, std::uint64_t> data = this->erase(node->right, val, count);
        node->right = data.first;
        node->count -= data.second;
        node->val = T::calc(T::calcMany(node->val, node->weight), T::calc(node->left ? node->left->val : T{}, node->right ? node->right->val : T{}));

        return std::pair<Node*, std::uint64_t>{ node, data.second };
    }

    void getRange(Node* node, std::uint64_t k, std::vector<T>& res, std::uint64_t l, std::uint64_t r) const
    {
        if (!node || k > r || k + node->count <= l)
        {
            return;
        }

        std::uint64_t currPos = k + (node->left ? node->left->count : 0);
        std::int64_t minus = 
            std::min((std::int64_t)currPos - (std::int64_t)l, (std::int64_t)0)
            + std::min((std::int64_t)r - (std::int64_t)(currPos + node->weight - 1), (std::int64_t)0);

        this->getRange(node->left, k, res, l, r);
        res.insert(res.end(), std::max((std::int64_t)node->weight + minus, (std::int64_t)0), node->val);
        this->getRange(node->right, currPos + node->weight, res, l, r);
    }

    Node* getKth(Node* node, std::uint64_t k) const
    {
        if (node->left && k < node->left->count)
        {
            return this->getKth(node->left, k);
        }

        if (node->right && k >= (node->left ? node->left->count : 0) + node->weight)
        {
            return this->getKth(node->right, k - (node->left ? node->left->count : 0) - node->weight);
        }

        return node;
    }

    void getRangeQuery(Node* node, std::uint64_t k, T& res, std::uint64_t l, std::uint64_t r) const
    {
        if (!node || k > r || k + node->count <= l)
        {
            return;
        }

        std::uint64_t currPos = k + (node->left ? node->left->count : 0);
        std::int64_t minus =
            std::min((std::int64_t)currPos - (std::int64_t)l, (std::int64_t)0)
            + std::min((std::int64_t)r - (std::int64_t)(currPos + node->weight - 1), (std::int64_t)0);

        if (l <= k && k + node->count - 1 <= r)
        {
            res = T::calc(res, node->val);
            return;
        }

        this->getRangeQuery(node->left, k, res, l, r);
        res = T::calc(res, T::calcMany(node->val, std::max((std::int64_t)node->weight + minus, (std::int64_t)0)));
        this->getRangeQuery(node->right, currPos + node->weight, res, l, r);
    }

    void getNearestSmaller(Node* node, const T& val, Node*& current) const
    {
        if (!node)
        {
            return;
        }

        if (!current || val.key < current->val.key || (node->val.key <= val.key && current->val.key < node->val.key))
        {
            current = node;
        }

        if (val.key < node->val.key)
        {
            this->getNearestSmaller(node->left, val, current);

            return;
        }

        while (current->right && current->right->val.key <= val.key)
        {
            current = current->right;
        }

        if (current->right)
        {
            this->getNearestSmaller(current->right->left, val, current);
        }
    }

    void getNearestBigger(Node* node, const T& val, Node*& current) const
    {
        if (!node)
        {
            return;
        }

        if (!current || val.key > current->val.key || (node->val.key >= val.key && current->val.key > node->val.key))
        {
            current = node;
        }

        if (val.key > node->val.key)
        {
            this->getNearestBigger(node->right, val, current);

            return;
        }

        while (current->left && current->left->val.key >= val.key)
        {
            current = current->left;
        }

        if (current->left)
        {
            this->getNearestBigger(current->left->right, val, current);
        }
    }

    Node* getSmallestK(Node* node, const T& val, std::uint64_t& counter) const
    {
        if (!node)
        {
            return nullptr;
        }

        if (node->val.key == val.key)
        {
            counter += (node->left ? node->left->count : 0);

            return node;
        }

        if (val.key < node->val.key)
        {
            return this->getSmallestK(node->left, val, counter);
        }

        counter += (node->left ? node->left->count : 0) + node->weight;

        return this->getSmallestK(node->right, val, counter);
    }

    Node* getBiggestK(Node* node, const T& val, std::uint64_t& counter) const
    {
        if (!node)
        {
            return nullptr;
        }

        if (node->val.key == val.key)
        {
            counter += (node->left ? node->left->count : 0) + node->weight - 1;

            return node;
        }

        if (val.key < node->val.key)
        {
            return this->getBiggestK(node->left, val, counter);
        }

        counter += (node->left ? node->left->count : 0) + node->weight;

        return this->getBiggestK(node->right, val, counter);
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

    static Key calc(const Key& firstKey, const Key& secondKey)
    {
        return Key(firstKey.key);
    }

    static Key calcMany(const Key& key, std::uint64_t count)
    {
        return Key(key.key);
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

    static Min calc(const Min& firstMin, const Min& secondMin)
    {
        return Min(firstMin.key, std::min(firstMin.min, secondMin.min));
    }

    static Min calcMany(const Min& min, std::uint64_t count)
    {
        return count ? Min(min.key) : Min{};
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

    static Max calc(const Max& firstMax, const Max& secondMax)
    {
        return Max(firstMax.key, std::max(firstMax.max, secondMax.max));
    }

    static Max calcMany(const Max& max, std::uint64_t count)
    {
        return count ? Max(max.key) : Max{};
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

    static Sum calc(const Sum& firstSum, const Sum& secondSum)
    {
        return Sum(firstSum.key, firstSum.sum + secondSum.sum);
    }

    static Sum calcMany(const Sum& sum, std::uint64_t count)
    {
        return Sum(sum.key, sum.key * (std::int64_t)count);
    }
};