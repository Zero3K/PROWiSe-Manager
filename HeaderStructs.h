#ifndef HEADERSTRUCTS_H
#define HEADERSTRUCTS_H

// Header structures for the application
#include <windows.h>

// Forward declarations and basic structures
typedef struct {
    DWORD CheckState[64];  // Array for various program settings
} PROGSETTINGS;

typedef struct {
    char* pCurrentUserName;
    DWORD dwCurrentUserNameSize;
} MAIN_SETTINGS;

typedef struct {
    HWND hwndDisplay;
    PVOID updTimerProc[16];
    DWORD StatusCPUUsgType;
} DLGHDR;

typedef struct {
    char* About;
    char* CaptionCLSIDinfo;
    char* JumpToRegistry;
    char* CPUusage_;
    char* Services_0x20;
    char* UnblOpenSCManager;
} STRINGS_MEM_TABLE;

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
    float* histCurPos;
} OSCIL_GRAPH_DATA;

typedef struct {
    DWORD* data;
    int count;
    int max_value;
} OSCILGRAPH_DATA;

#endif // HEADERSTRUCTS_H