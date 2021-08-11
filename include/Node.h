#pragma once

#include <bits/stdc++.h>
#include "Split.h"
#include <chrono>

using namespace std;
using namespace chrono;

class Node {

public:
    int height;
    array<float, 4> rect; // xlow, ylow, xhigh, yhigh

    // Directory specific members
    optional<vector<Node*>> contents;
    optional<vector<Split>> splits;

    // Page specific members
    optional<vector<array<float, 2>>> points;

    // Rect methods
    bool containsPt(array<float, 2> p) const;
    array<float, 2> getCenter() const;
    bool inside(array<float, 4>) const;
    double minSqrDist(array<float, 4>) const;
    bool overlap(array<float, 4>) const;

    // Node methods
    /* void flush();
    long pageCount() const;
    long pointCount() const; */
    bool insertPt(array<float, 2> p, Node *, int, int);
    int rangeSearch(array<float, 4>, map<string, double> &);
    int scan(array<float, 4>) const;
    int size() const;
    vector<Node*> splitDirectory(Node *);
    vector<Node*> splitPage(Node *, int pageCap);

    ~Node();
};