/* Main
 *
 * Author: Matuesz Bojanowski
 *  Email: bojanowski.mateusz@gmail.com
 */
#include <iostream>
#include <string>
#include <memory>
#include <ctime>
#include <algorithm>
#include <limits>
#include <chrono>
#include <unistd.h>
#include "apsp.h"

using namespace std;
using namespace std::chrono;

/**
 * Print help for command line parameters
 */
void usageCommand(string message = "") {
    cerr << message << std::endl;
    cout << "Usage: ./binnary.out [-a <int>] < input_data.txt" << endl;
    cout << "-a Type of algorithm for APSP" << endl
            << "\t0: NAIVE FLOYD WARSHALL" << endl
            << "\t1: CUDA NAIVE FLOYD WARSHALL" << endl
            << "\t2: CUDA BLOCKED FLOYD WARSHALL" << endl;
    cout << "-h for help" << endl;
    exit(-1);
}

/**
 * Parse command line parameters
 *
 * @param argc: number of parameters
 * @param argv: parameters
 *
 * @return: Algorithm type
 */
graphAPSPAlgorithm parseCommand(int argc, char** argv) {
    int opt;
    graphAPSPAlgorithm algorithm;
    while ((opt = getopt(argc, argv, "ha:")) != -1) {
        switch (opt) {
        case 'a':
            algorithm = static_cast<graphAPSPAlgorithm>(atoi(optarg));
            break;
        case 'h':
            usageCommand();
            break;
        default:
            usageCommand("Unknown argument !!!");
        }
    }
    if (algorithm > graphAPSPAlgorithm::CUDA_BLOCKED_FW ||
            algorithm < graphAPSPAlgorithm::NAIVE_FW)
        usageCommand("Incorrect value for -a !!!");
    return algorithm;
}

/**
 * Read data from input
 *
 * @result: unique ptr to graph data with allocated fields
 */
unique_ptr<graphAPSPTopology> readData() {
    int nvertex, nedges;
    int v1, v2, value;
    cin >> nvertex >> nedges;

    /* Init data graph */
    unique_ptr<graphAPSPTopology> data;
    data = unique_ptr<graphAPSPTopology>(new graphAPSPTopology(nvertex));
    fill_n(data->pred.get(), nvertex * nvertex, -1);
    fill_n(data->graph.get(), nvertex * nvertex, numeric_limits<int>::max() / 4);

    /* Load data from  standard input */
    for (int i=0; i < nedges; ++i) {
        cin >> v1 >> v2 >> value;
        data->graph[v1 * nvertex + v2] = value;
        data->pred[v1 * nvertex + v2] = v1;
    }

    /* Path from vertex v to vertex v is 0 */
    for (unsigned int i=0; i < nvertex; ++i) {
        data->graph[i * nvertex + i] = 0;
        data->pred[i * nvertex + i] = -1;
    }
    return data;
}

/**
 * Print data graph (graph matrix, prep) and time
 *
 * @param graph: pointer to graph data
 * @param time: time in seconds
 */
void printDataJson(const unique_ptr<graphAPSPTopology>& graph, int time) {
    // Lambda function for printMatrix
    ios::sync_with_stdio(false);
    auto printMatrix = [](unique_ptr<int []>& graph, int n) {
        cout << "[";
        for (int i = 0; i < n; ++i) {
            cout << "[";
            for (int j = 0; j < n; ++j) {
                cout << graph[i * n + j];
                if (j != n - 1) cout << ",";
            }
            if (i != n - 1)
                cout << "],\n";
            else
                cout << "]";
        }
        cout << "],\n";
    };

    cout << "{\n    graph:\n";
    printMatrix(graph->graph, graph->nvertex);
    cout << "    predecessors: \n";
    printMatrix(graph->pred, graph->nvertex);
    cout << "    compute_time [ms]: " << time << "\n}";
}

int main(int argc, char **argv) {
    auto algorithm = parseCommand(argc, argv);
    auto graph = readData();

    printDataJson(graph, 0);
    /* Compute APSP */
    high_resolution_clock::time_point start = high_resolution_clock::now();
    apsp(graph, algorithm);
    high_resolution_clock::time_point stop = high_resolution_clock::now();

    /* Print graph */
    auto duration = duration_cast<milliseconds>( stop - start ).count();
    printDataJson(graph, duration);
    return 0;
}