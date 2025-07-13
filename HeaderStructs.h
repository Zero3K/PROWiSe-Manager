#ifndef HEADERSTRUCTS_H
#define HEADERSTRUCTS_H

// Header structures for the application
#include <windows.h>

// Graph data structures (referenced in Variables.h)
typedef struct {
    DWORD* data;
    int count;
    int max_value;
} VERTICAL_GRAPH_DATA;

typedef struct {
    DWORD* data;
    int count;
    int max_value;
} OSCIL_GRAPH_DATA;

typedef struct {
    DWORD* data;
    int count;
    int max_value;
} OSCILGRAPH_DATA;

#endif // HEADERSTRUCTS_H