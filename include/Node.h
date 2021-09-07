#pragma once

#include "Split.h"
#include "common.h"

class Node {

public:
    int height = 0;
    array<float, 4> rect = {
        numeric_limits<float>::max(),
        numeric_limits<float>::max(),
        numeric_limits<float>::lowest(),
        numeric_limits<float>::lowest(),
    };

    // Directory specific members
    optional<int> numPoints;
    optional<vector<Node*>> contents;
    optional<vector<Split>> splits;

    // Page specific members
    optional<vector<array<float, 2>>> points;

    // Rect methods
    bool containsPt(array<float, 2> p) const;
    array<float, 2> getCenter() const;
    bool inside(array<float, 4>) const;
    double minSqrDist(array<float, 4>) const;
    double minSqrDist(array<float, 2>) const;
    bool overlap(array<float, 4>) const;
    array<float, 4> getRect(array<float, 2>);

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
