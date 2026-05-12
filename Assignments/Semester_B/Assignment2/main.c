#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "pagerank.h"

#define D 0.15 // damping factor

void initializeRanks(float *ranks, int N) {
    for (int i = 0; i < N; i++) {
        ranks[i] = 1.0 / N;
    }
}


void PageRank_serial(Graph *graph, int iterations, float* ranks) {
    int N = graph->numVertices;
    float *newRanks = (float *)malloc(N * sizeof(float));
    int* outlinkes = (int*)malloc(N* sizeof(int));

    initializeRanks(ranks, N);
    
    //outlinks calculations
    
    for(int i = 0; i < N; i++) {
        node* v = graph->adjacencyLists[i];
        while (v!=NULL) {
            outlinkes[i]++;
            v = v->next;
        }
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        //calculate nodes with outlinks to i
        for (int i = 0; i < N; i++) {
            vertex* out2i = (vertex*)malloc(N * sizeof(vertex));
            
            for(int j = 0; j < N; j++) {
                if( j == i) continue;
                node* v = graph->adjacencyLists[j];
                while (v != NULL) {
                    if (v->v == i) {
                        out2i[j] = 1;
                        break;
                    }
                    v = v->next;
                }
            }
            
            //calculate i rank
            double sumA = 0.0;
            double sumB = 0.0;
            for(int j = 0 ; j < N; j++) {
                if(out2i[j] == 1) {
                    sumA += ranks[j]/outlinkes[j];
                } else if(outlinkes[j] == 0) {
                    sumB += ranks[j]/N;
                }
            }
            newRanks[i] = D/N +(1-D)*(sumA+sumB);
        }

        for (int i = 0; i < N; i++) {
            ranks[i] = newRanks[i];
        }
    }

    free(newRanks);
}

#include <time.h>

// Helper function to get current time in seconds
double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(void) {
    int N = 5000; // Large number of nodes
    int iterations = 100; // Number of iterations
    int num_edges = 50000; // Large number of edges

    printf("Generating a graph with %d nodes and %d edges...\n", N, num_edges);
    
    // Initialize the graph
    Graph *graph = createGraph(N);
    
    // Seed random number generator
    srand(42);
    
    // Add random edges
    for (int i = 0; i < num_edges; i++) {
        int u = rand() % N;
        int v = rand() % N;
        addEdge(graph, u, v);
    }

    // Create two arrays to hold the results
    float *ranks_serial = (float *)malloc(N * sizeof(float));
    float *ranks_parallel = (float *)malloc(N * sizeof(float));

    printf("Running Serial PageRank...\n");
    double start_serial = get_time();
    PageRank_serial(graph, iterations, ranks_serial);
    double end_serial = get_time();
    
    printf("Running Parallel PageRank...\n");
    double start_parallel = get_time();
    PageRank(graph, iterations, ranks_parallel);
    double end_parallel = get_time();
    
    // Verify results match (check the first 5 nodes)
    printf("\n--- Results (First 5 nodes) ---\n");
    for (int i = 0; i < 5; i++) {
        printf("Node %d: Serial = %f, Parallel = %f\n", i, ranks_serial[i], ranks_parallel[i]);
    }
    
    printf("\n--- Performance Comparison ---\n");
    printf("Serial Time:   %f seconds\n", end_serial - start_serial);
    printf("Parallel Time: %f seconds\n", end_parallel - start_parallel);
    printf("Speedup:       %.2fx\n", (end_serial - start_serial) / (end_parallel - start_parallel));
    
    // Free allocated memory
    for (int i = 0; i < N; i++) {
        node *adjList = graph->adjacencyLists[i];
        while (adjList != NULL) {
            node *temp = adjList;
            adjList = adjList->next;
            free(temp);
        }
    }

    free(ranks_serial);
    free(ranks_parallel);
    free(graph->adjacencyLists);
    free(graph);

    return 0;
}
