#pragma once

#include <bits/stdc++.h>
#include "Node.h"
#include <chrono>

using namespace std;

class PageMin {

public:
    int directoryCap;
    int pageCap;
    Node *root;

    PageMin(int, int, array<float, 4>);
    ~PageMin();

    void snapshot() const;
    void load(string, long);
    void pageFission(Node *);
    void directoryFission(Node *);
    void bulkload(string, long);
    void rangeQuery(array<float, 4>, map<string, double> &);
    void deleteQuery(array<float, 2>, map<string, double> &);
    void insertQuery(array<float, 2>, map<string, double> &);
    void kNNQuery(array<float, 2>, map<string, double> &, int);
    void insertPoint(Node *, Node *, array<float, 2>);
    int size(map<string, double> &) const;
};
