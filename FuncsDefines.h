#ifndef FUNCSDEFINES_H
#define FUNCSDEFINES_H

// Function definitions and declarations
#include <windows.h>

// NT DLL functions structure
typedef struct {
    ULONG (*RtlGetLastWin32Error)(void);
    LONG (*pNtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);
    LONG (*pNtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    LONG (*pNtQueryInformationThread)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    LONG (*pNtQueryObject)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    LONG (*pNtQueryInformationFile)(HANDLE, PVOID, PVOID, ULONG, ULONG);
    LONG (*pNtTerminateProcess)(HANDLE, LONG);
    LONG (*pNtOpenThread)(HANDLE*, ACCESS_MASK, PVOID, PVOID);
    LONG (*pNtShutdownSystem)(ULONG);
    LONG (*pNtInitiatePowerAction)(ULONG, ULONG, ULONG, UCHAR);
    LONG (*NtIsProcessInJob)(HANDLE, HANDLE, UCHAR*);
    LONG (*NtCreateKey)(HANDLE*, ACCESS_MASK, PVOID, ULONG, PVOID, ULONG, PULONG);
    LONG (*NtOpenKey)(HANDLE*, ACCESS_MASK, PVOID);
    LONG (*NtDeleteKey)(HANDLE);
    LONG (*NtClose)(HANDLE);
    LONG (*NtOpenFile)(HANDLE*, ACCESS_MASK, PVOID, PVOID, ULONG, ULONG);
    LONG (*NtQueryDirectoryFile)(HANDLE, HANDLE, PVOID, PVOID, PVOID, PVOID, ULONG, ULONG, UCHAR, PVOID, UCHAR);
    LONG (*NtCreateJobObject)(HANDLE*, ACCESS_MASK, PVOID);
    ULONG (*RtlNtStatusToDosError)(LONG);
    LONG (*RtlUnicodeStringToAnsiString)(PVOID, PVOID, UCHAR);
    LONG (*RtlAnsiStringToUnicodeString)(PVOID, PVOID, UCHAR);
} NTDLL_FUNCTIONS;

extern NTDLL_FUNCTIONS ntdllFunctions;

// Assembly function declarations
extern "C" {
    int getstrlen(const char* str);
    int getstrlenW(const wchar_t* str);
    DWORD asmCalcHash32(const char* str);
    DWORD asmIsProcessInJob(HANDLE hProcess, HANDLE hJob, PVOID lpReserved);
    DWORD asmGetCurrentProcessId(void);
    PVOID asmGetCurrentPeb(void);
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