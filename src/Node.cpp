#include "Node.h"

// Definition of static variables.
int Node::directoryCap;
int Node::pageCap;
int Node::trendCoeff;

/////////////////////////////////////////////////////////////////////////////////////////
// Rectangle Methods
/////////////////////////////////////////////////////////////////////////////////////////

void printNode(string str, array<float, 4> r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

bool Node::containsPt(array<float, 2> p) const {
    bool result = true;
    for (int i = 0; i < D; i++)
        result = result & (rect[i] <= p[i]) & (rect[i + D] >= p[i]);
    return result;
}

bool Node::inside(array<float, 4> r) const {
    bool result = true;
    for (int i = 0; i < D; i++)
        result = result & (rect[i] >= r[i]) & (rect[i + D] <= r[i + D]);
    return result;
}

array<float, 2> Node::getCenter() const {
    return array<float, 2>{(rect[0] + rect[2]) / 2, (rect[1] + rect[3]) / 2};
}

double Node::minSqrDist(array<float, 4> r) const {
    bool left = r[2] < rect[0];
    bool right = rect[2] < r[0];
    bool bottom = r[3] < rect[1];
    bool top = rect[3] < r[1];
    if (top) {
        if (left)
            return dist(rect[0], rect[3], r[2], r[1]);
        if (right)
            return dist(rect[2], rect[3], r[0], r[1]);
        return (r[1] - rect[3]) * (r[1] - rect[3]);
    }
    if (bottom) {
        if (left)
            return dist(rect[0], rect[1], r[2], r[3]);
        if (right)
            return dist(rect[2], rect[1], r[0], r[3]);
        return (rect[1] - r[3]) * (rect[1] - r[3]);
    }
    if (left)
        return (rect[0] - r[2]) * (rect[0] - r[2]);
    if (right)
        return (r[0] - rect[2]) * (r[0] - rect[2]);
    return 0;
}

double Node::minSqrDist(array<float, 2> q) const {
    bool left = q[0] < rect[0];
    bool right = rect[2] < q[0];
    bool bottom = q[1] < rect[1];
    bool top = rect[3] < q[1];
    if (top) {
        if (left)
            return dist(rect[0], rect[3], q[0], q[1]);
        if (right)
            return dist(rect[2], rect[3], q[0], q[1]);
        return (q[1] - rect[3]) * (q[1] - rect[3]);
    }
    if (bottom) {
        if (left)
            return dist(rect[0], rect[1], q[0], q[1]);
        if (right)
            return dist(rect[2], rect[1], q[0], q[1]);
        return (rect[1] - q[1]) * (rect[1] - q[1]);
    }
    if (left)
        return (rect[0] - q[0]) * (rect[0] - q[0]);
    if (right)
        return (q[0] - rect[2]) * (q[0] - rect[2]);
    return 0;
}

bool Node::overlap(array<float, 4> r) const {
    for (int i = 0; i < D; i++)
        if (rect[i] > r[i + D] || r[i] > rect[i + D])
            return false;
    return true;
}

array<float, 4> Node::getRect(array<float, 2> p) {
    array<float, 4> r = rect;
    if (r[0] > p[0])
        r[0] = p[0];
    if (r[1] > p[1])
        r[1] = p[1];
    if (r[2] < p[0])
        r[2] = p[0];
    if (r[3] < p[1])
        r[3] = p[1];
    return r;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Ledger Methods
/////////////////////////////////////////////////////////////////////////////////////////

void Node::Ledger::upRead() {
    double rg = (gap > 0) ? gap : 0;
    reads = reads * (1 - rg / (rg + trendCoeff)) + 1;
    if (gap < 0)
        gap--;
    else
        gap = -1;
}

void Node::Ledger::upWrite() {
    double wg = (gap < 0) ? -gap : 0;
    writes = writes * (1 - (wg / (wg + trendCoeff))) + 1;
    if (gap > 0)
        gap++;
    else
        gap = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Node Methods
/////////////////////////////////////////////////////////////////////////////////////////

void Node::fission(Node *parent) {
    long splitPos = pageCap * floor(ceil(points->size() / float(pageCap)) / 2);
    vector<Node *> pages = splitPage(parent, splitPos);
    for (auto page : pages) {
        if (page->points->size() > pageCap) {
            page->fission(parent);
            delete page;
        } else
            parent->contents->emplace_back(page);
    }
}

void Node::fusion(Node *parent) {
    vector<Node *> directories = splitDirectory(parent);
    for (auto directory : directories) {
        if (directory->contents->size() > directoryCap) {
            directory->fusion(parent);
            delete directory;
        } else
            parent->contents->emplace_back(directory);
    }
}

Info Node::insertPt(Record pt) {
    auto getArea = [](array<float, 4> r) { return (r[3] - r[1]) * (r[2] - r[0]); };
    Info info;
    int key = -1;

    // Update MBRs.
    double area, newArea, minDiff = numeric_limits<float>::max();
    array<float, 4> newRect, minRect;
    for (uint i = 0; i < contents->size(); i++) {
        Node *cn = contents.value()[i];
        newRect = cn->getRect(pt.data);
        area = getArea(cn->rect);
        newArea = getArea(newRect);
        if (double diff = newArea - area; minDiff > diff) {
            minDiff = diff;
            minRect = newRect;
            key = i;
            if (diff == 0)
                break;
        }
    }
    Node *cn = contents.value()[key];
    if (minDiff > 0)
        cn->rect = minRect;

    // Check for node type and proceed.
    vector<Node *> newNodes;
    if (cn->points) {
        cn->points->emplace_back(pt);
        info = Info{0, 1, 0, 1};
        if (cn->points->size() > pageCap) {
            newNodes = cn->splitPage(this, cn->points->size() / 2);
            info.writes = 2;
            info.pages = 1;
        }
    } else {
        info = cn->insertPt(pt);
        if (cn->contents->size() > directoryCap)
            newNodes = cn->splitDirectory(this);
    }

    // Update the node contents if any new nodes (overflow)
    if (!newNodes.empty()) {
        contents->erase(contents->begin() + key);
        delete cn;
        for (auto node : newNodes)
            contents->emplace_back(node);
    }
    ledger->pages += info.pages;
    ledger->points++;
    ledger->upWrite();
    return info;
}

array<long, 2> Node::getInfo() const {
    if (points)
        return {1, long(points->size())};
    long numPages = 0;
    long numPoints = 0;
    for (auto node : contents.value()) {
        array temp = node->getInfo();
        numPages += temp[0];
        numPoints += temp[1];
    }
    return {numPages, numPoints};
}

Info Node::rangeSearch(array<float, 4> query) {
    Info info;
    if (points) {
        info.reads = 1;
        // info.points = scan(query);
    } else {
        for (auto cn : contents.value()) {
            if (cn->overlap(query))
                info += cn->rangeSearch(query);
        }
        ledger->pages += info.pages;
        info += refresh();
    }
    return info;
}

Info Node::refresh() {
    ledger->upRead();
    float fat = (ledger->pages / ceil(ledger->points / float(pageCap))) - 1;
    float writeTrend = 1 - abs(ledger->gap) / (abs(ledger->gap) + trendCoeff);
    float tolerance = ledger->writes / (ledger->writes + ledger->reads / writeTrend);
    // trace(fat, tolerance);
    if (fat > tolerance && height == 1) {
        int numPages = ledger->pages;
        unbind();
        contents->clear();
        splits->clear();
        fission(this);
        height = 1;
        ledger->pages = contents->size();
        points->clear();
        points.reset();
        while (contents->size() > directoryCap) {
            fusion(this);
            height++;
        }
        return Info{ledger->pages - numPages, 0, 0, numPages};
    }
    return Info();
}

inline bool overlaps(array<float, 4> r, array<float, 2> p) {
    for (int i = 0; i < D; i++) {
        if (r[i] > p[i] || p[i] > r[i + D])
            return false;
    }
    return true;
}

int Node::scan(array<float, 4> query) const {
    int totalPoints = 0;
    if (inside(query))
        return points->size();
    for (auto p : points.value())
        if (overlaps(query, p.data))
            totalPoints++;
    return totalPoints;
}

int Node::size() const {
    int rectSize = sizeof(float) * 4;
    int typeSize = 0;
    if (points)
        typeSize = sizeof(vector<array<float, 2>>);
    else
        typeSize = sizeof(vector<Node *>) + contents->size() * sizeof(Node *) +
                   sizeof(vector<Split *>) + splits->size() * (sizeof(Split *) + sizeof(Split));
    int totalSize = typeSize + rectSize;
    return totalSize;
}

vector<Node *> Node::splitDirectory(Node *pn) {
    vector<Node *> dirs = {new Node(), new Node()};
    Split bestSplit = splits.value()[0];
    for (auto dir : dirs) {
        dir->height = height;
        dir->contents = vector<Node *>();
        dir->splits = vector<Split>();
    }

    // cerr << "Make bounding rectangles" << endl;
    for (auto cn : contents.value()) {
        bool side = cn->getCenter()[bestSplit.axis] > bestSplit.pt[bestSplit.axis];
        dirs[side]->contents->emplace_back(cn);
        if (dirs[side]->rect[0] > cn->rect[0])
            dirs[side]->rect[0] = cn->rect[0];
        if (dirs[side]->rect[1] > cn->rect[1])
            dirs[side]->rect[1] = cn->rect[1];
        if (dirs[side]->rect[2] < cn->rect[2])
            dirs[side]->rect[2] = cn->rect[2];
        if (dirs[side]->rect[3] < cn->rect[3])
            dirs[side]->rect[3] = cn->rect[3];
    }

    // cerr << "Distribute splits" << endl;
    for (auto isplit = next(splits->begin()); isplit != splits->end(); isplit++)
        dirs[(*isplit).pt[bestSplit.axis] > bestSplit.pt[bestSplit.axis]]->splits->emplace_back(
            *isplit);

    for (auto dir : dirs) {
        array temp = dir->getInfo();
        dir->ledger = Ledger();
        dir->ledger->pages = temp[0];
        dir->ledger->points = temp[1];
        // NOTE: Using simple heuristic for now. Refine it later.
        dir->ledger->reads = ledger->reads / 2;
        dir->ledger->writes = ledger->writes / 2;
    }

    contents->clear();
    splits->clear();
    pn->splits->emplace_back(bestSplit);
    return dirs;
}

vector<Node *> Node::splitPage(Node *pn, long splitPos) {
    bool axis = (rect[2] - rect[0]) < (rect[3] - rect[1]);
    sort(all(points.value()),
        [axis](const Record &l, const Record &r) { return l.data[axis] < r.data[axis]; });
    Split newSplit = Split();
    newSplit.axis = axis;
    newSplit.pt[axis] = points.value()[splitPos].data[axis];
    newSplit.pt[!axis] = getCenter()[!axis];

    // cerr << "Create new pages" << endl;
    vector<Node *> pages = {new Node(), new Node()};

    // Splitting points
    pages[0]->points = vector<Record>(points->begin(), points->begin() + splitPos);
    pages[1]->points = vector<Record>(points->begin() + splitPos, points->end());

    // cerr << "Make bounding rectangles" << endl;
    for (auto page : pages) {
        for (auto p : page->points.value()) {
            if (page->rect[0] > p.data[0])
                page->rect[0] = p.data[0];
            if (page->rect[1] > p.data[1])
                page->rect[1] = p.data[1];
            if (page->rect[2] < p.data[0])
                page->rect[2] = p.data[0];
            if (page->rect[3] < p.data[1])
                page->rect[3] = p.data[1];
        }
    }

    pn->splits->emplace_back(newSplit);
    return pages;
}

void Node::unbind() {
    points = vector<Record>();
    for (auto node : contents.value()) {
        if (node->contents)
            node->unbind();
        points->insert(points->end(), all(node->points.value()));
        delete node;
    }
}

Node::~Node() {
    if (points) {
        points->clear();
        points.reset();
    } else {
        contents->clear();
        contents.reset();
        ledger.reset();
        splits->clear();
        splits.reset();
    }
}
