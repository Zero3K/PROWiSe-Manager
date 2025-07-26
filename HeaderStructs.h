#ifndef HEADERSTRUCTS_H
#define HEADERSTRUCTS_H

// Header structures for the application
#include <windows.h>

// Forward declarations and basic structures
typedef struct {
    DWORD CheckState[64];  // Array for various program settings
} PROGSETTINGS;

// Dialog item text mapping structure
typedef struct {
    DWORD offset;           // Offset within STRINGS_MEM_TABLE
    DWORD controlId;        // Dialog control ID
} DLGITEM_TEXT;

// Dialog item text table
typedef struct {
    DLGITEM_TEXT* items;    // Array of dialog item mappings
    DWORD count;            // Number of items
} DLGITEM_TEXT_TABLE;

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

// MODULE_INFOBUF structure for driver information
typedef struct {
    DWORD NameOfs;          // Offset within String[0] for the name part
    char* String[4];        // Array of strings: [0]=path, [1]=description, [2]=version, [3]=company
    char str1[16];          // Hex string buffer for base address
    char str2[16];          // Hex string buffer for size
} MODULE_INFOBUF;

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