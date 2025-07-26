#ifndef HEADERSTRUCTS_H
#define HEADERSTRUCTS_H

// Header structures for the application

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
    char* Wait_;
    char* CPUUsageHistory;
    char* Processes;
    char* ColumnTitle[20];  // Array for column titles
    char* Handles;
    char* Total;
    char* Available;
    char* SystemCache;
    char* Current;
    char* Limit;
    char* Peak;
    char* OK;
    char* Cancel;
    char* Browse_;
    char* RunAs_;
    char* Parameters_;
    char* RunAsCurrentUser_;
    char* RunAsAnotherUser_;
    char* RunAsUser_;
    char* TypeNameOfProgram;
    char* File_;
    // Reserve space for additional string fields
    char* Reserved[100];
} STRINGS_MEM_TABLE;

// MODULE_INFOBUF structure for driver information
typedef struct {
    DWORD NameOfs;          // Offset within String[0] for the name part
    char* String[4];        // Array of strings: [0]=path, [1]=description, [2]=version, [3]=company
    char str1[16];          // Hex string buffer for base address
    char str2[16];          // Hex string buffer for size
} MODULE_INFOBUF;

// Basic process information structure (placeholder)
typedef struct _PROCESS_INFO {
    DWORD cbSize;               // Size of structure for validation
    struct _PROCESS_INFO* NextEntry;  // Linked list
    struct _PROCESS_INFO* PrevEntry;  // Linked list
    char* Description;          // Process description
    char* PriorityText;        // Priority text
    int iIcon;                  // Icon index
    UNICODE_STRING ImagePathName; // Image path
    // Additional fields as needed
    DWORD Reserved[32];         // Reserve space for other fields
} PROCESS_INFO;

// Object types names table (placeholder)
typedef struct {
    char** TypeNames;           // Array of type name strings
    DWORD Count;                // Number of types
} OBJTYPES_NAMES_TABLE;

// Validation macro for PROCESS_INFO
#define IsVALID_PROCESS_INFO(p) ((p) && (p)->cbSize == sizeof(PROCESS_INFO))

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