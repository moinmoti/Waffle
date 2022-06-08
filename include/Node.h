#pragma once

#include "common.h"

struct NbEntry {
    Entry pt;
    double dist = numeric_limits<float>::max();
    bool operator<(const NbEntry &second) const { return dist < second.dist; }
};

struct NbNode {
    // Node *self;
    NbNode *parent;
    double dist = numeric_limits<float>::max();

    bool operator>(const NbNode &second) const { return dist > second.dist; }

    virtual void search(const Rect&, min_heap<NbNode *>&, max_heap<NbEntry> &, vector<NbNode *> &) = 0;
    virtual Info track() = 0;
    virtual ~NbNode() = default;
};

struct Node {
    struct Split {
        Point pt;
        bool axis;
    };

    Rect rect = {
        numeric_limits<float>::max(),
        numeric_limits<float>::max(),
        numeric_limits<float>::lowest(),
        numeric_limits<float>::lowest(),
    };

    // Rect methods
    bool containsPt(const Point&) const;
    Point getCenter() const;
    bool inside(const Rect&) const;
    double minSqrDist(const Rect&) const;
    double minSqrDist(const Point&) const;
    bool overlap(const Rect&) const;
    Rect getRect(const Point&) const;
    Rect getRect(const Rect&) const;

    // Node methods
    virtual void aggrInfo(Info&) const = 0;
    virtual uint findHeight() const = 0;
    virtual NbNode* getNbNode() = 0;
    virtual Info insert(Node *, uint, const Entry&) = 0;
    virtual Info range(const Rect&) = 0;
    virtual uint size(About&) const = 0;
    virtual array<Node*, 2> split(Node *, uint) = 0;
    virtual array<Node*, 2> split(Node *) = 0;
    virtual void snapshot(ofstream&) const = 0;
    virtual void unbind(Node *) = 0;

    virtual ~Node() = default;
};

struct Directory : Node {
    static uint capacity;
    Info ledger;
    vector<Node*> contents;
    vector<Split> splits;

    Directory();

    void aggrInfo(Info &) const;
    uint findHeight() const;
    void fusion(Node *);
    NbNode* getNbNode();
    Info insert(Node *pn, uint, const Entry&);
    Info range(const Rect&);
    Info refresh();
    uint size(About&) const;
    array<Node*, 2> split(Node *, uint);
    array<Node*, 2> split(Node *);
    void snapshot(ofstream&) const;
    void unbind(Node *);

    ~Directory();
};

struct NbDirectory : NbNode {
    Directory *self;
    unordered_set<NbNode *> branch;

    explicit NbDirectory(Directory *);

    void enlist(NbNode *);
    void search(const Rect&, min_heap<NbNode *>&, max_heap<NbEntry> &, vector<NbNode *> &);
    Info track();
    ~NbDirectory();
};

struct Page : Node {
    static uint capacity;
    vector<Entry> entries;

    Page();

    void aggrInfo(Info &) const;
    uint findHeight() const;
    void fission(Node *);
    NbNode* getNbNode();
    Info insert(Node *pn, uint, const Entry&);
    Info range(const Rect&);
    uint size(About&) const;
    array<Node*, 2> split(Node *, uint);
    array<Node*, 2> split(Node *);
    void snapshot(ofstream&) const;
    void unbind(Node *);

    ~Page();
};

struct NbPage : NbNode {
    Page *self;

    explicit NbPage(Page *);

    void search(const Rect&, min_heap<NbNode *>&, max_heap<NbEntry> &, vector<NbNode *> &);
    Info track();
    ~NbPage();
};
