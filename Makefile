ifndef FF_ROOT 
FF_ROOT		= ./fastflow
endif

serial: BFS_serial.cpp utimer.cpp Node.cpp Graph.cpp
	g++ -std=c++17  -O3 BFS_serial.cpp -o BFS_serial.out

cpp_threads: BFS_cpp_threads.cpp utimer.cpp Node.cpp Graph.cpp
	g++ -std=c++17  -O3 BFS_cpp_threads.cpp -o BFS_cpp_threads.out -pthread

ff: BFS_fastflow.cpp utimer.cpp Node.cpp Graph.cpp
	g++ -std=c++17 -O3 BFS_fastflow.cpp -o BFS_fastflow.out -I $(FF_ROOT) -pthread

generate_graph: generate_graph.cpp
	g++ -std=c++17 -O3 generate_graph.cpp -o generate_graph.out

all: serial cpp_threads ff generate_graph

clean: 
	rm -f *.out