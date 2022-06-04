#pragma once

#include "Node.h"
#include "common.h"

using namespace std;

struct Waffle {

    Node *root;

    Waffle(int, int);
    ~Waffle();

    void snapshot() const;
    void load(string, long);
    void bulkload(string, long);
    Info rangeQuery(Rect);
    Info deleteQuery(Entry);
    Info insertQuery(Entry);
    Info kNNQuery(array<float, 2>, int);
    void insertPoint(Node *, Node *, array<float, 2>);
    int size(map<string, double> &) const;
};
