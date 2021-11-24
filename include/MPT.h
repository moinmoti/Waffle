#pragma once

#include "Node.h"
#include "common.h"

using namespace std;

class MPT {

public:
    int directoryCap;
    int pageCap;
    Node *root;

    MPT(int, int);
    ~MPT();

    void snapshot() const;
    void load(string, long);
    void bulkload(string, long);
    void rangeQuery(array<float, 4>, map<string, double> &);
    void deleteQuery(Record, map<string, double> &);
    void insertQuery(Record, map<string, double> &);
    void kNNQuery(array<float, 2>, map<string, double> &, int);
    void insertPoint(Node *, Node *, array<float, 2>);
    int size(map<string, double> &) const;
};
