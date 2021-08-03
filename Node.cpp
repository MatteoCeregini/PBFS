#include <iostream>
#include <vector>


using namespace std;


/******************************************************************************************/
/*                                  NODE CLASS                                            */
/******************************************************************************************/


class Node {

    private:
        // Each node is identified by an unique ID (starting from zero).
        int id;
        // The value of the node. It's between 0 and 5 (inclusive)
        int value;
        // If color is true, it means that the node is already been visited
        bool color;
        // If alredySeen is true, it means that the node is already been inserted into the next layer
        bool alreadySeen; 
        // The IDs of the adjiencent nodes.
        vector<int> adjacentNodes;

    public:
        Node(int nodeId, int nodeValue, vector<int> adj);
        bool getColor();
        void setColor();
        bool getAlreadySeen();
        void setAlreadySeen();
        int getId();
        int getValue();
        vector<int> getAdjacentNodes();
        void print();

};

Node::Node(int nodeId, int nodeValue, vector<int> adj) {
    id = nodeId;
    value = nodeValue;
    adjacentNodes = adj;
    color = false;
    alreadySeen = false;
}


bool Node::getColor() {
    return color;
}


void Node::setColor() {
    color = true;
}


bool Node::getAlreadySeen() {
    return alreadySeen;
}


void Node::setAlreadySeen() {
    alreadySeen = true;
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


void Node::print() {
    cout << "NODE:" << endl;
    cout << "id: " << id << endl;
    cout << "color: "  << color << endl;
    cout << "Adj nodes: ";
    for(int idNode : adjacentNodes) {
        cout << idNode << " ";
    }
    cout << endl;
}
