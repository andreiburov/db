#ifndef DB_BPLUSTREE_HELPER_H
#define DB_BPLUSTREE_HELPER_H

struct IntComparator {
    int operator()(const int& a, const int& b) {
        if (a == b)
            return 0;
        else if (a < b)
            return -1;
        else
            return 1;
    }
};

typedef BPlusTree<int, IntComparator, PAGESIZE> BPT;

#endif //DB_BPLUSTREE_HELPER_H
