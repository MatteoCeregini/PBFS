#include <iostream>
#include <vector>
#include <stdio.h>
// #include "Node.cpp"


using namespace std;


/******************************************************************************************/
/*                                  GRAPH CLASS                                           */
/******************************************************************************************/


class Graph {

    private:
        vector<Node> nodes;
    public:
        Graph();
        Graph(string filePath);
        void print();
        Node& getNode(int idNode);
        int size();
};

Graph::Graph() {
}


Graph::Graph(string filePath) {
    FILE *file;
    file = fopen(filePath.c_str(), "r");
    if(file == NULL) {
        cout << "Cannot read file: " << filePath << endl;
        exit(EXIT_FAILURE);
    }

    int nodeId;
    int value;
    int lenght;
    while(fread(&nodeId, sizeof(int), 1, file) > 0) {
        size_t res;
        res = fread(&value, sizeof(int), 1, file);
        if(res != 1) {
            cout << "Cannot read file: " << filePath << endl;
            exit(EXIT_FAILURE);
        }
        res = fread(&lenght, sizeof(int), 1, file);
        if(res != 1) {
            cout << "Cannot read file: " << filePath << endl;
            exit(EXIT_FAILURE);
        }
        vector<int> adj;
        adj.resize(lenght);
        res = fread(&adj[0], sizeof(int), lenght, file);
        if(res != lenght) {
            cout << "Cannot read file: " << filePath << endl;
            exit(EXIT_FAILURE);
        }
        Node node(nodeId, value, adj);
        nodes.push_back(node);
    }
    fclose(file);
}


int Graph::size() {
    return nodes.size();
}


Node& Graph::getNode(int idNode) {
    return nodes[idNode];
}


void Graph::print() {
    cout << "NODES OF THE GRAPH:" << endl;
    for(Node node : nodes) {
        node.print();
    }
}