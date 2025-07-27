#pragma once

#include <vector>
#include <stack>
#include <cstdint>

class TwoSat
{
public:
	TwoSat(std::uint64_t n) :
		m_n{ n },
		m_implicationGraph(n * 2)
	{
	}

	void addTrueClause(std::uint64_t a, bool aVal = true)
	{
		if (!aVal)
		{
			a += m_n;
		}

		m_implicationGraph[(m_n + a) % (m_n * 2)].push_back(a);
	}

	void addOrClause(std::uint64_t a, std::uint64_t b, bool aVal = true, bool bVal = true)
	{
		if (!aVal)
		{
			a += m_n;
		}
		if (!bVal)
		{
			b += m_n;
		}

		m_implicationGraph[(m_n + a) % (m_n * 2)].push_back(b);
		m_implicationGraph[(m_n + b) % (m_n * 2)].push_back(a);
	}

	void addXorClause(std::uint64_t a, std::uint64_t b, bool aVal = true, bool bVal = true)
	{
		if (!aVal)
		{
			a += m_n;
		}
		if (!bVal)
		{
			b += m_n;
		}

		m_implicationGraph[(m_n + a) % (m_n * 2)].push_back(b);
		m_implicationGraph[(m_n + b) % (m_n * 2)].push_back(a);
		m_implicationGraph[a].push_back((m_n + b) % (m_n * 2));
		m_implicationGraph[b].push_back((m_n + a) % (m_n * 2));
	}

	void addEqualClause(std::uint64_t a, std::uint64_t b, bool aVal = true, bool bVal = true)
	{
		if (!aVal)
		{
			a += m_n;
		}
		if (!bVal)
		{
			b += m_n;
		}

		m_implicationGraph[a].push_back(b);
		m_implicationGraph[b].push_back(a);
		m_implicationGraph[(m_n + a) % (m_n * 2)].push_back((m_n + b) % (m_n * 2));
		m_implicationGraph[(m_n + b) % (m_n * 2)].push_back((m_n + a) % (m_n * 2));
	}

	void addZeroOrOneClause(std::uint64_t a, std::uint64_t b, bool aVal = true, bool bVal = true)
	{
		if (!aVal)
		{
			a += m_n;
		}
		if (!bVal)
		{
			b += m_n;
		}

		m_implicationGraph[a].push_back((m_n + b) % (m_n * 2));
		m_implicationGraph[b].push_back((m_n + a) % (m_n * 2));
	}

	void addEdge(std::uint64_t reason, std::uint64_t consequence, bool reasonVal = true, bool consequenceVal = true)
	{
		if (!reasonVal)
		{
			reason += m_n;
		}
		if (!consequenceVal)
		{
			consequence += m_n;
		}

		m_implicationGraph[reason].push_back(consequence);
	}

	std::vector<bool> compute() const
	{
		std::vector<std::uint64_t> disc(m_n * 2);
		std::vector<std::uint64_t> low(m_n * 2);
		std::stack<std::uint64_t> verticesStack{};
		std::vector<bool> inStack(m_n * 2);
		std::vector<std::uint64_t> sccComponents(m_n * 2);
		std::uint64_t verticesCounter = 0;
		std::uint64_t sccCounter = 0;
		std::stack<std::uint64_t> recursionStack{};
		std::vector<std::uint64_t> recursionToVisit(m_n * 2, 1);

		std::uint64_t unvisitedVertex = 0;
		while (verticesCounter != m_n * 2)
		{
			while (!recursionToVisit[unvisitedVertex])
			{
				unvisitedVertex++;
			}

			verticesCounter++;
			disc[unvisitedVertex] = verticesCounter;
			low[unvisitedVertex] = verticesCounter;
			verticesStack.push(unvisitedVertex);
			inStack[unvisitedVertex] = true;
			recursionStack.push(unvisitedVertex);
			recursionToVisit[unvisitedVertex] = m_implicationGraph[unvisitedVertex].size();

			while (!recursionStack.empty())
			{
				std::uint64_t recursionVertex = recursionStack.top();

				if (recursionToVisit[recursionVertex])
				{
					recursionToVisit[recursionVertex]--;

					std::uint64_t next = m_implicationGraph[recursionVertex][recursionToVisit[recursionVertex]];

					if (!disc[next])
					{
						verticesCounter++;
						disc[next] = verticesCounter;
						low[next] = verticesCounter;
						verticesStack.push(next);
						inStack[next] = true;
						recursionStack.push(next);
						recursionToVisit[next] = m_implicationGraph[next].size();
					}
				}
				else
				{
					recursionStack.pop();

					for (std::uint64_t next : m_implicationGraph[recursionVertex])
					{
						if (inStack[next])
						{
							low[recursionVertex] = std::min(low[recursionVertex], low[next]);
						}
					}

					if (low[recursionVertex] == disc[recursionVertex])
					{
						sccCounter++;
						while (true)
						{
							std::uint64_t top = verticesStack.top();
							verticesStack.pop();
							sccComponents[top] = sccCounter;
							inStack[top] = false;
							if (top == recursionVertex)
							{
								break;
							}
						}
					}
				}
			}
		}

		std::vector<bool> result(m_n, false);

		for (std::uint64_t i = 0; i < m_n; i++)
		{
			if (sccComponents[i] == sccComponents[m_n + i])
			{
				return std::vector<bool>{};
			}
			else if (sccComponents[i] < sccComponents[m_n + i])
			{
				result[i] = true;
			}
		}

		return result;
	}

private:
	std::uint64_t m_n;
	std::vector<std::vector<std::uint64_t>> m_implicationGraph;
};