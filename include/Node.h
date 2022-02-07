#pragma once

#include "common.h"

class Node {

public:
    // Ledger is different from Info structure.
    struct Ledger {
        int pages = 0;
        int points = 0;
        int reads = 0;
        int writes = 1;
    };

    struct Split {
        std::array<float, 2> pt;
        bool axis;
    };

    static int directoryCap;
    static int pageCap;

    int height = 0;
    array<float, 4> rect = {
        numeric_limits<float>::max(),
        numeric_limits<float>::max(),
        numeric_limits<float>::lowest(),
        numeric_limits<float>::lowest(),
    };

    // Directory specific members
    optional<Ledger> ledger;
    optional<vector<Node*>> contents;
    optional<vector<Split>> splits;

    // Page specific members
    optional<vector<Record>> points;

    // Rect methods
    bool containsPt(array<float, 2> p) const;
    array<float, 2> getCenter() const;
    bool inside(array<float, 4>) const;
    double minSqrDist(array<float, 4>) const;
    double minSqrDist(array<float, 2>) const;
    bool overlap(array<float, 4>) const;
    array<float, 4> getRect(array<float, 2>);

    // Node methods
    void fission(Node *);
    void fusion(Node *);
    Info insertPt(Record p);
    array<long, 2> getInfo() const;
    Info rangeSearch(array<float, 4>);
    Info refresh();
    int scan(array<float, 4>) const;
    int size() const;
    vector<Node*> splitDirectory(Node *);
    vector<Node*> splitPage(Node *, long);
    void unbind();

    Node(int = 0);
    ~Node();
};
