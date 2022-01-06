#include "MPT.h"

void printRect(string Rect, array<float, 4> r) {
    cerr << Rect << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

MPT::MPT(int _directoryCap, int _pageCap) {
    Node::directoryCap = _directoryCap;
    Node::pageCap = _pageCap;
    root = new Node();
    root->ledger->reads = 0;
    root->ledger->writes = 1;
}

MPT::~MPT() {}

void MPT::snapshot() const {
    ofstream log("MPT.csv");
    stack<Node *> toVisit({root});
    Node *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        log << dir->height << "," << dir->contents->size();
        for (auto p : dir->rect)
            log << "," << p;
        float tolerance = dir->ledger->writes / float(dir->ledger->writes + dir->ledger->reads);
        trace(dir->ledger->reads, dir->ledger->writes, tolerance);
        log << "," << tolerance;
        log << endl;
        for (auto cn : dir->contents.value()) {
            if (cn->points) {
                /* log << cn->height << "," << cn->points->size();
                for (auto p : cn->rect)
                    log << "," << p;
                log << endl; */
            } else {
                toVisit.push(cn);
            }
        }
    }
    log.close();
}

void MPT::bulkload(string filename, long limit) {
    string line;
    ifstream file(filename);

    int i = 0;
    vector<Record> Points;
    Points.reserve(limit);
    if (file.is_open()) {
        // getline(file, line);
        while (getline(file, line)) {
            int id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lon >> lat;
            Record pt;
            pt.id = id;
            pt.data = {lon, lat};
            Points.emplace_back(pt);
            if (root->rect[0] > pt.data[0])
                root->rect[0] = pt.data[0];
            if (root->rect[1] > pt.data[1])
                root->rect[1] = pt.data[1];
            if (root->rect[2] < pt.data[0])
                root->rect[2] = pt.data[0];
            if (root->rect[3] < pt.data[1])
                root->rect[3] = pt.data[1];
            if (++i >= limit)
                break;
        }
        file.close();
    } else
        cerr << "Data file " << filename << " not found!";

    cout << "Initiate page fission" << endl;
    root->points = Points;
    root->contents = vector<Node *>();
    root->splits = vector<Split>();
    root->fission(root);
    root->ledger->pages = root->contents->size();
    root->ledger->points = root->points->size();
    root->height = 1;
    root->points->clear();
    root->points.reset();

    cout << "Initiate directory fission" << endl;
    while (root->contents->size() > root->directoryCap) {
        root->fusion(root);
        root->height++;
    }
}

Info MPT::insertQuery(Record pt) {
    if (root->minSqrDist(pt.data) > 0)
        root->rect = root->getRect(pt.data);
    Info stats = root->insertPt(pt);
    if (root->contents->size() > root->directoryCap) {
        vector<Node *> newNodes = root->splitDirectory(root);
        for (auto node : newNodes)
            root->contents->emplace_back(node);
        root->height++;
    }
    return stats;
}

Info MPT::deleteQuery(Record p) {
    Node *node = root;
    while (node->contents) {
        auto cn = node->contents->begin();
        while (!(*cn)->containsPt(p.data))
            cn++;
        node = *cn;
    }
    Info stats;
    // Find the point using the id and delete it.
    return stats;
}

Info MPT::rangeQuery(array<float, 4> query) {
    Info stats = root->rangeSearch(query);
    int pointCount = stats.points;
    // trace(pointCount);
    return stats;
}

struct knnPoint {
    Record pt;
    double dist = numeric_limits<float>::max();
    bool operator<(const knnPoint &second) const { return dist < second.dist; }
};

struct knnNode {
    Node *self;
    knnNode *parent;
    double dist = numeric_limits<float>::max();
    unordered_set<knnNode *> branch;

    Info track() {
        Info info;
        if (self->points) {
            info.reads = 1;
        } else {
            for (auto kn : branch)
                info += kn->track();
            self->ledger->pages += info.pages;
            self->ledger->reads++;
            info += self->refresh();
        }
        return info;
    }
};

Info MPT::kNNQuery(array<float, 2> queryPt, int k) {
    auto calcSqrDist = [](array<float, 2> x, array<float, 2> y) {
        return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2);
    };
    auto compare = [](knnNode *l, knnNode *r) { return l->dist > r->dist; };

    vector<knnPoint> tempPts(k);
    priority_queue<knnPoint, vector<knnPoint>> knnPts(all(tempPts));
    priority_queue<knnNode *, vector<knnNode *>, decltype(compare)> unseenNodes(compare);
    knnNode *rootKNode = new knnNode();
    rootKNode->self = root;
    rootKNode->parent = NULL;
    rootKNode->dist = root->minSqrDist(queryPt);
    unseenNodes.emplace(rootKNode);

    while (!unseenNodes.empty()) {
        knnNode *kNode = unseenNodes.top();
        Node *node = kNode->self;
        double dist = kNode->dist;
        unseenNodes.pop();
        double minDist = knnPts.top().dist;
        if (dist < minDist) {
            if (node->points) {
                for (auto p : node->points.value()) {
                    minDist = knnPts.top().dist;
                    dist = calcSqrDist(queryPt, p.data);
                    if (dist < minDist) {
                        knnPoint kPt;
                        kPt.pt = p;
                        kPt.dist = dist;
                        knnPts.pop();
                        knnPts.push(kPt);
                    }
                }
                while (kNode->parent) {
                    kNode->parent->branch.insert(kNode);
                    kNode = kNode->parent;
                }
            } else {
                minDist = knnPts.top().dist;
                for (auto cn : node->contents.value()) {
                    dist = cn->minSqrDist(queryPt);
                    if (dist < minDist) {
                        knnNode *kn = new knnNode();
                        kn->self = cn;
                        kn->parent = kNode;
                        kn->dist = dist;
                        unseenNodes.push(kn);
                    }
                }
            }
        } else
            break;
    }

    /* double sqrDist;
    Record p;
    while (!knnPts.empty()) {
        p = knnPts.top().pt;
        sqrDist = knnPts.top().dist;
        knnPts.pop();
        trace(p.id, sqrDist);
    } */

    Info info = rootKNode->track();
    return info;
}

int MPT::size(map<string, double> &stats) const {
    int totalSize = 2 * sizeof(int);
    int pageSize = 4 * sizeof(float) + sizeof(int) + sizeof(Node *);
    int directorySize = 4 * sizeof(float) + sizeof(int) + sizeof(Node *) + sizeof(Node::Ledger);
    int splitSize = 2 * sizeof(float) + sizeof(bool);
    stack<Node *> toVisit({root});
    Node *directory;
    while (!toVisit.empty()) {
        directory = toVisit.top();
        toVisit.pop();
        stats["directories"]++;
        stats["splits"] += directory->splits->size();
        for (auto cn : directory->contents.value()) {
            if (cn->contents)
                toVisit.push(cn);
            else
                stats["pages"]++;
        }
    }
    totalSize += pageSize * stats["pages"] + directorySize * stats["directories"] +
                 splitSize * stats["splits"];
    return totalSize;
}
