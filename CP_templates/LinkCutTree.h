#include <cstdint>
#include <vector>
#include <algorithm>
#include <limits>
#include <type_traits>

enum class StoreType
{
	NO_DATA,
	PATH_DATA,
	ALL_DATA
};

struct Empty
{
	Empty() = delete;
};

template <typename T, StoreType storeType = StoreType::ALL_DATA, bool preserveRoot = true>
class LinkCutTree
{
public:
	LinkCutTree(std::uint64_t size) :
		m_nodes(size)
	{
	}

	std::uint64_t size() const
	{
		return m_nodes.size();
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

		m_nodes[a].child[1] = b;
		m_nodes[b].parent = a;
		if constexpr (storeType >= StoreType::PATH_DATA)
		{
			recalc(a);
		}
	}

	void cut(std::uint64_t a, std::uint64_t b)
	{
		if constexpr (preserveRoot)
		{
			std::uint64_t prevRoot = getRoot(a);

			reroot(a);
			expose(b);

			m_nodes[a].parent = std::numeric_limits<std::uint64_t>::max();
			m_nodes[b].child[0] = std::numeric_limits<std::uint64_t>::max();
			if constexpr (storeType >= StoreType::PATH_DATA)
			{
				recalc(b);
			}

			reroot(prevRoot);
		}
		else
		{
			reroot(a);
			expose(b);

			m_nodes[a].parent = std::numeric_limits<std::uint64_t>::max();
			m_nodes[b].child[0] = std::numeric_limits<std::uint64_t>::max();
			if constexpr (storeType >= StoreType::PATH_DATA)
			{
				recalc(b);
			}
		}
	}

	std::uint64_t getLCA(std::uint64_t a, std::uint64_t b)
	{
		expose(a);

		return expose(b);
	}

	std::uint64_t getRoot(std::uint64_t a)
	{
		expose(a);

		while (m_nodes[a].child[0] != std::numeric_limits<std::uint64_t>::max())
		{
			a = m_nodes[a].child[0];
			propagate(a);
		}

		splay(a);

		return a;
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::PATH_DATA, std::uint64_t> getNthParent(std::uint64_t a, std::uint64_t n)
	{
		expose(a);

		a = m_nodes[a].child[0];
		propagate(a);

		while (true)
		{
			if (n > (m_nodes[a].child[1] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[a].child[1]].size : 0))
			{
				n -= (m_nodes[a].child[1] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[a].child[1]].size : 0) + 1;
				a = m_nodes[a].child[0];
				propagate(a);
			}
			else if (n < (m_nodes[a].child[1] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[a].child[1]].size : 0))
			{
				a = m_nodes[a].child[1];
				propagate(a);
			}
			else
			{
				splay(a);
				return a;
			}
		}
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::PATH_DATA, std::uint64_t> getPathSize(std::uint64_t a, std::uint64_t b)
	{
		if constexpr (preserveRoot)
		{
			std::uint64_t prevRoot = getRoot(a);

			reroot(a);
			expose(b);

			std::uint64_t result = m_nodes[b].size;

			reroot(prevRoot);

			return result;
		}

		reroot(a);
		expose(b);

		return m_nodes[b].size;
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::PATH_DATA && !std::is_same_v<T, Empty>, T> queryPath(std::uint64_t a, std::uint64_t b)
	{
		if constexpr (preserveRoot)
		{
			std::uint64_t prevRoot = getRoot(a);

			reroot(a);
			expose(b);

			T result = m_nodes[b].val;

			reroot(prevRoot);

			return result;
		}

		reroot(a);
		expose(b);

		return m_nodes[b].val;
		
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::PATH_DATA && !std::is_same_v<T, Empty>, void> updatePathReplace(std::uint64_t a, std::uint64_t b, const T& newVal)
	{
		if constexpr (preserveRoot)
		{
			std::uint64_t prevRoot = getRoot(a);

			reroot(a);
			expose(b);

			m_nodes[b].lazyType ^= s_lazyUpdateReplaceBit;
			m_nodes[b].lazyVal = newVal;

			reroot(prevRoot);
		}
		else
		{
			reroot(a);
			expose(b);

			m_nodes[b].lazyType ^= s_lazyUpdateReplaceBit;
			m_nodes[b].lazyVal = newVal;
		}
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::PATH_DATA && !std::is_same_v<T, Empty>, void> updatePathBy(std::uint64_t a, std::uint64_t b, const T& updateVal)
	{
		if constexpr (preserveRoot)
		{
			std::uint64_t prevRoot = getRoot(a);

			reroot(a);
			expose(b);

			m_nodes[b].lazyType ^= s_lazyUpdateByBit;
			m_nodes[b].lazyVal = updateVal;

			reroot(prevRoot);
		}
		else
		{
			reroot(a);
			expose(b);

			m_nodes[b].lazyType ^= s_lazyUpdateByBit;
			m_nodes[b].lazyVal = updateVal;
		}
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::ALL_DATA, std::uint64_t> getSubtreeSize(std::uint64_t a)
	{
		expose(a);

		return m_nodes[a].virtualSubtreeSize + 1;
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::ALL_DATA && !std::is_same_v<T, Empty>, T> querySubtree(std::uint64_t a)
	{
		expose(a);

		return T::calcLeft(T::getPure(m_nodes[a].val), m_nodes[a].virtualSubtreeVal);
	}

private:
	template <StoreType nodeStoreType, bool isEmptyNode, typename Dummy = void>
	struct NodeTemplate
	{
	};

	template <bool isEmptyNode, typename Dummy>
	struct NodeTemplate<StoreType::NO_DATA, isEmptyNode, Dummy>
	{
		std::uint64_t parent = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t child[2]{
			std::numeric_limits<std::uint64_t>::max(),
			std::numeric_limits<std::uint64_t>::max()
		};
		std::uint8_t lazyType = 0;
	};

	template <typename Dummy>
	struct NodeTemplate<StoreType::PATH_DATA, false, Dummy>
	{
		std::uint64_t parent = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t child[2]{
			std::numeric_limits<std::uint64_t>::max(),
			std::numeric_limits<std::uint64_t>::max()
		};
		std::uint8_t lazyType = 0;

		std::uint64_t size = 1;
		T val{};
		T lazyVal{};
	};

	template <typename Dummy>
	struct NodeTemplate<StoreType::PATH_DATA, true, Dummy>
	{
		std::uint64_t parent = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t child[2]{
			std::numeric_limits<std::uint64_t>::max(),
			std::numeric_limits<std::uint64_t>::max()
		};
		std::uint8_t lazyType = 0;

		std::uint64_t size = 1;
	};

	template <typename Dummy>
	struct NodeTemplate<StoreType::ALL_DATA, false, Dummy>
	{
		std::uint64_t parent = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t child[2]{
			std::numeric_limits<std::uint64_t>::max(),
			std::numeric_limits<std::uint64_t>::max()
		};
		std::uint8_t lazyType = 0;

		std::uint64_t size = 1;
		T val{};
		T lazyVal{};

		std::uint64_t subtreeSize = 1;
		std::uint64_t virtualSubtreeSize = 0;
		T subtreeVal{};
		T virtualSubtreeVal{};
	};

	template <typename Dummy>
	struct NodeTemplate<StoreType::ALL_DATA, true, Dummy>
	{
		std::uint64_t parent = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t child[2]{
			std::numeric_limits<std::uint64_t>::max(),
			std::numeric_limits<std::uint64_t>::max()
		};
		std::uint8_t lazyType = 0;

		std::uint64_t size = 1;

		std::uint64_t subtreeSize = 1;
		std::uint64_t virtualSubtreeSize = 0;
	};

	using Node = NodeTemplate<storeType, std::is_same_v<T, Empty>>;

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::PATH_DATA && !std::is_same_v<T, Empty>, void> updateValReplace(std::uint64_t node, const T& newVal)
	{
		if constexpr (storeType >= StoreType::ALL_DATA)
		{
			m_nodes[node].subtreeVal = T::uncalc(m_nodes[node].subtreeVal, m_nodes[node].val);
		}
		m_nodes[node].val = newVal;
		if constexpr (storeType >= StoreType::ALL_DATA)
		{
			m_nodes[node].subtreeVal = T::calcLeft(m_nodes[node].subtreeVal, m_nodes[node].val);
		}
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::PATH_DATA && !std::is_same_v<T, Empty>, void> updateValBy(std::uint64_t node, const T& updateVal)
	{
		m_nodes[node].val = T::calcLazy(m_nodes[node].val, updateVal);
		if constexpr (storeType >= StoreType::ALL_DATA)
		{
			m_nodes[node].subtreeVal = T::calcLeft(m_nodes[node].subtreeVal, updateVal);
		}
	}

	void propagate(std::uint64_t node)
	{
		if (m_nodes[node].lazyType & s_lazyReverseBit)
		{
			if (m_nodes[node].child[0] != std::numeric_limits<std::uint64_t>::max())
			{
				m_nodes[m_nodes[node].child[0]].lazyType ^= s_lazyReverseBit;
			}
			if (m_nodes[node].child[1] != std::numeric_limits<std::uint64_t>::max())
			{
				m_nodes[m_nodes[node].child[1]].lazyType ^= s_lazyReverseBit;
			}
			std::swap(m_nodes[node].child[0], m_nodes[node].child[1]);
			if constexpr (storeType >= StoreType::PATH_DATA && !std::is_same_v<T, Empty>)
			{
				updateValReplace(node, T::reverse(m_nodes[node].val));
			}
			m_nodes[node].lazyType ^= s_lazyReverseBit;
		}

		if constexpr (storeType >= StoreType::PATH_DATA && !std::is_same_v<T, Empty>)
		{
			if (m_nodes[node].lazyType & s_lazyUpdateReplaceBit)
			{
				if (m_nodes[node].child[0] != std::numeric_limits<std::uint64_t>::max())
				{
					m_nodes[m_nodes[node].child[0]].lazyType &= s_lazyReverseBit;
					m_nodes[m_nodes[node].child[0]].lazyType ^= s_lazyUpdateReplaceBit;
					m_nodes[m_nodes[node].child[0]].lazyVal = m_nodes[node].lazyVal;
				}
				if (m_nodes[node].child[1] != std::numeric_limits<std::uint64_t>::max())
				{
					m_nodes[m_nodes[node].child[1]].lazyType &= s_lazyReverseBit;
					m_nodes[m_nodes[node].child[1]].lazyType ^= s_lazyUpdateReplaceBit;
					m_nodes[m_nodes[node].child[1]].lazyVal = m_nodes[node].lazyVal;
				}
				updateValReplace(node, T::calcMany(m_nodes[node].lazyVal, m_nodes[node].size));
				m_nodes[node].lazyType ^= s_lazyUpdateReplaceBit;
				m_nodes[node].lazyVal = T{};
			}
			else if (m_nodes[node].lazyType & s_lazyUpdateByBit)
			{
				if (m_nodes[node].child[0] != std::numeric_limits<std::uint64_t>::max())
				{
					if (m_nodes[m_nodes[node].child[0]].lazyType ^ s_lazyUpdateReplaceBit)
					{
						m_nodes[m_nodes[node].child[0]].lazyType |= s_lazyUpdateByBit;
					}
					m_nodes[m_nodes[node].child[0]].lazyVal = T::calcLazy(m_nodes[m_nodes[node].child[0]].lazyVal, m_nodes[node].lazyVal);
				}
				if (m_nodes[node].child[1] != std::numeric_limits<std::uint64_t>::max())
				{
					if (m_nodes[m_nodes[node].child[1]].lazyType ^ s_lazyUpdateReplaceBit)
					{
						m_nodes[m_nodes[node].child[1]].lazyType |= s_lazyUpdateByBit;
					}
					m_nodes[m_nodes[node].child[1]].lazyVal = T::calcLazy(m_nodes[m_nodes[node].child[1]].lazyVal, m_nodes[node].lazyVal);
				}
				updateValBy(node, T::calcMany(m_nodes[node].lazyVal, m_nodes[node].size));
				m_nodes[node].lazyType ^= s_lazyUpdateByBit;
				m_nodes[node].lazyVal = T{};
			}
		}
	}

	std::uint64_t expose(std::uint64_t node)
	{
		std::uint64_t prev = std::numeric_limits<std::uint64_t>::max();

		for (std::uint64_t tmp = node; tmp != std::numeric_limits<std::uint64_t>::max(); tmp = m_nodes[tmp].parent)
		{
			splay(tmp);
			if constexpr (storeType >= StoreType::ALL_DATA)
			{
				if (m_nodes[tmp].child[1] != std::numeric_limits<std::uint64_t>::max())
				{
					if constexpr (!std::is_same_v<T, Empty>)
					{
						propagate(m_nodes[tmp].child[1]);
						recalc(m_nodes[tmp].child[1]);
					}
					m_nodes[tmp].virtualSubtreeSize += m_nodes[m_nodes[tmp].child[1]].subtreeSize;
					if constexpr (!std::is_same_v<T, Empty>)
					{
						m_nodes[tmp].virtualSubtreeVal = T::calcLeft(m_nodes[tmp].virtualSubtreeVal, m_nodes[m_nodes[tmp].child[1]].subtreeVal);
					}
				}
			}
			m_nodes[tmp].child[1] = prev;
			if constexpr (storeType >= StoreType::ALL_DATA)
			{
				if (prev != std::numeric_limits<std::uint64_t>::max())
				{
					m_nodes[tmp].virtualSubtreeSize -= m_nodes[prev].subtreeSize;
					if constexpr (!std::is_same_v<T, Empty>)
					{
						m_nodes[tmp].virtualSubtreeVal = T::uncalc(m_nodes[tmp].virtualSubtreeVal, m_nodes[prev].subtreeVal);
					}
				}
			}
			if constexpr (storeType >= StoreType::PATH_DATA)
			{
				recalc(tmp);
			}
			prev = tmp;
		};

		splay(node);

		return prev;
	}

	template <StoreType methodStoreType = storeType>
	std::enable_if_t<methodStoreType >= StoreType::PATH_DATA, void> recalc(std::uint64_t node)
	{
		if constexpr (!std::is_same_v<T, Empty>)
		{
			if (m_nodes[node].child[0] != std::numeric_limits<std::uint64_t>::max())
			{
				propagate(m_nodes[node].child[0]);
			}
			if (m_nodes[node].child[1] != std::numeric_limits<std::uint64_t>::max())
			{
				propagate(m_nodes[node].child[1]);
			}
		}
		m_nodes[node].size =
			(m_nodes[node].child[0] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].child[0]].size : 0)
			+ 1
			+ (m_nodes[node].child[1] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].child[1]].size : 0);
		if constexpr (!std::is_same_v<T, Empty>)
		{
			m_nodes[node].val = T::calcLeft(
				T::calcRight(
					m_nodes[node].child[0] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].child[0]].val : T{},
					T::getPure(m_nodes[node].val)
				),
				m_nodes[node].child[1] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].child[1]].val : T{}
			);
		}
		if constexpr (storeType >= StoreType::ALL_DATA)
		{
			m_nodes[node].subtreeSize =
				(m_nodes[node].child[0] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].child[0]].subtreeSize : 0)
				+ 1
				+ (m_nodes[node].child[1] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].child[1]].subtreeSize : 0)
				+ m_nodes[node].virtualSubtreeSize;
			if constexpr (!std::is_same_v<T, Empty>)
			{
				m_nodes[node].subtreeVal = T::calcLeft(
					T::calcLeft(
						T::calcLeft(
							T::getPure(m_nodes[node].val),
							m_nodes[node].child[0] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].child[0]].subtreeVal : T{}
						),
						m_nodes[node].child[1] != std::numeric_limits<std::uint64_t>::max() ? m_nodes[m_nodes[node].child[1]].subtreeVal : T{}
					),
					m_nodes[node].virtualSubtreeVal
				);
			}
		}
	}

	void splay(std::uint64_t node)
	{
		if (isSplayRoot(node))
		{
			propagate(node);
			return;
		}

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
				if (node == m_nodes[m_nodes[node].parent].child[0])
				{
					rotate<false>(m_nodes[node].parent);
				}
				else
				{
					rotate<true>(m_nodes[node].parent);
				}
			}
			else if (m_nodes[node].parent == m_nodes[m_nodes[m_nodes[node].parent].parent].child[0])
			{
				if (node == m_nodes[m_nodes[node].parent].child[0])
				{
					rotate<false>(m_nodes[m_nodes[node].parent].parent);
				}
				else
				{
					rotate<true>(m_nodes[node].parent);
				}
				rotate<false>(m_nodes[node].parent);
			}
			else
			{
				if (node == m_nodes[m_nodes[node].parent].child[1])
				{
					rotate<true>(m_nodes[m_nodes[node].parent].parent);
				}
				else
				{
					rotate<false>(m_nodes[node].parent);
				}
				rotate<true>(m_nodes[node].parent);
			}
		}
	}

	template <bool isRotateLeft>
	void rotate(std::uint64_t node)
	{
		std::uint64_t newParent = m_nodes[node].child[isRotateLeft];
		if (m_nodes[node].parent != std::numeric_limits<std::uint64_t>::max())
		{
			if (node == m_nodes[m_nodes[node].parent].child[0])
			{
				m_nodes[m_nodes[node].parent].child[0] = newParent;
			}
			else if (node == m_nodes[m_nodes[node].parent].child[1])
			{
				m_nodes[m_nodes[node].parent].child[1] = newParent;
			}
		}
		m_nodes[newParent].parent = m_nodes[node].parent;

		m_nodes[node].parent = newParent;
		m_nodes[node].child[isRotateLeft] = m_nodes[newParent].child[!isRotateLeft];
		if (m_nodes[node].child[isRotateLeft] != std::numeric_limits<std::uint64_t>::max())
		{
			m_nodes[m_nodes[node].child[isRotateLeft]].parent = node;
		}
		m_nodes[newParent].child[!isRotateLeft] = node;

		if constexpr (storeType >= StoreType::PATH_DATA)
		{
			recalc(node);
			recalc(newParent);
		}
	}

	bool isSplayRoot(std::uint64_t node) const
	{
		return m_nodes[node].parent == std::numeric_limits<std::uint64_t>::max()
			|| (m_nodes[m_nodes[node].parent].child[0] != node && m_nodes[m_nodes[node].parent].child[1] != node);
	}

	std::vector<Node> m_nodes;

	constexpr static std::uint8_t s_lazyReverseBit = 1;
	constexpr static std::uint8_t s_lazyUpdateReplaceBit = 2;
	constexpr static std::uint8_t s_lazyUpdateByBit = 4;
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