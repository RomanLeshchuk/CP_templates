#include <iostream>

#include "LinkCutTree.h"

using namespace std;

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int q;
    cin >> q;

    int last = 0;
    LinkCutTree<Sum, StoreType::PATH_DATA> tree(q);

    int lastAns = 0;
    while (q--)
    {
        int op;
        cin >> op;
        if (op == 1)
        {
            int v;
            cin >> v;
            v ^= lastAns;
            --v;
            ++last;
            tree.link(v, last);
        }
        else
        {
            int u, v;
            cin >> u >> v;
            u ^= lastAns;
            v ^= lastAns;
            --u;
            --v;
            lastAns = tree.getPathSize(u, v) - 1;
            cout << lastAns << '\n';
        }
    }
    
    return 0;
}