#include "MPT.h"

struct Stats {
    struct StatType {
        long count = 0;
        long io = 0;
        // long time = 0;
    };

    StatType del;
    StatType insert;
    map<int, StatType> knn;
    map<float, StatType> range;
    StatType reload;
};

void knnQuery(tuple<char, vector<float>, float> q, MPT *index, Stats &stats) {
    array<float, 2> p;
    for (uint i = 0; i < p.size(); i++)
        p[i] = get<1>(q)[i];
    int k = get<2>(q);
    Info info = index->kNNQuery(p, k);
    stats.knn[k].io += info.reads;
    if (info.writes > 0) {
        stats.reload.count++;
        stats.reload.io += info.writes;
    }
    stats.knn[k].count++;
}

void rangeQuery(tuple<char, vector<float>, float> q, MPT *index, Stats &stats) {
    array<float, 4> query;
    for (uint i = 0; i < query.size(); i++)
        query[i] = get<1>(q)[i];
    float rs = get<2>(q);

    Info info = index->rangeQuery(query);
    stats.range[rs].io += info.reads;
    if (info.writes > 0) {
        stats.reload.count++;
        stats.reload.io += info.writes;
    }
    stats.range[rs].count++;
}

void insertQuery(tuple<char, vector<float>, float> q, MPT *index, Stats &stats) {
    Record p;
    for (uint i = 0; i < p.data.size(); i++)
        p.data[i] = get<1>(q)[i];
    p.id = get<2>(q);
    Info info = index->insertQuery(p);
    stats.insert.io += info.writes;
    stats.insert.count++;
}

void deleteQuery(tuple<char, vector<float>, float> q, MPT *index, Stats &stats) {
    Record p;
    for (uint i = 0; i < p.data.size(); i++)
        p.data[i] = get<1>(q)[i];
    p.id = get<2>(q);
    Info info = index->deleteQuery(p);
    stats.del.count++;
}

void evaluate(MPT *index, string queryFile, string logFile) {
    Stats stats;
    auto roundit = [](float val, int d = 2) { return round(val * pow(10, d)) / pow(10, d); };

    cout << "Begin Querying " << queryFile << endl;
    string line;
    ifstream file(queryFile);
    if (file.is_open()) {
        // getline(file, line); // Skip the header line
        while (getline(file, line)) {
            char type = line[line.find_first_not_of(" ")];
            tuple<char, vector<float>, float> q;
            vector<float> pts;
            if (type == 'l') {
                q = make_tuple(type, pts, 0);
            } else {
                line = line.substr(line.find_first_of(type) + 1);
                const char *cs = line.c_str();
                char *end;
                int params = (type == 'r') ? 4 : 2;
                for (uint d = 0; d < params; d++) {
                    pts.emplace_back(strtof(cs, &end));
                    cs = end;
                }
                float info = strtof(cs, &end);
                q = make_tuple(type, pts, info);
            }
            if (get<0>(q) == 'k') {
                knnQuery(q, index, stats);
            } else if (get<0>(q) == 'r') {
                rangeQuery(q, index, stats);
            } else if (get<0>(q) == 'i') {
                insertQuery(q, index, stats);
            } else if (get<0>(q) == 'd') {
                deleteQuery(q, index, stats);
            } else if (get<0>(q) == 'z') {
                stats = Stats();
            } else if (get<0>(q) == 'l') {
                ofstream log;
                log.open(logFile, ios_base::app);
                if (!log.is_open())
                    cerr << "Unable to open log.txt";

                log << "------------------Results-------------------" << endl << endl;

                log << "------------------Range Queries-------------------" << endl;
                log << setw(8) << "Size" << setw(8) << "Count" << setw(8) << "I/O" << setw(8)
                    << "Time" << endl;
                for (auto &l : stats.range) {
                    log << setw(8) << l.first << setw(8) << l.second.count << setw(8)
                        << roundit(l.second.io / double(l.second.count)) << endl;
                    // << roundit(l.second.time / double(l.second.count)) << endl;
                }

                log << endl << "------------------KNN Queries-------------------" << endl;
                log << setw(8) << "k" << setw(8) << "Count" << setw(8) << "I/O" << setw(8) << "Time"
                    << endl;
                for (auto &l : stats.knn) {
                    log << setw(8) << l.first << setw(8) << l.second.count << setw(8)
                        << roundit(l.second.io / double(l.second.count)) << endl;
                    // << roundit(l.second.time / double(l.second.count)) << endl;
                }

                log << endl << "------------------Insert Queries-------------------" << endl;
                log << "Count:\t" << stats.insert.count << endl;
                log << "I/O:\t" << stats.insert.io / double(stats.insert.count) << endl;
                // log << "Time: \t" << stats.insert.time / double(stats.insert.count) << endl <<
                // endl;

                log << endl << "------------------ Reloading -------------------" << endl;
                log << "Count:\t" << stats.reload.count << endl;
                log << "I/O (overall):\t" << stats.reload.io << endl << endl;

                map<string, double> info;
                float indexSize = index->size(info);
                log << "MPT size in MB: " << float(indexSize / 1e6) << endl;
                log << "No. of pages: " << info["pages"] << endl;
                log << "No. of directories: " << info["directories"] << endl;

                log << endl << "************************************************" << endl;

                log.close();
            } else
                cerr << "Invalid Query!!!" << endl;
            // cerr << endl;
        }
        file.close();
    }
    cout << "Finish Querying..." << endl;
}

int main(int argCount, char **args) {
    map<string, string> config;
    string projectPath = string(args[1]);
    string queryType = string(args[2]);
    int directoryCap = stoi(string(args[3]));
    int pageCap = stoi(string(args[4]));
    // long limit = 1e7;
    string sign = "-" + to_string(directoryCap);
    // sign += "-T" + to_string(int(100 * TOLERANCE));

    string expPath = projectPath + "/Experiments/";
    string prefix = expPath + queryType + "/";
    string queryFile = projectPath + "/Data/ships/Queries/" + queryType + ".txt";
    string dataFile = projectPath + "/Data/ships/ships1e8.txt";
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
    // high_resolution_clock::time_point start = high_resolution_clock::now();
    cout << "Defining MPT..." << endl;
    MPT index = MPT(directoryCap, pageCap);
    /* cout << "Bulkloading MPT..." << endl;
    index.bulkload(dataFile, limit);
    double hTreeCreationTime =
        duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    log << "MPT Creation Time: " << hTreeCreationTime << endl; */
    log << "Directory Capacity: " << directoryCap << endl;
    log << "Page Capacity: " << pageCap << endl;
    log << "Trend Coefficient: " << TC << endl;
    /* map<string, double> stats;
    float indexSize = index.size(stats);
    log << "MPT size in MB: " << float(indexSize / 1e6) << endl;
    log << "No. of pages: " << stats["pages"] << endl;
    log << "No. of directories: " << stats["directories"] << endl; */

    cout << "---Evaluation--- " << endl;
    evaluate(&index, queryFile, logFile);
    return 0;
}
