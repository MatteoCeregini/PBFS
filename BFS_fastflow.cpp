#include <iostream>
#include <fstream>
#include <vector>
#include <atomic>
#include <stdio.h>

#include <ff/ff.hpp>

#include "Node.cpp"
#include "Graph.cpp"
#include "utimer.cpp"

using namespace std;
using namespace ff;

// Global variable used to keep track of the occurrences of the value we are looking for in the graph
atomic<int> COUNT = 0;


/******************************************************************************************/
/*                                       EMITTER                                          */
/******************************************************************************************/

struct Emitter: ff_node_t<vector<int>, int> {
    vector<int> currentLayer;
    vector<int> nextLayer;
    // These two variable are used to determine whether the workers have finished
    // processing the current level
    // How many messages the Emitter has sent to the workers of the farm
    int sentMessages;
    // How many messages the Emitter has received
    int receivedMessages;
    Graph* graph;

    // Takes in input a pointer to the graph and the id of the starting node
    Emitter(Graph* g, int id) {
        graph = g;
        currentLayer.push_back(id);
        sentMessages = 0;
        receivedMessages = 0;
    }
    
    int* svc(vector<int>* task) {
        // task == nullptr can only happen at the beginning of the BFS search 
        if(task == nullptr) {
            // current layer contains only one id, so i could write something like
            // ff_send_out(new int(currentLayer[0]))
            for(int & id : currentLayer) {
                ff_send_out(new int(id));
                sentMessages++;
            }
            // the current layer has been processed, so remove all ids from it
            currentLayer.clear();
            return GO_ON;
        } else {
            vector<int> ids = *task;
            delete task;
            // Insert the ids in the next level, but only those that have not yet been inserted
            for(int & id : ids) {
                Node& u = graph->getNode(id);
                if(!u.getAlreadySeen()) {
                    nextLayer.push_back(id);
                    u.setAlreadySeen();
                }
            }
            receivedMessages++;
            // If we received the same number of messages that
            // we sent this means that the workers finished the processing of the current layer
            if(receivedMessages == sentMessages) {
                // If there are no more node to process, the BFS is over
                if(nextLayer.empty()) {
                    return EOS;
                } else {
                    // Otherwise, we need to get ready to process the next layer
                    swap(currentLayer, nextLayer);
                    sentMessages = 0;
                    receivedMessages = 0;
                    // Send current layer ids to workers
                    for(int & id : currentLayer) {
                        ff_send_out(new int(id));
                        sentMessages++;
                    }
                    // the current layer has been processed, so remove all ids from it
                    currentLayer.clear();
                }
            }
        }
        return GO_ON;
    }
};


/******************************************************************************************/
/*                                      WORKER                                            */
/******************************************************************************************/

struct Worker: ff_node_t<int, vector<int>> {
    Graph* graph;
    vector<int> ids;
    // the value we are looking for in the graph
    int value;

    Worker(Graph* g, int v) {
        graph = g;
        value = v;
    }

    vector<int>* svc(int* task) {
        int id = *task;
        delete task;
        Node &u = graph->getNode(id);
        // If the value of the node "u" is the one we are looking for, increment the global atomic variable COUNT
        if(u.getValue() == value) {
            COUNT++;
        }
        for(int & id : u.getAdjacentNodes()) {
            Node& v = graph->getNode(id);
            if(!v.getColor()) {
                ids.push_back(v.getId());
                v.setColor();
            }
        }
        ff_send_out(new vector<int>(ids));
        // Clear the vector ids, so it's ready for the exploration of the next node
        ids.clear();
        return GO_ON;
    }
};



/******************************************************************************************/
/*                                  MAIN FUNCTION                                         */
/******************************************************************************************/



int main(int argc, char *argv[]) {
    int startingNode;
    int value;
    int nWorkers;
    string filePath;
    // Put the user's input in a vector, so it's easier to check if it's valid
    vector<string> input(argv + 1, argv + argc);
    // Check if the input is correct, otherwise terminate the program
    if(input.size()  == 4) {
        try {
            nWorkers = stoi(input[0]);
            if(nWorkers <= 0) {
                cout << "The number of threads must be an integer greater than zero." << endl;
                return -1;
            }
        } catch(const exception& e) {
            cout << "The number of threads must be an integer greater or equal than zero." << endl;
            return -1;
        }
        try {
            startingNode = stoi(input[1]);
            if(startingNode < 0) {
                cout << "The starting node must be an integer greater or equal than zero." << endl;
                return -1;
            }
        } catch(const exception& e) {
            cout << "The starting node must be an integer greater or equal than zero." << endl;
            return -1;
        }
        try {
            value = stoi(input[2]);
        } catch(const exception& e) {
            cout << "The value must be an integer." << endl;
            return -1;
        }
        filePath = input[3];
        ifstream file(input[3]);
        if(!file.is_open()) {
            cout << "Cannot open " << input[3] << ". Are you sure it exists?" << endl;
            return -1;
        } else {
            file.close();
        }
    } else {
        cout << "Wrong number of arguments. USAGE:" << endl;
        cout << "./executable [number of threads] [starting node] [value] [file path]" << endl;
        return -1;
    }

    Graph graph(filePath);

    if(graph.size() <= startingNode) {
        cout << "The starting node " << startingNode << " doesn't exists in the graph." << endl;
        return -1;
    }
    
    graph.getNode(startingNode).setColor();
    Emitter emitter(&graph, startingNode);
    vector<unique_ptr<ff_node>> workers;

    for(int i = 0; i < nWorkers; i++) {
        workers.push_back(make_unique<Worker>(&graph, value));
    }

    ff_Farm<float> farm(move(workers), emitter);
    farm.remove_collector();
    farm.wrap_around();
    {
        // The string is ignored (not printed)
        utimer tseq("Parallel BFS (FastFlow)");
        if(farm.run_and_wait_end() < 0) {
            error("running farm");
            return -1;
        }
    }
    cout << "Occurences: " << COUNT << endl;
    return 0;
}