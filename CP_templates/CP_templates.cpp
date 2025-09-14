#include <iostream>
#include <random>

#include "LinkCutTree.h"

using namespace std;

int root;
vector<vector<int>> tree;
vector<long long> vals;

bool addToPath(int v, int prev, int to, long long val)
{
    if (v == to)
    {
        vals[v] += val;
        return true;
    }
    bool onPath = false;
    for (int nxtV : tree[v])
    {
        if (nxtV != prev)
        {
            onPath |= addToPath(nxtV, v, to, val);
        }
    }
    if (onPath)
    {
        vals[v] += val;
    }
    return onPath;
}

pair<bool, long long> queryPath(int v, int prev, int to)
{
    if (v == to)
    {
        return { true, vals[v] };
    }
    bool onPath = false;
    long long res = numeric_limits<int64_t>::min();
    for (int nxtV : tree[v])
    {
        if (nxtV != prev)
        {
            pair<bool, long long> currRes = queryPath(nxtV, v, to);
            onPath |= currRes.first;
            res = max(res, currRes.second);
        }
    }
    if (onPath)
    {
        res = max(res, vals[v]);
    }
    return { onPath, res };
}

void addToSubtree(int v, int prev, long long val)
{
    vals[v] += val;
    for (int nxtV : tree[v])
    {
        if (nxtV != prev)
        {
            addToSubtree(nxtV, v, val);
        }
    }
}

long long querySubtree(int v, int prev)
{
    long long res = vals[v];
    for (int nxtV : tree[v])
    {
        if (nxtV != prev)
        {
            res = max(res, querySubtree(nxtV, v));
        }
    }
    return res;
}

int main()
{
    constexpr int n = 1000;
    constexpr int q = 10000;

    random_device device{};
    mt19937_64 engine(device());
    uniform_int_distribution<int> dist(0);

    tree.resize(n);
    vals.resize(n);
    int root = 0;

    LinkCutTree<Max, StoreType::ALL_DATA> lct(n);

    for (int i = 1; i < n; ++i)
    {
        int parent = dist(engine) % i;
        tree[parent].push_back(i);
        tree[i].push_back(parent);
        lct.link(parent, i);
    }

    // update subtree by does not work with replace path together, so no replace path

    for (int query = 1; query <= q; ++query)
    {
        int op = dist(engine) % 6;
        if (op == 0)
        {
            // reroot
            root = dist(engine) % n;
            lct.reroot(root);
        }
        else if (op == 1)
        {
            // add to path
            int a = dist(engine) % n;
            int b = dist(engine) % n;
            long long val = dist(engine);
            addToPath(a, -1, b, val);
            lct.updatePathBy(a, b, val);
        }
        else if (op == 2)
        {
            // query path
            int a = dist(engine) % n;
            int b = dist(engine) % n;
            long long bruteforceRes = queryPath(a, -1, b).second;
            long long lctRes = lct.queryPath(a, b).max;
            if (bruteforceRes != lctRes)
            {
                cout << lctRes << ' ' << bruteforceRes << '\n';
                cout << "bug in query path\n";
            }
        }
        else if (op == 3)
        {
            // cut and link and reroot
            int a = dist(engine) % n;
            int b = tree[a][dist(engine) % tree[a].size()];
            lct.cut(a, b);
            int c = dist(engine) % n;
            int d = c;
            while (lct.getRoot(c) == lct.getRoot(d))
            {
                d = dist(engine) % n;
            }
            lct.link(c, d);
            root = dist(engine) % n;
            lct.reroot(root);

            tree[a].erase(find(tree[a].begin(), tree[a].end(), b));
            tree[b].erase(find(tree[b].begin(), tree[b].end(), a));
            tree[c].push_back(d);
            tree[d].push_back(c);
        }
        else if (op == 4)
        {
            // update subtree
            int a = dist(engine) % n;
            long long val = dist(engine);
            if (lct.getRoot(a) == a)
            {
                addToSubtree(a, -1, val);
            }
            else
            {
                addToSubtree(a, lct.getNthParent(a, 0), val);
            }
            lct.updateSubtreeBy(a, val);
        }
        else if (op == 5)
        {
            /*// query subtree
            int a = dist(engine) % n;
            long long bruteforceRes;
            if (lct.getRoot(a) == a)
            {
                bruteforceRes = querySubtree(a, -1);
            }
            else
            {
                bruteforceRes = querySubtree(a, lct.getNthParent(a, 0));
            }
            long long lctRes = lct.querySubtree(a).max;
            if (bruteforceRes != lctRes)
            {
                cout << lctRes << ' ' << bruteforceRes << '\n';
                cout << "bug in query subtree\n";
            }*/
        }
    }
    
    return 0;
}

