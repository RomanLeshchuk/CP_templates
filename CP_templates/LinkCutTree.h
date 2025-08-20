#include <cstdint>
#include <vector>
#include <algorithm>
#include <limits>

template <typename T>
class LinkCutTree
{
public:
	LinkCutTree(std::uint64_t size) :
		m_nodes(size)
	{
	}

	void reroot(std::uint64_t a)
	{
		expose(a);
		m_nodes[a].lazyType ^= s_lazyReverseBit;
	}

	void link(std::uint64_t a, std::uint64_t b)
	{
		reroot(b);
		expose(a);

		m_nodes[a].right = b;
		m_nodes[b].parent = a;
		recalc(a);
	}

	void cut(std::uint64_t a, std::uint64_t b)
	{
		std::uint64_t prevRoot = getRoot(a);

		reroot(a);
		expose(b);

		m_nodes[a].parent = std::numeric_limits<std::uint64_t>::max();
		m_nodes[b].left = std::numeric_limits<std::uint64_t>::max();
		recalc(b);

		reroot(prevRoot);
	}

	std::uint64_t getLCA(std::uint64_t a, std::uint64_t b)
	{
		expose(a);

		return expose(b);
	}

	std::uint64_t getRoot(std::uint64_t a)
	{
		expose(a);

		while (m_nodes[a].left != std::numeric_limits<std::uint64_t>::max())
		{
			a = m_nodes[a].left;
			propagate(a);
		}

		splay(a);

		return a;
	}

	std::uint64_t getNthParent(std::uint64_t a, std::uint64_t n)
	{
		expose(a);

		a = m_nodes[a].left;
		propagate(a);

		while (true)
		{
			if (n > (m_nodes[a].right != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[a].right].size : 0))
			{
				n -= (m_nodes[a].right != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[a].right].size : 0) + 1;
				a = m_nodes[a].left;
				propagate(a);
			}
			else if (n < (m_nodes[a].right != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[a].right].size : 0))
			{
				a = m_nodes[a].right;
				propagate(a);
			}
			else
			{
				splay(a);
				return a;
			}
		}
	}

	std::uint64_t getPathSize(std::uint64_t a, std::uint64_t b)
	{
		std::uint64_t prevRoot = getRoot(a);
		reroot(a);

		expose(b);
		std::uint64_t result = m_nodes[b].size;

		reroot(prevRoot);

		return result;
	}

	T queryPath(std::uint64_t a, std::uint64_t b)
	{
		std::uint64_t prevRoot = getRoot(a);
		reroot(a);

		expose(b);
		T result = m_nodes[b].val;

		reroot(prevRoot);

		return result;
	}

	void updatePathReplace(std::uint64_t a, std::uint64_t b, const T& newVal)
	{
		std::uint64_t prevRoot = getRoot(a);
		reroot(a);

		expose(b);
		m_nodes[b].lazyType ^= s_lazyUpdateReplaceBit;
		m_nodes[b].lazyVal = newVal;

		reroot(prevRoot);
	}

	void updatePathBy(std::uint64_t a, std::uint64_t b, const T& updateVal)
	{
		std::uint64_t prevRoot = getRoot(a);
		reroot(a);

		expose(b);
		m_nodes[b].lazyType ^= s_lazyUpdateByBit;
		m_nodes[b].lazyVal = updateVal;

		reroot(prevRoot);
	}

	std::uint64_t size() const
	{
		return m_nodes.size();
	}

	std::uint64_t getSubtreeSize(std::uint64_t a)
	{
		expose(a);

		return m_nodes[a].virtualSubtreeSize + 1;
	}

	T querySubtree(std::uint64_t a)
	{
		expose(a);

		return T::calcLeft(T::getPure(m_nodes[a].val), m_nodes[a].virtualSubtreeVal);
	}

private:
	struct Node
	{
		std::uint64_t parent = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t left = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t right = std::numeric_limits<std::uint64_t>::max();

		T val{};

		std::uint64_t size = 1;
		std::uint64_t subtreeSize = 1;
		std::uint64_t virtualSubtreeSize = 0;
		T subtreeVal{};
		T virtualSubtreeVal{};

		std::uint8_t lazyType = 0;
		T lazyVal{};
	};

	void updateVal(std::uint64_t node, const T& newVal)
	{
		m_nodes[node].subtreeVal = T::uncalc(m_nodes[node].subtreeVal, m_nodes[node].val);
		m_nodes[node].val = newVal;
		m_nodes[node].subtreeVal = T::calcLeft(m_nodes[node].subtreeVal, m_nodes[node].val);
	}

	void propagate(std::uint64_t node)
	{
		if (m_nodes[node].lazyType & s_lazyReverseBit)
		{
			if (m_nodes[node].left != std::numeric_limits<std::uint64_t>::max())
			{
				m_nodes[m_nodes[node].left].lazyType ^= s_lazyReverseBit;
			}
			if (m_nodes[node].right != std::numeric_limits<std::uint64_t>::max())
			{
				m_nodes[m_nodes[node].right].lazyType ^= s_lazyReverseBit;
			}
			std::swap(m_nodes[node].left, m_nodes[node].right);
			updateVal(node, T::reverse(m_nodes[node].val));
			m_nodes[node].lazyType ^= s_lazyReverseBit;
		}

		if (m_nodes[node].lazyType & s_lazyUpdateReplaceBit)
		{
			if (m_nodes[node].left != std::numeric_limits<std::uint64_t>::max())
			{
				m_nodes[m_nodes[node].left].lazyType &= s_lazyReverseBit;
				m_nodes[m_nodes[node].left].lazyType ^= s_lazyUpdateReplaceBit;
				m_nodes[m_nodes[node].left].lazyVal = m_nodes[node].lazyVal;
			}
			if (m_nodes[node].right != std::numeric_limits<std::uint64_t>::max())
			{
				m_nodes[m_nodes[node].right].lazyType &= s_lazyReverseBit;
				m_nodes[m_nodes[node].right].lazyType ^= s_lazyUpdateReplaceBit;
				m_nodes[m_nodes[node].right].lazyVal = m_nodes[node].lazyVal;
			}
			updateVal(node, T::calcMany(m_nodes[node].lazyVal, m_nodes[node].size));
			m_nodes[node].lazyType ^= s_lazyUpdateReplaceBit;
			m_nodes[node].lazyVal = T{};
		}
		else if (m_nodes[node].lazyType & s_lazyUpdateByBit)
		{
			if (m_nodes[node].left != std::numeric_limits<std::uint64_t>::max())
			{
				if (m_nodes[m_nodes[node].left].lazyType ^ s_lazyUpdateReplaceBit)
				{
					m_nodes[m_nodes[node].left].lazyType |= s_lazyUpdateByBit;
				}
				m_nodes[m_nodes[node].left].lazyVal = T::calcLazy(m_nodes[m_nodes[node].left].lazyVal, m_nodes[node].lazyVal);
			}
			if (m_nodes[node].right != std::numeric_limits<std::uint64_t>::max())
			{
				if (m_nodes[m_nodes[node].right].lazyType ^ s_lazyUpdateReplaceBit)
				{
					m_nodes[m_nodes[node].right].lazyType |= s_lazyUpdateByBit;
				}
				m_nodes[m_nodes[node].right].lazyVal = T::calcLazy(m_nodes[m_nodes[node].right].lazyVal, m_nodes[node].lazyVal);
			}
			updateVal(node, T::calcLazy(m_nodes[node].val, T::calcMany(m_nodes[node].lazyVal, m_nodes[node].size)));
			m_nodes[node].lazyType ^= s_lazyUpdateByBit;
			m_nodes[node].lazyVal = T{};
		}
	}

	std::uint64_t expose(std::uint64_t node)
	{
		std::uint64_t prev = std::numeric_limits<std::uint64_t>::max();

		for (std::uint64_t tmp = node; tmp != std::numeric_limits<std::uint64_t>::max(); tmp = m_nodes[tmp].parent)
		{
			splay(tmp);
			if (m_nodes[tmp].right != std::numeric_limits<std::uint64_t>::max())
			{
				propagate(m_nodes[tmp].right);
				recalc(m_nodes[tmp].right);
				m_nodes[tmp].virtualSubtreeSize += m_nodes[m_nodes[tmp].right].subtreeSize;
				m_nodes[tmp].virtualSubtreeVal = T::calcLeft(m_nodes[tmp].virtualSubtreeVal, m_nodes[m_nodes[tmp].right].subtreeVal);
			}
			m_nodes[tmp].right = prev;
			if (prev != std::numeric_limits<std::uint64_t>::max())
			{
				m_nodes[tmp].virtualSubtreeSize -= m_nodes[prev].subtreeSize;
				m_nodes[tmp].virtualSubtreeVal = T::uncalc(m_nodes[tmp].virtualSubtreeVal, m_nodes[prev].subtreeVal);
			}
			recalc(tmp);
			prev = tmp;
		};

		splay(node);

		return prev;
	}

	void recalc(std::uint64_t node)
	{
		if (m_nodes[node].left != std::numeric_limits<std::uint64_t>::max())
		{
			propagate(m_nodes[node].left);
		}
		if (m_nodes[node].right != std::numeric_limits<std::uint64_t>::max())
		{
			propagate(m_nodes[node].right);
		}
		m_nodes[node].size =
			(m_nodes[node].left != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].left].size : 0)
			+ 1
			+ (m_nodes[node].right != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].right].size : 0);
		m_nodes[node].subtreeSize =
			(m_nodes[node].left != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].left].subtreeSize : 0)
			+ 1
			+ (m_nodes[node].right != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].right].subtreeSize : 0)
			+ m_nodes[node].virtualSubtreeSize;
		m_nodes[node].val = T::calcLeft(
			T::calcRight(
				m_nodes[node].left != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].left].val : T{},
				T::getPure(m_nodes[node].val)
			),
			m_nodes[node].right != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].right].val : T{}
		);
		m_nodes[node].subtreeVal = T::calcLeft(
			T::calcLeft(
				T::calcLeft(
					T::getPure(m_nodes[node].val),
					m_nodes[node].left != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].left].subtreeVal : T{}
				),
				m_nodes[node].right != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].right].subtreeVal : T{}
			),
			m_nodes[node].virtualSubtreeVal
		);
	}

	void splay(std::uint64_t node)
	{
		while (!isSplayRoot(node))
		{
			if (!isSplayRoot(m_nodes[node].parent))
			{
				propagate(m_nodes[m_nodes[node].parent].parent);
			}
			propagate(m_nodes[node].parent);
			propagate(node);

			if (isSplayRoot(m_nodes[node].parent))
			{
				if (node == m_nodes[m_nodes[node].parent].left)
				{
					rotateRight(m_nodes[node].parent);
				}
				else
				{
					rotateLeft(m_nodes[node].parent);
				}
			}
			else if (m_nodes[node].parent == m_nodes[m_nodes[m_nodes[node].parent].parent].left)
			{
				if (node == m_nodes[m_nodes[node].parent].left)
				{
					rotateRight(m_nodes[m_nodes[node].parent].parent);
				}
				else
				{
					rotateLeft(m_nodes[node].parent);
				}
				rotateRight(m_nodes[node].parent);
			}
			else
			{
				if (node == m_nodes[m_nodes[node].parent].right)
				{
					rotateLeft(m_nodes[m_nodes[node].parent].parent);
				}
				else
				{
					rotateRight(m_nodes[node].parent);
				}
				rotateLeft(m_nodes[node].parent);
			}
		}

		propagate(node);
	}

	void rotateLeft(std::uint64_t node)
	{
		std::uint64_t newParent = m_nodes[node].right;
		if (m_nodes[node].parent != std::numeric_limits<std::uint64_t>::max())
		{
			if (node == m_nodes[m_nodes[node].parent].left)
			{
				m_nodes[m_nodes[node].parent].left = newParent;
			}
			else if (node == m_nodes[m_nodes[node].parent].right)
			{
				m_nodes[m_nodes[node].parent].right = newParent;
			}
		}
		m_nodes[newParent].parent = m_nodes[node].parent;

		m_nodes[node].parent = newParent;
		m_nodes[node].right = m_nodes[newParent].left;
		if (m_nodes[node].right != std::numeric_limits<std::uint64_t>::max())
		{
			m_nodes[m_nodes[node].right].parent = node;
		}
		m_nodes[newParent].left = node;

		recalc(node);
		recalc(newParent);
	}

	void rotateRight(std::uint64_t node)
	{
		std::uint64_t newParent = m_nodes[node].left;
		if (m_nodes[node].parent != std::numeric_limits<std::uint64_t>::max())
		{
			if (node == m_nodes[m_nodes[node].parent].left)
			{
				m_nodes[m_nodes[node].parent].left = newParent;
			}
			else if (node == m_nodes[m_nodes[node].parent].right)
			{
				m_nodes[m_nodes[node].parent].right = newParent;
			}
		}
		m_nodes[newParent].parent = m_nodes[node].parent;

		m_nodes[node].parent = newParent;
		m_nodes[node].left = m_nodes[newParent].right;
		if (m_nodes[node].left != std::numeric_limits<std::uint64_t>::max())
		{
			m_nodes[m_nodes[node].left].parent = node;
		}
		m_nodes[newParent].right = node;

		recalc(node);
		recalc(newParent);
	}

	bool isSplayRoot(std::uint64_t node) const
	{
		return m_nodes[node].parent == std::numeric_limits<std::uint64_t>::max()
			|| (m_nodes[m_nodes[node].parent].left != node && m_nodes[m_nodes[node].parent].right != node);
	}

	std::vector<Node> m_nodes;

	constexpr static std::uint8_t s_lazyReverseBit = 1;
	constexpr static std::uint8_t s_lazyUpdateReplaceBit = 2;
	constexpr static std::uint8_t s_lazyUpdateByBit = 4;
};

struct Empty
{
	Empty() = default;

	static Empty getPure(const Empty& empty)
	{
		return Empty{};
	}

	static Empty calcLeft(const Empty& firstEmpty, const Empty& secondEmpty)
	{
		return Empty{};
	}

	static Empty calcRight(const Empty& firstEmpty, const Empty& secondEmpty)
	{
		return Empty{};
	}

	static Empty calcLazy(const Empty& empty, const Empty& lazy)
	{
		return Empty{};
	}

	static Empty calcMany(const Empty& empty, std::uint64_t count)
	{
		return Empty{};
	}

	static Empty reverse(const Empty& empty)
	{
		return Empty{};
	}

	static Empty uncalc(const Empty& empty, const Empty& uncalcEmpty)
	{
		return Empty{};
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

	static Min getPure(const Min& min)
	{
		return Min(min.key);
	}

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

	static Min reverse(const Min& min)
	{
		return min;
	}

	static Min uncalc(const Min& min, const Min& uncalcMin)
	{
		// irrevertible operations will not work with subtree queries, so just ignore
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

	static Max getPure(const Max& max)
	{
		return Max(max.key);
	}

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

	static Max reverse(const Max& max)
	{
		return max;
	}

	static Max uncalc(const Max& max, const Max& uncalcMax)
	{
		// irrevertible operations will not work with subtree queries, so just ignore
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

	static Sum getPure(const Sum& sum)
	{
		return Sum(sum.key);
	}

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
		return Sum(sum.key, sum.key * static_cast<std::int64_t>(count));
	}

	static Sum reverse(const Sum& sum)
	{
		return sum;
	}

	static Sum uncalc(const Sum& sum, const Sum& uncalcSum)
	{
		return Sum(sum.key, sum.sum - uncalcSum.sum);
	}
};