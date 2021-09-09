#pragma once

#include "Split.h"
#include "common.h"

class Node {

public:
    int height;
    array<float, 4> rect; // xlow, ylow, xhigh, yhigh

    // Directory specific members
    optional<vector<Node*>> contents;
    optional<int> numPoints;
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
    void fission(Node *, int);
    void fusion(Node *, int);
    void insertPt(array<float, 2> p, int, int);
    long pageCount() const;
    long pointCount() const;
    int rangeSearch(array<float, 4>, map<string, double> &);
    int scan(array<float, 4>) const;
    int size() const;
    vector<Node*> splitDirectory(Node *);
    vector<Node*> splitPage(Node *, long);
    int unbind();

    ~Node();
};
