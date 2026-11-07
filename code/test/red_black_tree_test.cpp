#include "red_black_tree.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <random>
#include <set>
#include <vector>

// The tests below verify observable behavior against std::set, which is
// itself backed by a red-black tree: identical insertion/removal results and
// a matching ascending in-order sequence strongly exercise the rebalancing
// paths (rotations, recoloring, transplant and delete fixup).

static void testBasics() {
    RedBlackTree<int> tree;
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(tree.minimum() == nullptr);
    assert(tree.maximum() == nullptr);

    assert(tree.insert(10));
    assert(tree.insert(5));
    assert(tree.insert(20));
    assert(!tree.insert(10));            // duplicate ignored
    assert(tree.size() == 3);
    assert(tree.contains(5) && tree.contains(10) && tree.contains(20));
    assert(!tree.contains(7));
    assert(tree.minimum()->key == 5);
    assert(tree.maximum()->key == 20);

    std::vector<int> expected{5, 10, 20};
    assert(tree.inOrder() == expected);
}

static void testRemove() {
    RedBlackTree<int> tree;
    for (int v : {50, 30, 70, 20, 40, 60, 80, 10}) assert(tree.insert(v));
    assert(tree.remove(20));
    assert(!tree.remove(999));           // absent key
    assert(!tree.contains(20));
    assert(tree.size() == 7);

    std::vector<int> expected{10, 30, 40, 50, 60, 70, 80};
    assert(tree.inOrder() == expected);

    // Remove every element.
    for (int v : expected) assert(tree.remove(v));
    assert(tree.empty());
    assert(tree.inOrder().empty());
}

static void testAgainstStdSet() {
    std::mt19937 rng(12345);
    RedBlackTree<int> tree;
    std::set<int> reference;

    // Random inserts.
    for (int i = 0; i < 5000; ++i) {
        int v = static_cast<int>(rng() % 1000);
        assert(tree.insert(v) == reference.insert(v).second);
    }
    assert(tree.size() == reference.size());

    std::vector<int> mine = tree.inOrder();
    std::vector<int> theirs(reference.begin(), reference.end());
    assert(mine == theirs);

    // Random removals and lookups.
    for (int i = 0; i < 5000; ++i) {
        int v = static_cast<int>(rng() % 1000);
        bool removed = tree.remove(v);
        bool inRef = reference.erase(v) > 0;
        assert(removed == inRef);
        assert(tree.contains(v) == (reference.count(v) > 0));
    }
    assert(tree.size() == reference.size());

    mine = tree.inOrder();
    theirs.assign(reference.begin(), reference.end());
    assert(mine == theirs);
    if (!mine.empty()) {
        assert(tree.minimum()->key == mine.front());
        assert(tree.maximum()->key == mine.back());
    }
}

static void testSortedAndReverseInserts() {
    RedBlackTree<int> tree;
    for (int i = 0; i < 200; ++i) assert(tree.insert(i));
    std::vector<int> mine = tree.inOrder();
    for (int i = 0; i < 200; ++i) assert(mine[i] == i);

    RedBlackTree<int> tree2;
    for (int i = 199; i >= 0; --i) assert(tree2.insert(i));
    for (int i = 0; i < 200; ++i) assert(tree2.inOrder()[i] == i);
}

int main() {
    testBasics();
    testRemove();
    testSortedAndReverseInserts();
    testAgainstStdSet();
    std::printf("All red-black tree tests passed.\n");
    return 0;
}
