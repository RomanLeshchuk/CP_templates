#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>

class MaxFlowMinCost
{
public:
	MaxFlowMinCost(std::uint64_t n, std::uint64_t source, std::uint64_t sink) :
		m_graph(n),
		m_source{ source },
		m_sink{ sink }
	{
	}

	MaxFlowMinCost(const MaxFlowMinCost& maxFlowMinCost) = default;

	void addEdgeDirected(std::uint64_t from, std::uint64_t to, std::uint64_t capacity, std::int64_t cost)
	{
		if (from == to)
		{
			return;
		}

		m_graph[from].push_back({ to, capacity, m_graph[to].size(), cost });
		m_graph[to].push_back({ from, 0, m_graph[from].size() - 1, -cost });
	}

	void addEdgeUndirected(std::uint64_t a, std::uint64_t b, std::uint64_t capacity, std::int64_t cost)
	{
		if (a == b)
		{
			return;
		}

		addEdgeDirected(a, b, capacity, cost);
		addEdgeDirected(b, a, capacity, cost);
	}

	void passFlow(std::uint64_t flow)
	{
		std::vector<std::int64_t> potential(m_graph.size(), std::numeric_limits<std::int64_t>::max());
		potential[m_source] = 0;

		for (std::uint64_t i = 0; i < m_graph.size() - 1; ++i)
		{
			for (std::uint64_t j = 0; j < m_graph.size(); ++j)
			{
				if (potential[j] == std::numeric_limits<std::int64_t>::max())
				{
					continue;
				}
				for (const Edge& edgeToNxt : m_graph[j])
				{
					if (edgeToNxt.flow)
					{
						potential[edgeToNxt.to] = std::min(potential[edgeToNxt.to], potential[j] + edgeToNxt.cost);
					}
				}
			}
		}

		std::int64_t sinkCumulativePotential = 0;

		while (potential[m_sink] != std::numeric_limits<std::int64_t>::max())
		{
			sinkCumulativePotential += potential[m_sink];

			for (std::uint64_t i = 0; i < m_graph.size(); ++i)
			{
				for (Edge& edgeToNxt : m_graph[i])
				{
					if (potential[i] != std::numeric_limits<std::int64_t>::max()
						&& potential[edgeToNxt.to] != std::numeric_limits<std::int64_t>::max())
					{
						edgeToNxt.cost += potential[i] - potential[edgeToNxt.to];
					}
				}
			}

			while (true)
			{
				std::vector<std::uint64_t> dist(m_graph.size(), std::numeric_limits<std::uint64_t>::max());
				std::queue<std::uint64_t> bfs{};
				dist[m_source] = 0;
				bfs.push(m_source);
				while (!bfs.empty())
				{
					std::uint64_t v = bfs.front();
					bfs.pop();
					for (const Edge& edgeToNxt : m_graph[v])
					{
						if (edgeToNxt.flow && !edgeToNxt.cost && dist[edgeToNxt.to] == std::numeric_limits<std::uint64_t>::max())
						{
							dist[edgeToNxt.to] = dist[v] + 1;
							bfs.push(edgeToNxt.to);
						}
					}
				}
				if (dist[m_sink] == std::numeric_limits<std::uint64_t>::max())
				{
					break;
				}

				std::uint64_t currPassedFlow = dfs(dist, m_source, flow);
				m_passedFlow += currPassedFlow;
				m_passedFlowCost += currPassedFlow * sinkCumulativePotential;
				flow -= currPassedFlow;
			}

			std::fill(potential.begin(), potential.end(), std::numeric_limits<std::int64_t>::max());

			std::priority_queue<
				std::pair<std::int64_t, std::uint64_t>,
				std::vector<std::pair<std::int64_t, std::uint64_t>>,
				std::greater<std::pair<std::int64_t, std::uint64_t>>> q{};

			q.push({ 0, m_source });

			while (!q.empty())
			{
				std::pair<std::int64_t, std::uint64_t> v = q.top();
				q.pop();

				if (potential[v.second] != std::numeric_limits<std::int64_t>::max())
				{
					continue;
				}

				potential[v.second] = v.first;

				for (const Edge& edgeToNxt : m_graph[v.second])
				{
					if (edgeToNxt.flow && potential[edgeToNxt.to] == std::numeric_limits<std::int64_t>::max())
					{
						q.push({ v.first + edgeToNxt.cost, edgeToNxt.to });
					}
				}
			}
		}
	}

	std::uint64_t getPassedFlow()
	{
		return m_passedFlow;
	}

	std::int64_t getPassedFlowCost()
	{
		return m_passedFlowCost;
	}

	std::vector<bool> getAccessibleFromSource()
	{
		std::vector<bool> visited(m_graph.size());
		std::queue<std::uint64_t> bfs{};
		visited[m_source] = true;
		bfs.push(m_source);
		while (!bfs.empty())
		{
			std::uint64_t v = bfs.front();
			bfs.pop();
			for (const Edge& edgeToNxt : m_graph[v])
			{
				if (edgeToNxt.flow && !visited[edgeToNxt.to])
				{
					visited[edgeToNxt.to] = true;
					bfs.push(edgeToNxt.to);
				}
			}
		}
		return visited;
	}

	std::vector<std::pair<std::uint64_t, std::uint64_t>> getFilledDirectedEdges()
	{
		std::vector<std::pair<std::uint64_t, std::uint64_t>> filledDirectedEdges{};

		for (std::uint64_t i = 0; i < m_graph.size(); ++i)
		{
			for (const Edge& edgeToNxt : m_graph[i])
			{
				if (!edgeToNxt.flow)
				{
					filledDirectedEdges.emplace_back(i, edgeToNxt.to);
				}
			}
		}

		return filledDirectedEdges;
	}

	std::uint64_t getSource()
	{
		return m_source;
	}

	std::uint64_t getSink()
	{
		return m_sink;
	}

private:
	struct Edge
	{
		std::uint64_t to;
		std::uint64_t flow;
		std::uint64_t backInd;
		std::int64_t cost;
	};

	std::uint64_t dfs(std::vector<std::uint64_t>& dist, std::uint64_t v, std::uint64_t flow)
	{
		if (v == m_sink)
		{
			return flow;
		}

		std::uint64_t currPassedFlow = 0;
		for (Edge& edgeToNxt : m_graph[v])
		{
			if (!flow)
			{
				break;
			}
			if (!edgeToNxt.flow || edgeToNxt.cost || dist[edgeToNxt.to] != dist[v] + 1)
			{
				continue;
			}
			std::uint64_t nxtPassedFlow = dfs(dist, edgeToNxt.to, std::min(flow, edgeToNxt.flow));
			currPassedFlow += nxtPassedFlow;
			flow -= nxtPassedFlow;
			edgeToNxt.flow -= nxtPassedFlow;
			m_graph[edgeToNxt.to][edgeToNxt.backInd].flow += nxtPassedFlow;
		}

		dist[v] = std::numeric_limits<std::uint64_t>::max();

		return currPassedFlow;
	}

	std::vector<std::vector<Edge>> m_graph;
	std::uint64_t m_source;
	std::uint64_t m_sink;
	std::uint64_t m_passedFlow = 0;
	std::int64_t m_passedFlowCost = 0;
};