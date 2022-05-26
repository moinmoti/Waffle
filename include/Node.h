#pragma once

#include "common.h"

struct Node {

    // Ledger is different from Info structure.
    struct Ledger {
        int pages = 0;
        int points = 0;
        float reads = 0;
        float writes = 1;
    };

    struct Split {
        bool axis;
        Point pt;
    };

    static int directoryCap;
    static int pageCap;

    int height = 0;
    Rect rect = {
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
    optional<vector<Entry>> points;

    // Rect methods
    bool containsPt(Point p) const;
    Point getCenter() const;
    bool inside(Rect) const;
    double minSqrDist(Rect) const;
    double minSqrDist(Point) const;
    bool overlap(Rect) const;
    Rect getRect(Point);

    // Node methods
    void fission(Node *);
    void fusion(Node *);
    Info insertPt(Entry p);
    array<long, 2> getInfo() const;
    Info rangeSearch(Rect);
    Info refresh();
    int scan(Rect) const;
    int size() const;
    vector<Node*> splitDirectory(Node *);
    vector<Node*> splitPage(Node *, long);
    void unbind();

    Node(int = 0);
    ~Node();
};

struct Directory: Node {
    static int capacity;
    Ledger ledger;
    vector<Node*> sections;
    vector<Split> splits;

};

struct Page: Node {
    static int capacity;
    vector<Entry> entries;

};
