#include "PageMin.h"

using namespace std;
using namespace std::chrono;

#define TRACE
#ifdef TRACE
#define trace(...) __f(#__VA_ARGS__, __VA_ARGS__)
template <typename Arg1> void __f(const char *name, Arg1 &&arg1) {
    cerr << name << " : " << arg1 << endl;
}
template <typename Arg1, typename... Args>
void __f(const char *names, Arg1 &&arg1, Args &&... args) {
    const char *comma = strchr(names + 1, ',');
    cerr.write(names, comma - names) << " : " << setw(9) << arg1 << " | ";
    __f(comma + 1, args...);
}
#else
#define trace(...)
#endif

#define NUMDIMS 2
#define all(c) c.begin(), c.end()

void printRect(string Rect, array<float, 4> r) {
    cerr << Rect << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

PageMin::PageMin(int _pageCap, int _directoryCap, array<float, 4> _boundary) {
    pageCap = _pageCap;
    directoryCap = _directoryCap;

    root = new Node();
    root->rect = _boundary;
    root->height = 0;
}

PageMin::~PageMin() {}

void PageMin::snapshot() const {
    ofstream log("PageMin.csv");
    stack<Node *> toVisit({root});
    Node *directory;
    while (!toVisit.empty()) {
        directory = toVisit.top();
        toVisit.pop();
        log << directory->height << "," << directory->contents->size();
        for (auto p : directory->rect)
            log << "," << p;
        log << endl;
        for (auto cn : directory->contents.value()) {
            if (cn->points) {
                log << cn->height << "," << cn->points->size();
                for (auto p : cn->rect)
                    log << "," << p;
                log << endl;
            } else {
                toVisit.push(cn);
            }
        }
    }
    log.close();
}

void PageMin::directoryFission(Node *node) {
    vector<Node *> directories = node->splitDirectory(root);
    for (auto directory : directories) {
        if (directory->contents->size() > directoryCap) {
            directoryFission(directory);
            delete directory;
        } else
            root->contents->emplace_back(directory);
    }
}

void PageMin::pageFission(Node *node) {
    vector<Node *> pages = node->splitPage(root, pageCap);
    for (auto page : pages) {
        if (page->points->size() > pageCap) {
            pageFission(page);
            delete page;
        } else
            root->contents->emplace_back(page);
    }
}

void PageMin::bulkload(string filename, long limit) {
    string line;
    ifstream file(filename);

    int i = 0;
    vector<array<float, 2>> Points;
    Points.reserve(limit);
    if (file.is_open()) {
        // getline(file, line);
        while (getline(file, line)) {
            int id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lat >> lon;
            array pt{lon, lat};
            Points.emplace_back(pt);
            if (++i >= limit)
                break;
        }
        file.close();
    } else
        cerr << "Data file " << filename << " not found!";

    cout << "Initiate page fission" << endl;
    root->points = Points;
    root->contents = vector<Node *>();
    root->splits = vector<Split *>();
    pageFission(root);
    root->height = 1;
    root->points->clear();
    root->points.reset();

    cout << "Initiate directory fission" << endl;
    while (root->contents->size() > directoryCap) {
        directoryFission(root);
        root->height++;
    }
}

void PageMin::insertPoint(Node *pn, Node *node, array<float, 2> p) {
    vector<Node *> newNodes;
    if (node->points) {
        node->points->emplace_back(p);
        if (node->points->size() > pageCap)
            newNodes = node->splitPage(pn, pageCap);
    } else {
        auto cn = node->contents->begin();
        while (!(*cn)->containsPt(p))
            cn++;
        insertPoint(node, *cn, p);
        if (node->contents->size() > directoryCap)
            newNodes = node->splitDirectory(pn);
    }
    if (!newNodes.empty()) {
        if (node == root) {
            root->contents->clear();
            root->height++;
        } else {
            pn->contents->erase(find(all(pn->contents.value()), node));
            delete node;
        }
        for (auto cn : newNodes)
            pn->contents->emplace_back(cn);
    }
}

void PageMin::insertQuery(array<float, 2> p, map<string, double> &stats) {
    insertPoint(root, root, p);
}

void PageMin::deleteQuery(array<float, 2> p, map<string, double> &stats) {
    Node *node = root;
    while (node->contents) {
        auto cn = node->contents->begin();
        while (!(*cn)->containsPt(p))
            cn++;
        node = *cn;
    }
    auto pt = find(all(node->points.value()), p);
    if (pt != node->points->end())
        node->points->erase(pt);
}

void rangeSearch(Node *node, int &pointCount, array<float, 4> query, map<string, double> &stats) {
    if (node->points) {
        stats["io"]++;
        // high_resolution_clock::time_point startTime = high_resolution_clock::now();
        // pointCount += node->scan(query);
        /* stats["scanTime"] +=
            duration_cast<microseconds>(high_resolution_clock::now() - startTime).count(); */
    } else {
        for (auto cn : node->contents.value())
            if (cn->overlap(query))
                rangeSearch(cn, pointCount, query, stats);
    }
}

void PageMin::rangeQuery(array<float, 4> query, map<string, double> &stats) {
    int pointCount = 0;
    rangeSearch(root, pointCount, query, stats);
    // trace(pointCount);
}

typedef struct knnPoint {
    array<float, 2> pt;
    double dist = FLT_MAX;
    bool operator<(const knnPoint &second) const { return dist < second.dist; }
} knnPoint;

typedef struct knnNode {
    Node *sn;
    double dist = FLT_MAX;
    bool operator<(const knnNode &second) const { return dist > second.dist; }
} knnNode;

void kNNSearch(Node *node, array<float, 4> query,
               priority_queue<knnPoint, vector<knnPoint>> &knnPts, map<string, double> &stats) {
    auto sqrDist = [](array<float, 4> x, array<float, 2> y) {
        return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2);
    };
    priority_queue<knnNode, vector<knnNode>> unseenNodes;
    unseenNodes.emplace(knnNode{node, node->minSqrDist(query)});
    double dist, minDist;
    // high_resolution_clock::time_point startTime;
    while (!unseenNodes.empty()) {
        // startTime = high_resolution_clock::now();
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnPts.top().dist;
        /* stats["explore"] +=
            duration_cast<microseconds>(high_resolution_clock::now() - startTime).count(); */
        if (dist < minDist) {
            if (node->points) {
                // startTime = high_resolution_clock::now();
                for (auto p : node->points.value()) {
                    minDist = knnPts.top().dist;
                    dist = sqrDist(query, p);
                    if (dist < minDist) {
                        knnPoint kPt;
                        kPt.pt = p;
                        kPt.dist = dist;
                        knnPts.pop();
                        knnPts.push(kPt);
                        // stats["heapAccess"]++;
                    }
                    // stats["scanCount"]++;
                }
                stats["io"]++;
                /* stats["scan"] +=
                    duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
                 */
            } else {
                // startTime = high_resolution_clock::now();
                for (auto cn : node->contents.value()) {
                    minDist = knnPts.top().dist;
                    dist = cn->minSqrDist(query);
                    if (dist < minDist) {
                        knnNode kn;
                        kn.sn = cn;
                        kn.dist = dist;
                        unseenNodes.push(kn);
                    }
                }
                /* stats["explore"] +=
                    duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
                 */
            }
        } else
            break;
    }
}

void PageMin::kNNQuery(array<float, 2> p, map<string, double> &stats, int k) {
    Node *foundNode;
    array query{p[0], p[1], p[0], p[1]};

    vector<knnPoint> tempPts(k);
    priority_queue<knnPoint, vector<knnPoint>> knnPts(all(tempPts));
    kNNSearch(root, query, knnPts, stats);

    /* double sqrDist;
    while (!knnPts.empty()) {
        p = knnPts.top().pt;
        sqrDist = knnPts.top().dist;
        knnPts.pop();
        trace(p[0], p[1], sqrDist);
    } */
}

int PageMin::size(map<string, double> &stats) const {
    int totalSize = 2 * sizeof(int);
    int pageSize = 4 * sizeof(float) + sizeof(int) + sizeof(Node *);
    int directorySize = 4 * sizeof(float) + 2 * sizeof(int) + sizeof(Node *);
    int splitSize = 2 * sizeof(float) + sizeof(bool);
    stack<Node *> toVisit({root});
    Node *directory;
    while (!toVisit.empty()) {
        directory = toVisit.top();
        toVisit.pop();
        stats["directories"]++;
        for (auto cn : directory->contents.value()) {
            if (cn->contents) {
                stats["splits"] += cn->splits->size();
                toVisit.push(cn);
            } else
                stats["pages"]++;
        }
    }
    totalSize += pageSize * stats["pages"] + directorySize * stats["directories"] +
                 splitSize * stats["splits"];
    return totalSize;
}
