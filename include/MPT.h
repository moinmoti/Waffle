#pragma once

#include "Node.h"
#include "common.h"

using namespace std;

class MPT {

public:
    Node *root;

    MPT(int, int);
    ~MPT();

    void bulkload(string, long);
    Info deleteQuery(Record);
    Info insertQuery(Record);
    Info kNNQuery(array<float, 2>, int);
    Info rangeQuery(array<float, 4>);
    int size(map<string, double> &) const;
    void snapshot(string) const;
};
