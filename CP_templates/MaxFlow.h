#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>

class MaxFlow
{
public:
	MaxFlow(std::uint64_t n, std::uint64_t source, std::uint64_t sink) :
		m_graph(n),
		m_source{ source },
		m_sink{ sink }
	{
	}

	MaxFlow(const MaxFlow& maxFlow) = default;

	void addEdgeDirected(std::uint64_t from, std::uint64_t to, std::uint64_t capacity)
	{
		if (from == to)
		{
			return;
		}

		m_graph[from].push_back({ to, capacity, m_graph[to].size() });
		m_graph[to].push_back({ from, 0, m_graph[from].size() - 1 });
	}

	void addEdgeUndirected(std::uint64_t a, std::uint64_t b, std::uint64_t capacity)
	{
		if (a == b)
		{
			return;
		}

		m_graph[a].push_back({ b, capacity, m_graph[b].size() });
		m_graph[b].push_back({ a, capacity, m_graph[a].size() - 1 });
	}

	void passFlow(std::uint64_t flow)
	{
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
					if (edgeToNxt.flow && dist[edgeToNxt.to] == std::numeric_limits<std::uint64_t>::max())
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
			flow -= currPassedFlow;
		}
	}

	std::uint64_t getPassedFlow()
	{
		return m_passedFlow;
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
			if (!edgeToNxt.flow || dist[edgeToNxt.to] != dist[v] + 1)
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
};