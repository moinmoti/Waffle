#include "MPT.h"

void createQuerySet(string fileName, vector<tuple<char, vector<float>, float>> &queryArray) {
    cout << "Begin query creation for MPT" << endl;
    string line;

    ifstream file(fileName);
    if (file.is_open()) {
        // getline(file, line); // Skip the header line
        while (getline(file, line)) {
            char type = line[line.find_first_not_of(" ")];
            vector<float> q;
            if (type == 'l') {
                queryArray.emplace_back(make_tuple(type, q, 0));
            } else {
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
            }
        }
        file.close();
    }
    cout << "Finish query creation for MPT" << endl;
}

void knnQuery(tuple<char, vector<float>, float> q, MPT *index, map<string, double> &knnLog) {
    array<float, 2> p;
    for (uint i = 0; i < p.size(); i++)
        p[i] = get<1>(q)[i];
    int k = get<2>(q);

    // cerr << "Points: " << p[0] << " | " << p[1] << endl;

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    Info stats = index->kNNQuery(p, k);
    knnLog["knn_total " + to_string(k)] +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
    knnLog["io " + to_string(k)] += stats.reads;
    knnLog["count " + to_string(k)]++;
}

void rangeQuery(tuple<char, vector<float>, float> q, MPT *index, map<string, double> &rangeLog) {
    array<float, 4> query;
    for (uint i = 0; i < query.size(); i++)
        query[i] = get<1>(q)[i];
    float rs = get<2>(q);

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    Info stats = index->rangeQuery(query);
    rangeLog["total " + to_string(rs)] +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
    rangeLog["io " + to_string(rs)] += stats.reads;
    rangeLog["count " + to_string(rs)]++;
}

void insertQuery(tuple<char, vector<float>, float> q, MPT *index, map<string, double> &insertLog) {
    Record p;
    for (uint i = 0; i < p.data.size(); i++)
        p.data[i] = get<1>(q)[i];
    p.id = get<2>(q);

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    Info stats = index->insertQuery(p);
    insertLog["total"] +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
}

void deleteQuery(tuple<char, vector<float>, float> q, MPT *index, map<string, double> &deleteLog) {
    Record p;
    for (uint i = 0; i < p.data.size(); i++)
        p.data[i] = get<1>(q)[i];
    p.id = get<2>(q);

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    Info stats = index->deleteQuery(p);
    deleteLog["total"] +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
}

void evaluate(MPT *index, vector<tuple<char, vector<float>, float>> queryArray, string logFile) {
    map<string, double> deleteLog, insertLog, rangeLog, knnLog;

    cout << "Begin Querying..." << endl;
    for (auto q : queryArray) {
        if (get<0>(q) == 'k') {
            knnQuery(q, index, knnLog);
            knnLog["count"]++;
            // trace(knnLog["count"]);
        } else if (get<0>(q) == 'r') {
            rangeQuery(q, index, rangeLog);
            rangeLog["count"]++;
            // trace(rangeLog["count"]);
        } else if (get<0>(q) == 'i') {
            insertQuery(q, index, insertLog);
            insertLog["count"]++;
            /* if (long(insertLog["count"]) % long(1e6) == 0)
                trace(insertLog["count"]); */
        } else if (get<0>(q) == 'd') {
            deleteQuery(q, index, deleteLog);
            deleteLog["count"]++;
            // trace(deleteLog["count"]);
        } else if (get<0>(q) == 'l') {
            ofstream log;
            log.open(logFile, ios_base::app);
            if (!log.is_open())
                cerr << "Unable to open log.txt";

            log << "------------------Results-------------------" << endl;

            log << "------------------Range Queries-------------------" << endl;
            for (auto &l : rangeLog) {
                log << l.first << ":\t" << l.second << endl;
                l.second = 0;
            }

            log << "------------------KNN Queries-------------------" << endl;
            for (auto &l : knnLog) {
                log << l.first << ":\t" << l.second << endl;
                l.second = 0;
            }

            /* log << "------------------Delete Queries-------------------" << endl;
            for (auto &l : deleteLog) {
                log << l.first << ":\t" << l.second << endl;
                l.second = 0;
            } */

            log << "------------------Insert Queries-------------------" << endl;
            for (auto &l : insertLog) {
                log << l.first << ":\t" << l.second << endl;
                l.second = 0;
            }

            map<string, double> stats;
            float indexSize = index->size(stats);
            log << "MPT size in MB: " << float(indexSize / 1e6) << endl;
            // index->snapshot();
            log << "No. of pages: " << stats["pages"] << endl;
            log << "No. of directories: " << stats["directories"] << endl;
            log << "Tolerance: " << TOLERANCE << endl;

            log << endl << "************************************************" << endl;

            log.close();
        } else
            cerr << "Invalid Query!!!" << endl;
        // cerr << endl;
    }
    cout << "Finish Querying..." << endl;
    index->snapshot();
}

int main(int argCount, char **args) {
    map<string, string> config;
    string projectPath = string(args[1]);
    string queryType = string(args[2]);
    int directoryCap = stoi(string(args[3]));
    int pageCap = stoi(string(args[4]));
    long limit = 1e7;
    string sign = "-1e7-" + to_string(directoryCap);
    // sign += "-T" + to_string(int(100 * TOLERANCE));

    string expPath = projectPath + "/Experiments/";
    string prefix = expPath + queryType + "/";
    string queryFile = projectPath + "/data/ships-dinos/Queries/" + queryType;
    string dataFile = projectPath + "/data/ships-dinos/ships1e8.txt";
    /* string queryFile = projectPath + "/data/OSM-USA/" + queryType;
    string dataFile = projectPath + "/data/OSM-USA/osm-usa-10mil"; */
    /* string queryFile = projectPath + "/data/NewYorkTaxi/" + queryType;
    string dataFile = projectPath + "/data/NewYorkTaxi/taxiNY"; */
    int offset = 0;

    cout << "---Generation--- " << endl;

    string logFile = prefix + "log" + sign + ".txt";
    ofstream log(logFile);
    if (!log.is_open())
        cout << "Unable to open log.txt";
    high_resolution_clock::time_point start = high_resolution_clock::now();
    cout << "Defining MPT..." << endl;
    MPT index = MPT(directoryCap, pageCap);
    cout << "Bulkloading MPT..." << endl;
    index.bulkload(dataFile, limit);
    double hTreeCreationTime =
        duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    log << "MPT Creation Time: " << hTreeCreationTime << endl;
    log << "Directory Capacity: " << directoryCap << endl;
    log << "Page Capacity: " << pageCap << endl;
    map<string, double> stats;
    float indexSize = index.size(stats);
    log << "MPT size in MB: " << float(indexSize / 1e6) << endl;
    // index.snapshot();
    log << "No. of pages: " << stats["pages"] << endl;
    log << "No. of directories: " << stats["directories"] << endl;

    cout << "---Creating query set---" << endl;
    vector<tuple<char, vector<float>, float>> queryArray;
    createQuerySet(queryFile, queryArray);

    cout << "---Evaluation--- " << endl;
    evaluate(&index, queryArray, logFile);
    return 0;
}
