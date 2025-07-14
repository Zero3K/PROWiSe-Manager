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

// Assembly function declarations (using __stdcall to match original assembly)
extern "C" {
    int __stdcall getstrlen(const char* str);
    int __stdcall getstrlenW(const wchar_t* str);
    DWORD __cdecl asmCalcHash32(const char* str);
    DWORD __stdcall asmIsProcessInJob(HANDLE hProcess, HANDLE hJob, PVOID lpReserved);
    DWORD __cdecl asmGetCurrentProcessId(void);
    PVOID __cdecl asmGetCurrentPeb(void);
    void __stdcall asmMyCreateWindowExW_EndCode(void);
}

// C function declarations  
int copystring(char* dest, const char* src);
void strappend(char* dest, const char* src);
LPVOID LocalAllocAndCopy(LPCSTR str);
void Try_Except_Error(DWORD code);
void WatchProcesses();
BOOL CheckCriticalDependencies();

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