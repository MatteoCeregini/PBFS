#include <iostream>
#include <queue>
#include <algorithm>
#include <random>
#include <stdio.h>


using namespace std;

int main() {
    srand(time(nullptr));

    // PARAMETERS:

    // Number of nodes in the graph
    int number_of_nodes = 30000;
    // Max. number of children for each node
    int max_children = 15;
    // Min.number of children for each node
    int min_children = 1;
    // Percentile of extra edges
    int percent_other_edges = 0;


    // Nodes that we have already inserted in the graph
    queue<int> nodes;
    vector<vector<int>> graph(number_of_nodes, vector<int>());
    //Insert the node 0 in the graph
    nodes.push(0);
    number_of_nodes--;
    // The "biggest" node that we have generated so far
    int max_node = 0;

    // Create a tree by adding nodes until we added number_of_nodes - 1 nodes (zero is already in the tree)
    while(number_of_nodes > 0) {
        int parent_node = nodes.front();
        nodes.pop();
        // Get a random number of children between min_children and max_children (included)
        int number_of_children = rand() % max_children + min_children;
        for(int i = 0; i < number_of_children; i++) {
            if(number_of_nodes > 0) {
                max_node++;
                number_of_nodes--;
                nodes.push(max_node);
                graph[parent_node].push_back(max_node);
            } else {
                continue;
            }
        }
    }
    cout << "Adding extra edges..." << endl;

    for(int from_node = 0; from_node < graph.size() - 1; from_node++) {
        for(int to_node = from_node + 1; to_node < graph.size() - 1; to_node++) {
            int r = rand() % 101; 
            if(r < percent_other_edges) {
                auto iter = find(graph[from_node].begin(), graph[from_node].end(), to_node);
                if(iter == graph[from_node].end())  {
                    graph[from_node].push_back(to_node);
                }
            }
        }
    }

    cout << "Writing graph into file..." << endl;
    FILE *file;
    file = fopen("./graph", "w");
    if(file == NULL) {
        cout << "CANNOT OPEN FILE" << endl;
        return -1;
    }
    for(int node = 0; node < graph.size(); node++) {
        fwrite(&node, sizeof(int), 1, file);
        int value = rand() % 6;
        fwrite(&value, sizeof(int), 1, file);
        int lenght = graph[node].size();
        fwrite(&lenght, sizeof(int), 1, file);
        fwrite(&(graph[node][0]), sizeof(int), lenght, file);
    }
    fclose(file);
}
