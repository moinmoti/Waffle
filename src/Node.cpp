#include "Node.h"

#define TRACE
#ifdef TRACE
#define trace(...) __f(#__VA_ARGS__, __VA_ARGS__)
template <typename Arg1> void __f(const char *name, Arg1 &&arg1) {
    cerr << name << " : " << arg1 << endl;
}
template <typename Arg1, typename... Args>
void __f(const char *names, Arg1 &&arg1, Args &&... args) {
    const char *comma = strchr(names + 1, ',');
    cerr.write(names, comma - names) << " : " << arg1 << " | ";
    __f(comma + 1, args...);
}
#else
#define trace(...)
#endif

#define all(c) c.begin(), c.end()
#define dist(x1, y1, x2, y2) (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)
#define distManhattan(x1, y1, x2, y2) std::abs(x1 - x2) + std::abs(y1 - y2)
#define oppDir(d) (d + DIMS) % (DIMS * 2)

#define DIMS 2
#define TOLERANCE 0.1
#define V 0
#define H 1

/////////////////////////////////////////////////////////////////////////////////////////
// Rectangle Methods
/////////////////////////////////////////////////////////////////////////////////////////

void printNode(string str, array<float, 4> r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

bool Node::overlap(array<float, 4> r) const {
    for (int i = 0; i < DIMS; i++)
        if (rect[i] > r[i + DIMS] || r[i] > rect[i + DIMS])
            return false;
    return true;
}

bool Node::containsPt(array<float, 2> p) const {
    bool result = true;
    for (int i = 0; i < DIMS; i++)
        result = result & (rect[i] <= p[i]) & (rect[i + DIMS] >= p[i]);
    return result;
}

bool Node::inside(array<float, 4> r) const {
    bool result = true;
    for (int i = 0; i < DIMS; i++)
        result = result & (rect[i] >= r[i]) & (rect[i + DIMS] <= r[i + DIMS]);
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
        } else
            parent->contents->emplace_back(directory);
    }
}

void Node::insertPt(array<float, 2> p, int pageCap, int directoryCap) {
    for (uint i = 0; i < contents->size(); i++) {
        if (Node *cn = contents.value()[i]; cn->containsPt(p)) {
            long P = 0;
            if (cn->points) {
                cn->points->emplace_back(p);
                if (cn->points->size() > pageCap)
                    P = pageCount() + 1;
            } else {
                cn->insertPt(p, pageCap, directoryCap);
                if (cn->contents->size() > directoryCap)
                    P = pageCount();
            }
            if (P != 0) {
                long N = pointCount();
                if (float fat = (P / ceil(N / float(pageCap))) - 1; fat > TOLERANCE) {
                    int targetHeight = unbind();
                    contents->clear();
                    splits->clear();
                    fission(this, pageCap);
                    height = 1;
                    points->clear();
                    points.reset();
                    while (height < targetHeight) {
                        fusion(this, directoryCap);
                        height++;
                    }
                    /* N = pointCount();
                    P = pageCount();
                    float newFat = (P / ceil(N / float(pageCap))) - 1; */
                } else {
                    vector<Node *> newNodes;
                    if (cn->points)
                        newNodes = cn->splitPage(this, cn->points->size() / 2);
                    else
                        newNodes = cn->splitDirectory(this);
                    contents->erase(contents->begin() + i);
                    delete cn;
                    for (auto node : newNodes)
                        contents->emplace_back(node);
                }
            }
            break;
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
        totalPoints += scan(query);
    } else {
        for (auto cn : contents.value())
            if (cn->overlap(query))
                totalPoints += cn->rangeSearch(query, stats);
    }
    return totalPoints;
}

inline bool overlaps(array<float, 4> r, array<float, 2> p) {
    for (int i = 0; i < DIMS; i++) {
        if (r[i] > p[i] || p[i] > r[i + DIMS])
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
    vector<Node *> nodes(2);
    Split bestSplit = splits.value()[0];
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i] = new Node();
        nodes[i]->height = height;
        nodes[i]->rect = rect;
        nodes[i]->rect[bestSplit.axis + !i * DIMS] = bestSplit.pt[bestSplit.axis];
        nodes[i]->contents = vector<Node *>();
        // nodes[i]->contents->reserve(contents->size());
        nodes[i]->splits = vector<Split>();
    }
    for (auto cn : contents.value())
        nodes[cn->getCenter()[bestSplit.axis] > bestSplit.pt[bestSplit.axis]]
            ->contents->emplace_back(cn);
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
    vector<Node *> pages(2);
    for (int i = 0; i < pages.size(); i++) {
        pages[i] = new Node();
        pages[i]->height = 0;
        pages[i]->rect = rect;
        pages[i]->rect[newSplit.axis + !i * DIMS] = newSplit.pt[newSplit.axis];
    }

    // Splitting points
    pages[0]->points = vector<array<float, 2>>(points->begin(), points->begin() + splitPos);
    pages[1]->points = vector<array<float, 2>>(points->begin() + splitPos, points->end());

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
    }
}
