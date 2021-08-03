#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <queue>
#include <mutex>

#include "utimer.cpp"

using namespace std;


/******************************************************************************************/
/*                                  NODE CLASS                                            */
/******************************************************************************************/


class Node {

    private:
        int id; // Each node is identified by an unique ID (starting from zero).
        int value;
        bool visited;
        vector<int> adjacentNodes; // The IDs of the adjiencent nodes.

    public:
        Node(int nodeId, int nodeValue, vector<int> adj);
        void print();
        bool isVisited();
        void setVisited();
        int getId();
        int getValue();
        vector<int> getAdjacentNodes();

};

Node::Node(int nodeId, int nodeValue, vector<int> adj) {
    id = nodeId;
    value = nodeValue;
    visited = false;
    adjacentNodes = adj;
}

void Node::print() {
    cout << "NODE INFO:" << endl;
    cout << "- Id: " << id << endl;
    cout << "- Value: " << value << endl;
    cout << "- Visited: " << (visited ? "True" : "False") << endl;
    cout << "- Adjacent Nodes: ";
    for(int & idNode : adjacentNodes) {
        cout << idNode << ", ";
    }
    cout << endl;
    cout << "----------------" << endl;
}

bool Node::isVisited() {
    return visited;
}

void Node::setVisited() {
    visited = true;
}

int Node::getId() {
    return id;
}

int Node::getValue() {
    return value;
}

vector<int> Node::getAdjacentNodes() {
    return adjacentNodes;
}




/******************************************************************************************/
/*                                  GRAPH CLASS                                           */
/******************************************************************************************/

class Graph {

    private:
        vector<Node> nodes;

    public:
        Graph();
        Graph(string filePath);
        void insertNode(Node node);
        void print();
        Node& getNode(int idNode);
        int getNumberOfNodes();
};

Graph::Graph() {
}



Graph::Graph(string filePath) {
    FILE *file;
    file = fopen(filePath.c_str(), "r");
    if(file == NULL) {
        cout << "CANT READ FILE: " << filePath << endl;
    }

    int nodeId;
    int value;
    int lenght;
    while(fread(&nodeId, sizeof(int), 1, file) > 0) {
        int result = fread(&value, sizeof(int), 1, file);
        fread(&lenght, sizeof(int), 1, file);
        vector<int> adj;
        adj.resize(lenght);
        fread(&adj[0], sizeof(int), lenght, file);
        Node node(nodeId, value, adj);
        nodes.push_back(node);
    }
    fclose(file);

}


void Graph::insertNode(Node node) {
    auto iter = nodes.begin() + node.getId();
    nodes.insert(iter, node);
}

void Graph::print() {
    cout << "NODES OF THE GRAPH:" << endl;
    cout << "----------------" << endl;
    for(Node & node : nodes) {
        node.print();
    }
}

// Returns the node with id = idNode by reference
Node& Graph::getNode(int idNode) {
    return nodes[idNode];
}


int Graph::getNumberOfNodes() {
    return nodes.size();
}



/******************************************************************************************/
/*                                      BFS CLASS                                         */
/******************************************************************************************/



class SerialBFS {
    private:
        Graph graph;

    public:
        SerialBFS(Graph g);
        int countOccurrences(int startingNodeId, int value);
    
};

SerialBFS::SerialBFS(Graph g) {
    graph = g;
}

int SerialBFS::countOccurrences(int startingNodeId, int value) {
    int count = 0;
    queue<Node> q;
    auto& startingNode = graph.getNode(startingNodeId);
    startingNode.setVisited();
    q.push(startingNode);
    while(!q.empty()) {
        auto u = q.front();
        q.pop();
        if(u.getValue() == value) {
            count++;
        }
        for(auto idNode : u.getAdjacentNodes()) {
            auto &v = graph.getNode(idNode);
            if(!v.isVisited()) {
                v.setVisited();
                q.push(v);
            }
        }
    }
    return count;
}


/******************************************************************************************/
/*                                  MAIN FUNCTION                                         */
/******************************************************************************************/



int main(int argc, char *argv[]) {

    int startingNode;
    int value;
    string filePath;
    // Put the user's input in a vector, so it's easier to check if it's valid
    vector<string> input(argv + 1, argv + argc);
    // Check if the input is correct, otherwise terminate the program
    if(input.size()  == 3) {
        try {
            startingNode = stoi(input[0]);
            if(startingNode < 0) {
                cout << "The starting node must be an integer greater or equal than zero." << endl;
                return -1;
            }
        } catch(const exception& e) {
            cout << "The starting node must be an integer greater or equal than zero." << endl;
            return -1;
        }
        try {
            value = stoi(input[1]);
        } catch(const exception& e) {
            cout << "The value must be an integer." << endl;
            return -1;
        }
        filePath = input[2];
        ifstream file(input[2]);
        if(!file.is_open()) {
            cout << "Cannot open " << input[2] << ". Are you sure it exists?" << endl;
            return -1;
        } else {
            file.close();
        }
    } else {
        cout << "Wrong number of arguments. USAGE:" << endl;
        cout << "./executable [starting node] [value] [file path]" << endl;
        return -1;
    }

    Graph graph(filePath);
    if(graph.getNumberOfNodes() <= startingNode) {
        cout << "The starting node " << startingNode << " doesn't exists in the graph." << endl;
        return -1;
    }
    SerialBFS bfs(graph);
    int x;
    {
        utimer tseq("");
        x = bfs.countOccurrences(startingNode, value);
    }
    cout << "Occurences: " << x << endl;

    return 0;
}