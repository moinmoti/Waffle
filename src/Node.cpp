#include "Node.h"

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
// Node Methods
/////////////////////////////////////////////////////////////////////////////////////////

void Node::fission(Node *parent, int pageCap) {
    long splitPos = pageCap * floor(ceil(points->size() / float(pageCap)) / 2);
    vector<Node *> pages = splitPage(parent, splitPos);
    for (auto page : pages) {
        if (page->points->size() > pageCap) {
            page->fission(parent, pageCap);
            delete page;
        } else
            parent->contents->emplace_back(page);
    }
}

void Node::fusion(Node *parent, int directoryCap) {
    vector<Node *> directories = splitDirectory(parent);
    for (auto directory : directories) {
        if (directory->contents->size() > directoryCap) {
            directory->fusion(parent, directoryCap);
            delete directory;
        } else {
            // if constexpr (TOLERANCE < 1)
            directory->numPoints = directory->pointCount();
            parent->contents->emplace_back(directory);
        }
    }
}

void Node::insertPt(array<float, 2> pt, int pageCap, int directoryCap) {
    auto getArea = [](array<float, 4> r) { return (r[3] - r[1]) * (r[2] - r[0]); };
    // if constexpr (TOLERANCE < 1)
    numPoints = numPoints.value() + 1;
    int key = -1;
    double area, newArea, minDiff = numeric_limits<float>::max();
    array<float, 4> newRect, minRect;
    for (uint i = 0; i < contents->size(); i++) {
        Node *cn = contents.value()[i];
        newRect = cn->getRect(pt);
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
    long P = 0;
    if (cn->points) {
        cn->points->emplace_back(pt);
        if (cn->points->size() > pageCap) {
            // if constexpr (TOLERANCE < 1)
            P = contents->size() + 1;
            /* else {
                vector<Node *> newNodes;
                newNodes = cn->splitPage(this, cn->points->size() / 2);
                contents->erase(contents->begin() + key);
                delete cn;
                for (auto node : newNodes)
                    contents->emplace_back(node);
            } */
        }
    } else {
        cn->insertPt(pt, pageCap, directoryCap);
        if (cn->contents->size() > directoryCap) {
            // if constexpr (TOLERANCE < 1)
            P = pageCount();
            /* else {
                vector<Node *> newNodes;
                newNodes = cn->splitDirectory(this);
                contents->erase(contents->begin() + key);
                delete cn;
                for (auto node : newNodes)
                    contents->emplace_back(node);
            } */
        }
    }
    if (P != 0) {
        if (float fat = (P / ceil(numPoints.value() / float(pageCap))) - 1; fat > TOLERANCE) {
            int targetHeight = unbind();
            contents->clear();
            splits->clear();
            fission(this, pageCap);
            height = 1;
            numPoints = points->size();
            points->clear();
            points.reset();
            while (height < targetHeight) {
                fusion(this, directoryCap);
                height++;
            }
        } else {
            vector<Node *> newNodes;
            if (cn->points)
                newNodes = cn->splitPage(this, cn->points->size() / 2);
            else {
                newNodes = cn->splitDirectory(this);
                for (auto dir : newNodes)
                    dir->numPoints = dir->pointCount();
            }
            contents->erase(contents->begin() + key);
            delete cn;
            for (auto node : newNodes)
                contents->emplace_back(node);
        }
    }
}

long Node::pageCount() const {
    if (points)
        return 1;
    long totalPages = 0;
    for (auto node : contents.value())
        totalPages += node->pageCount();
    return totalPages;
}

long Node::pointCount() const {
    if (points)
        return points->size();
    long totalPoints = 0;
    for (auto node : contents.value())
        totalPoints += node->pointCount();
    return totalPoints;
}

int Node::rangeSearch(array<float, 4> query, map<string, double> &stats) {
    int totalPoints = 0;
    if (points) {
        stats["io"]++;
        // totalPoints += scan(query);
    } else {
        for (auto cn : contents.value())
            if (cn->overlap(query))
                totalPoints += cn->rangeSearch(query, stats);
    }
    return totalPoints;
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
        if (overlaps(query, p))
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
    vector<Node *> nodes = {new Node(), new Node()};
    Split bestSplit = splits.value()[0];
    for (auto node : nodes) {
        node->height = height;
        node->contents = vector<Node *>();
        node->splits = vector<Split>();
    }

    // cerr << "Make bounding rectangles" << endl;
    for (auto cn : contents.value()) {
        bool side = cn->getCenter()[bestSplit.axis] > bestSplit.pt[bestSplit.axis];
        nodes[side]->contents->emplace_back(cn);
        if (nodes[side]->rect[0] > cn->rect[0])
            nodes[side]->rect[0] = cn->rect[0];
        if (nodes[side]->rect[1] > cn->rect[1])
            nodes[side]->rect[1] = cn->rect[1];
        if (nodes[side]->rect[2] < cn->rect[2])
            nodes[side]->rect[2] = cn->rect[2];
        if (nodes[side]->rect[3] < cn->rect[3])
            nodes[side]->rect[3] = cn->rect[3];
    }

    // cerr << "Distribute splits" << endl;
    for (auto isplit = next(splits->begin()); isplit != splits->end(); isplit++)
        nodes[(*isplit).pt[bestSplit.axis] > bestSplit.pt[bestSplit.axis]]->splits->emplace_back(
            *isplit);

    contents->clear();
    splits->clear();
    pn->splits->emplace_back(bestSplit);
    return nodes;
}

vector<Node *> Node::splitPage(Node *pn, long splitPos) {
    bool axis = (rect[2] - rect[0]) < (rect[3] - rect[1]);
    sort(all(points.value()),
         [axis](const array<float, 2> &l, const array<float, 2> &r) { return l[axis] < r[axis]; });
    Split newSplit = Split();
    newSplit.axis = axis;
    newSplit.pt[axis] = points.value()[splitPos][axis];
    newSplit.pt[!axis] = getCenter()[!axis];

    // cerr << "Create new pages" << endl;
    vector<Node *> pages = {new Node(), new Node()};

    // Splitting points
    pages[0]->points = vector<array<float, 2>>(points->begin(), points->begin() + splitPos);
    pages[1]->points = vector<array<float, 2>>(points->begin() + splitPos, points->end());

    // cerr << "Make bounding rectangles" << endl;
    for (auto page : pages) {
        for (auto p : page->points.value()) {
            if (page->rect[0] > p[0])
                page->rect[0] = p[0];
            if (page->rect[1] > p[1])
                page->rect[1] = p[1];
            if (page->rect[2] < p[0])
                page->rect[2] = p[0];
            if (page->rect[3] < p[1])
                page->rect[3] = p[1];
        }
    }

    pn->splits->emplace_back(newSplit);
    return pages;
}

int Node::unbind() {
    int height = 1;
    points = vector<array<float, 2>>();
    for (auto node : contents.value()) {
        if (node->contents)
            height = node->unbind() + 1;
        points->insert(points->end(), all(node->points.value()));
        delete node;
    }
    return height;
}

Node::~Node() {
    if (points) {
        points->clear();
        points.reset();
    } else {
        splits->clear();
        splits.reset();
        contents->clear();
        contents.reset();
        numPoints.reset();
    }
}
