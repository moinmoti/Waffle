#include "PageMin.h"
#include <bits/stdc++.h>
#include <chrono>

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
    cerr.write(names, comma - names) << " : " << arg1 << " | ";
    __f(comma + 1, args...);
}
#else
#define trace(...)
#endif

void createQuerySet(string fileName, vector<tuple<char, vector<float>, float>> &queryArray) {
    cout << "Begin query creation for PageMin" << endl;
    string line;
    int i = 0;

    ifstream file(fileName);
    if (file.is_open()) {
        // getline(file, line); // Skip the header line
        while (getline(file, line)) {
            char type = line[line.find_first_not_of(" ")];
            vector<float> q;
            line = line.substr(line.find_first_of(type) + 1);
            const char *cs = line.c_str();
            char *end;
            int params = (type == 'r') ? 4 : 2;
            for (uint d = 0; d < params; d++) {
                q.emplace_back(strtof(cs, &end));
                cs = end;
            }
            float info = strtof(cs, &end);
            queryArray.emplace_back(make_tuple(type, q, info));
            i++;
        }
        file.close();
    }
    cout << "Finish query creation for PageMin" << endl;
}

void knnQuery(tuple<char, vector<float>, float> q, PageMin *index, map<string, double> &knnLog) {
    array<float, 2> p;
    for (uint i = 0; i < p.size(); i++)
        p[i] = get<1>(q)[i];
    int k = get<2>(q);

    // cerr << "Points: " << p[0] << " | " << p[1] << endl;

    map<string, double> stats;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    index->kNNQuery(p, stats, k);
    knnLog["knn_total " + to_string(k)] +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
    // knnLog["scan " + to_string(k)] += stats["scan"];
    knnLog["io " + to_string(k)] += stats["io"];
    /* knnLog["explore " + to_string(k)] += stats["explore"];
    knnLog["scanned " + to_string(k)] += stats["scanCount"];
    knnLog["heapAccess " + to_string(k)] += stats["heapAccess"]; */
    knnLog["count " + to_string(k)]++;
}

void rangeQuery(tuple<char, vector<float>, float> q, PageMin *index, array<float, 4> boundary,
                map<string, double> &rangeLog) {
    array<float, 4> query;
    for (uint i = 0; i < query.size(); i++)
        query[i] = get<1>(q)[i];
    float rs = get<2>(q);

    map<string, double> stats;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    index->rangeQuery(query, stats);
    rangeLog["total " + to_string(rs)] +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
    // rangeLog["scan " + to_string(rs)] += stats["scanTime"];
    rangeLog["io " + to_string(rs)] += stats["io"];
    rangeLog["count " + to_string(rs)]++;
}

void insertQuery(tuple<char, vector<float>, float> q, PageMin *index,
                 map<string, double> &insertLog) {
    array<float, 2> p;
    for (uint i = 0; i < p.size(); i++)
        p[i] = get<1>(q)[i];
    int id = get<2>(q);

    map<string, double> stats;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    index->insertQuery(p, stats);
    insertLog["total"] +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
}

void deleteQuery(tuple<char, vector<float>, float> q, PageMin *index,
                 map<string, double> &deleteLog) {
    array<float, 2> p;
    for (uint i = 0; i < p.size(); i++)
        p[i] = get<1>(q)[i];
    int id = get<2>(q);

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    map<string, double> stats;
    index->deleteQuery(p, stats);
    deleteLog["total"] +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
}

void evaluate(PageMin *index, vector<tuple<char, vector<float>, float>> queryArray,
              array<float, 4> boundary, string logFile) {
    map<string, double> deleteLog, insertLog, rangeLog, knnLog;

    cout << "Begin Querying..." << endl;
    for (auto q : queryArray) {
        if (get<0>(q) == 'k') {
            knnQuery(q, index, knnLog);
            knnLog["count"]++;
            // trace(knnLog["count"]);
        } else if (get<0>(q) == 'r') {
            rangeQuery(q, index, boundary, rangeLog);
            rangeLog["count"]++;
            // trace(rangeLog["count"]);
        } else if (get<0>(q) == 'i') {
            insertQuery(q, index, insertLog);
            insertLog["count"]++;
            if (long(insertLog["count"]) % long(1e6) == 0)
                trace(insertLog["count"]);
        } else if (get<0>(q) == 'd') {
            deleteQuery(q, index, deleteLog);
            deleteLog["count"]++;
            // trace(deleteLog["count"]);
        } else
            cerr << "Invalid Query!!!" << endl;
        // cerr << endl;
    }
    cout << "Finish Querying..." << endl;

    ofstream log;
    log.open(logFile, ios_base::app);
    if (!log.is_open())
        cerr << "Unable to open log.txt";

    log << "------------------Results-------------------" << endl;

    log << "------------------Range Queries-------------------" << endl;
    for (auto it = rangeLog.cbegin(); it != rangeLog.cend(); ++it)
        log << it->first << ": " << it->second << endl;

    log << "------------------KNN Queries-------------------" << endl;
    for (auto it = knnLog.cbegin(); it != knnLog.cend(); ++it)
        log << it->first << ": " << it->second << endl;

    log << "------------------Insert Queries-------------------" << endl;
    for (auto it = insertLog.cbegin(); it != insertLog.cend(); ++it)
        log << it->first << ": " << it->second << endl;

    log << "------------------Delete Queries-------------------" << endl;
    for (auto it = deleteLog.cbegin(); it != deleteLog.cend(); ++it)
        log << it->first << ": " << it->second << endl;

    log << endl << "************************************************" << endl;
    map<string, double> stats;
    float indexSize = index->size(stats);
    log << "PageMin size in MB: " << float(indexSize / 1e6) << endl;
    // index.snapshot();
    log << "No. of pages: " << stats["pages"] << endl;
    log << "No. of directories: " << stats["directories"] << endl;

    log.close();
}

int main(int argCount, char **args) {
    map<string, string> config;
    string projectPath = string(args[1]);
    string queryType = string(args[2]);
    int directoryCap = stoi(string(args[3]));
    int pageCap = stoi(string(args[4]));
    long insertions = 1e7;
    long limit = 1e8 - insertions;
    /* string sign = "-I1e" + to_string(int(log10(insertions))) + "-" + to_string(directoryCap) +
       "-" + to_string(pageCap); */
    string sign = "-1e8-" + to_string(directoryCap) + "-" + to_string(pageCap);

    string expPath = projectPath + "/Experiments/";
    string prefix = expPath + queryType + "/";
    string queryFile = projectPath + "/data/ships-dinos/Queries/" + queryType;
    string dataFile = projectPath + "/data/ships-dinos/ships1e8.txt";
    int offset = 0;
    array<float, 4> boundary{-180.0, -90.0, 180.0, 90.0};

    cout << "---Generation--- " << endl;

    string logFile = prefix + "log" + sign + ".txt";
    ofstream log(logFile);
    if (!log.is_open())
        cout << "Unable to open log.txt";
    high_resolution_clock::time_point start = high_resolution_clock::now();
    cout << "Defining PageMin..." << endl;
    PageMin index = PageMin(pageCap, directoryCap, boundary);
    cout << "Bulkloading PageMin..." << endl;
    index.bulkload(dataFile, limit);
    double hTreeCreationTime =
        duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    log << "PageMin Creation Time: " << hTreeCreationTime << endl;
    log << "Directory Capacity: " << directoryCap << endl;
    log << "Page Capacity: " << pageCap << endl;
    map<string, double> stats;
    float indexSize = index.size(stats);
    log << "PageMin size in MB: " << float(indexSize / 1e6) << endl;
    // index.snapshot();
    log << "No. of pages: " << stats["pages"] << endl;
    log << "No. of directories: " << stats["directories"] << endl;

    vector<tuple<char, vector<float>, float>> queryArray;
    createQuerySet(queryFile, queryArray);

    cout << "---Evaluation--- " << endl;
    evaluate(&index, queryArray, boundary, logFile);
    return 0;
}
