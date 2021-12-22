#pragma once

#include "Node.h"
#include "common.h"

using namespace std;

class MPT {

public:
    Node *root;

    MPT(int, int);
    ~MPT();

    void snapshot() const;
    void load(string, long);
    void bulkload(string, long);
    Info rangeQuery(array<float, 4>);
    Info deleteQuery(Record);
    Info insertQuery(Record);
    Info kNNQuery(array<float, 2>, int);
    void insertPoint(Node *, Node *, array<float, 2>);
    int size(map<string, double> &) const;
};
