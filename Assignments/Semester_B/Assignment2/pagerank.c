/* 208853598 Omri Asudon */
/* 211465968 Matan Kronfeld */
#include "pagerank.h"
#include "thr_pool.h"
#include <stdlib.h>
#include <stdio.h>

#define D 0.15

typedef struct {
    int start;
    int end;
    float* old_ranks;
    float* new_ranks;
    int* outlinks;
    int** in_edges;
    int* in_degrees;
    double sumB;
    int N;
} PageRankTask;

void* compute_ranks(void* arg) {
    PageRankTask* task = (PageRankTask*)arg;
    for (int i = task->start; i < task->end; i++) {
        double sumA = 0.0;
        for (int k = 0; k < task->in_degrees[i]; k++) {
            int j = task->in_edges[i][k];
            sumA += task->old_ranks[j] / (double)task->outlinks[j];
        }
        task->new_ranks[i] = (float)(D / task->N + (1.0 - D) * (sumA + task->sumB));
    }
    return NULL;
}

void PageRank(Graph* g, int n, float* rank) {
    if (g == NULL || g->numVertices == 0 || n <= 0) return;
    int N = g->numVertices;
    
    // Allocate arrays for graph traversal
    int* outlinks = (int*)calloc(N, sizeof(int));
    int* in_degrees = (int*)calloc(N, sizeof(int));
    int** in_edges = (int**)malloc(N * sizeof(int*));
    int* seen = (int*)calloc(N, sizeof(int));
    
    // 1st Pass: Calculate unique outlinks and in_degrees
    // Ignore self-loops and duplicate edges as per instructions
    for (int i = 0; i < N; i++) {
        node* v = g->adjacencyLists[i];
        while (v != NULL) {
            int target = v->v;
            if (target != i && seen[target] != (i + 1)) {
                seen[target] = i + 1;
                outlinks[i]++;
                in_degrees[target]++;
            }
            v = v->next;
        }
    }
    
    // Allocate in_edges array for each vertex
    for (int i = 0; i < N; i++) {
        in_edges[i] = (int*)malloc(in_degrees[i] * sizeof(int));
        in_degrees[i] = 0; // reset to 0 to use as insertion index
    }
    
    // 2nd Pass: Populate in_edges
    for (int i = 0; i < N; i++) {
        node* v = g->adjacencyLists[i];
        while (v != NULL) {
            int target = v->v;
            if (target != i && seen[target] != (i + 1 + N)) {
                seen[target] = i + 1 + N;
                in_edges[target][in_degrees[target]++] = i;
            }
            v = v->next;
        }
    }
    free(seen);
    
    // Setup ping-pong buffers for ranks to avoid copying
    float* r1 = (float*)malloc(N * sizeof(float));
    float* r2 = (float*)malloc(N * sizeof(float));
    for (int i = 0; i < N; i++) {
        r1[i] = 1.0f / N;
    }
    
    float* current_ranks = r1;
    float* next_ranks = r2;
    
    // Setup thread pool
    int num_threads = 8;
    thr_pool_t* pool = thr_pool_create(num_threads, num_threads, 10, NULL);
    
    // Create task partitions
    int num_tasks = num_threads * 4; // Oversubscribe for better load balancing
    if (N < num_tasks) num_tasks = N;
    
    PageRankTask* tasks = (PageRankTask*)malloc(num_tasks * sizeof(PageRankTask));
    int chunk_size = (N + num_tasks - 1) / num_tasks;
    
    for (int iter = 0; iter < n; iter++) {
        // Compute sumB sequentially - computing it once is O(N)
        double sumB = 0.0;
        for (int j = 0; j < N; j++) {
            if (outlinks[j] == 0) {
                sumB += current_ranks[j] / N;
            }
        }
        
        // Queue tasks
        for (int t = 0; t < num_tasks; t++) {
            tasks[t].start = t * chunk_size;
            tasks[t].end = (t + 1) * chunk_size;
            if (tasks[t].end > N) tasks[t].end = N;
            
            tasks[t].old_ranks = current_ranks;
            tasks[t].new_ranks = next_ranks;
            tasks[t].outlinks = outlinks;
            tasks[t].in_edges = in_edges;
            tasks[t].in_degrees = in_degrees;
            tasks[t].sumB = sumB;
            tasks[t].N = N;
            
            if (tasks[t].start < N) {
                thr_pool_queue(pool, compute_ranks, &tasks[t]);
            }
        }
        
        // Wait for all computations in this iteration
        thr_pool_wait(pool);
        
        // Swap rank buffers
        float* temp = current_ranks;
        current_ranks = next_ranks;
        next_ranks = temp;
    }
    
    // Copy result back to caller's array
    for (int i = 0; i < N; i++) {
        rank[i] = current_ranks[i];
    }
    
    // Cleanup
    thr_pool_destroy(pool);
    free(tasks);
    free(r1);
    free(r2);
    for (int i = 0; i < N; i++) {
        free(in_edges[i]);
    }
    free(in_edges);
    free(in_degrees);
    free(outlinks);
}
