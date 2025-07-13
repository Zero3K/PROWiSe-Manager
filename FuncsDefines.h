#ifndef FUNCSDEFINES_H
#define FUNCSDEFINES_H

// Function definitions and declarations
#include <windows.h>

// NT DLL functions structure
typedef struct {
    ULONG (*RtlGetLastWin32Error)(void);
    // Add other NT functions as needed
} NTDLL_FUNCTIONS;

extern NTDLL_FUNCTIONS ntdllFunctions;

// HTML Help function pointer
extern HWND (__stdcall *pHtmlHelpA)(HWND, LPCSTR, UINT, DWORD_PTR);
extern HMODULE hHctrlLib;

// Constants for HTML Help
#define ATOM_HTMLHELP_API_ANSI "HtmlHelpA"
#define HH_DISPLAY_TOPIC 0x0000

// Thread function declarations
DWORD WINAPI UpdateThread(LPVOID lpParam);

// Initialization function
void InitializeNtdllFunctions(void);

// Message function
void FailMessage(const char* msg, DWORD code, DWORD flags);

#endif // FUNCSDEFINES_H