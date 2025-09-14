#include <iostream>

#include "LinkCutTree.h"

#include <iostream>

using namespace std;

int main()
{
    srand(time(nullptr));

    int n = 1000;
    int q = 10000;

    LinkCutTree<Sum, StoreType::ALL_DATA> tree(n);

    for (int i = 0; i < tree.size() - 1; ++i)
    {
        tree.link(i, i + 1);
    }

    int root = 0;
    vector<int> arr(n);
    for (int query = 1; query <= q; ++query)
    {
        if (rand() & 1)
        {
            int subtree = rand() % n;
            int val = 1;
            tree.updateSubtreeBy(subtree, val);
            if (subtree < root)
            {
                for (int i = 0; i <= subtree; ++i)
                {
                    arr[i] += val;
                }
            }
            else if (subtree > root)
            {
                for (int i = subtree; i < n; ++i)
                {
                    arr[i] += val;
                }
            }
            else
            {
                for (int i = 0; i < n; ++i)
                {
                    arr[i] += val;
                }
            }
        }
        else
        {
            int subtree = rand() % n;
            long long bruteforceAns = 0;
            if (subtree < root)
            {
                for (int i = 0; i <= subtree; ++i)
                {
                    bruteforceAns += arr[i];
                }
            }
            else if (subtree > root)
            {
                for (int i = subtree; i < n; ++i)
                {
                    bruteforceAns += arr[i];
                }
            }
            else
            {
                for (int i = 0; i < n; ++i)
                {
                    bruteforceAns += arr[i];
                }
            }
            long long ans = tree.querySubtree(subtree).sum;
            if (ans != bruteforceAns)
            {
                cout << ans << " != " << bruteforceAns << ", bug\n";
            }
        }
    }
    
    return 0;
}

