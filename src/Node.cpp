#include "Node.h"

void printNode(string str, Rect r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

bool Node::containsPt(const Point &p) const {
    bool result = true;
    for (int i = 0; i < D; i++)
        result = result & (rect[i] <= p[i]) & (rect[i + D] >= p[i]);
    return result;
}

bool Node::inside(const Rect &r) const {
    bool result = true;
    for (int i = 0; i < D; i++)
        result = result & (rect[i] >= r[i]) & (rect[i + D] <= r[i + D]);
    return result;
}

Point Node::getCenter() const {
    Point pt;
    for (uint d = 0; d < D; d++)
        pt[d] = (rect[d] + rect[d + D]) / 2;
    return pt;
}

double Node::minSqrDist(const Rect &r) const {
    double sqrDist = 0;
    for (uint d = 0; d < D; d++)
        sqrDist += pow(max({0.f, rect[d + D] - r[d], r[d + D] - rect[d]}), 2);
    return sqrDist;
    /* bool left = r[2] < rect[0];
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
    return 0; */
}

double Node::minSqrDist(const Point &q) const {
    double sqrDist = 0;
    for (uint d = 0; d < D; d++)
        sqrDist += pow(max({0.f, rect[d + D] - q[d], q[d] - rect[d]}), 2);
    return sqrDist;
    /* bool left = q[0] < rect[0];
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
    return 0; */
}

bool Node::overlap(const Rect &r) const {
    for (int i = 0; i < D; i++)
        if (rect[i] > r[i + D] || r[i] > rect[i + D])
            return false;
    return true;
}

Rect Node::getRect(const Rect &otherRect) const {
    Rect r = rect;
    for (uint d = 0; d < D; d++) {
        if (r[d] > otherRect[d])
            r[d] = otherRect[d];
        if (r[d + D] < otherRect[d + D])
            r[d + D] = otherRect[d + D];
    }
    return r;
}

Rect Node::getRect(const Point &p) const {
    Rect r{p[0], p[1], p[0], p[1]};
    return getRect(r);
}
