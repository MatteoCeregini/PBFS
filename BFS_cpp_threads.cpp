#include <iostream>
#include <fstream>
#include <vector>
#include <atomic>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <math.h> 

#include "Node.cpp"
#include "Graph.cpp"
#include "utimer.cpp"

using namespace std;

/******************************************************************************************/
/*                                  PARALLEL BFS CLASS                                    */
/******************************************************************************************/



class ParallelBFS {

    private:
        Graph graph;
        int numberOfWorkers;    // Number of thread that are used in the BFS
        vector<thread> workers; // Vector used to keep track of the threads
        vector<int> currentLayer;
        vector<int> nextLayer;
        int value;              // The value we want to count during the BFS
        atomic<int> valueCount;
        bool finished;          // If true it means the BFS is over
        /* These variables are used to synchronize the threads after processing currentLayer */
        int countWorkers;       // Used to keep track of the number of threads that have finished processing currentLayer
        mutex m;                // Mutex and condition variable used to synchronize the threads
        condition_variable cv;
        void explore(int tid);
    
    public:
        ParallelBFS(Graph g, int nWorkers, int val);
        int getValueCount();
        void countOccurrences(int startingNodeId);
};


ParallelBFS::ParallelBFS(Graph g, int nWorkers, int val) {
    countWorkers = 0;
    graph = g;
    numberOfWorkers = nWorkers;
    workers.resize(nWorkers);
    value = val;
    valueCount = 0;
    finished = false;
}

int ParallelBFS::getValueCount() {
    return valueCount;
}

void ParallelBFS::countOccurrences(int startingNodeId) {
    // Insert the id of the starting node into the current layer
    currentLayer.push_back(startingNodeId);
    graph.getNode(startingNodeId).setColor();
    graph.getNode(startingNodeId).setAlreadySeen();

    for(int tid = 0; tid < numberOfWorkers; tid++) {
        workers[tid] = thread(&ParallelBFS::explore, this, tid);
    }
    for(int tid = 0; tid < numberOfWorkers; tid++) {
        workers[tid].join();
    }
    //graph.print();
}

void ParallelBFS::explore(int tid) {
    // Until the BFS search is not finished
    do {
        // Vector used to store the ids of the nodes that must be added to the next layer
        vector<int> nextLayerIds;
        // Starting position of the portion of the current layer assigned to the worker
        int start;
        // Ending position (inclusive) of the portion of the current layer assigned to the worker
        int end;
        // If the worker has some nodes to explore (true) or not (false)
        bool work = false;
        // If there are more workers then node ids
        if(numberOfWorkers > currentLayer.size()) {
            // Each worker with tid smaller then the size of the current layer will get a node (work = true)
            // The other workers will have nothing to do (work is still false for them)
            if(tid < currentLayer.size()) {
                start = tid;
                end = tid;
                work = true;
            }
        // If there are more node ids then workers, each worker will have some nodes to explore
        } else {
            // Each worker will get more or less the same number of ids. (the last one may get more ids then the others)
            int chunck = currentLayer.size() / numberOfWorkers;
            start = tid * chunck;
            // The last worker will get the nodes up to the end of the current layer
            if(tid == numberOfWorkers - 1) {
                end = currentLayer.size() - 1;
            } else {
                end = ((tid + 1) * chunck) - 1;
            }
            work = true;
        }
        // If the worker has some nodes to explore
        if(work) {
            // Explore the nodes
            for(int i = start; i <= end; i++) {
                int nodeId = currentLayer[i];
                Node& u = graph.getNode(nodeId);
                if(u.getValue() == value) {
                    valueCount++;
                }
                for(int & id : u.getAdjacentNodes()) {
                    Node& v = graph.getNode(id);
                    if(!v.getColor()) {
                        nextLayerIds.push_back(id);
                        v.setColor();
                    }
                }
            }
        }
        // Synchronization phase of the threads
        {
            unique_lock<mutex> lock(m);
            countWorkers++;
	    // Write the nodes that we have found in the next layer (if any)
            for(int & id : nextLayerIds) {
                Node& node = graph.getNode(id);
                if(!node.getAlreadySeen()) {
                    nextLayer.push_back(id);
                    node.setAlreadySeen();
                }
            }
            // If last thread
            if(countWorkers == numberOfWorkers) {
                // Reset the thread counter for the next iteration
                countWorkers = 0;
                // If there the next layer is empty then the BFS search is finished
                if(nextLayer.empty()) {
                    finished = true;
                } else { // Otherwise, prepare the layers for the next iteration
                    swap(currentLayer, nextLayer);
                    nextLayer.clear();
                }
                cv.notify_all();
            } else {
                cv.wait(lock);
            }
        }
    } while(!finished);
}



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
    ParallelBFS bfs(graph, nWorkers, value);

    {
        // The string is ignored (not printed)
        utimer tseq("Parallel BFS (cpp threads)");
        bfs.countOccurrences(startingNode);
    }
    cout << "Occurences: " << bfs.getValueCount() << endl;
    return 0;
}
