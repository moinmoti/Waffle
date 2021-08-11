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
    array<float, 4> combineRect(array<float, 4>);
    bool overlap(array<float, 4>) const;
    bool containsPt(array<float, 2> p) const;
    bool inside(array<float, 4>) const;
    array<float, 2> getCenter() const;
    double minSqrDist(array<float, 4>) const;

    // Common Node methods
    int size() const;

    // Directory specific methods
    vector<Node*> splitPage(Node *, int pageCap);

    // Page specific methods
    vector<Node*> splitDirectory(Node *);
    int scan(array<float, 4>) const;

    ~Node();
};
