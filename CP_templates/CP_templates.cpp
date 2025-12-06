#include <iostream>
#include <string>

#include "DisjointSparseTable.h"

using namespace std;

int main()
{
    int n, q;
    cin >> n >> q;
    vector<Sum> a(n);
    for (int i = 0; i < n; ++i)
    {
        cin >> a[i].val;
    }
    DisjointSparseTable<Sum> table(a);
    while (q--)
    {
        int l, r;
        cin >> l >> r;
        --l;
        --r;
        cout << table.query(l, r).val << '\n';
    }
    return 0;
}