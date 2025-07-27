#include <iostream>

#include "LinkCutTree.h"

using namespace std;

int main()
{
    // subtree queries work, tested on https://uoj.ac/problem/207

    LinkCutTree<Sum> tree(10);

    for (int i = 0; i < 9; ++i)
    {
        tree.link(i, i + 1);
    }

    tree.updatePathReplace(0, 9, Sum{ 1 });
    tree.reroot(5);
    cout << tree.querySubtree(5).sum << '\n';
    tree.reroot(1);
    cout << tree.querySubtree(5).sum << '\n';
    tree.cut(2, 3);
    tree.reroot(5);
    cout << tree.querySubtree(5).sum << '\n';
    
    return 0;
}