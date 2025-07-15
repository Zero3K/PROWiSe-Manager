#ifndef FUNCSDEFINES_H
#define FUNCSDEFINES_H

// Function definitions and declarations
#include <windows.h>
#include "NTstruct.h"

// NT Status codes
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif
#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#endif

// NT data types
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif
#ifndef PBOOLEAN
typedef BOOLEAN* PBOOLEAN;
#endif

// NT DLL functions structure
typedef struct {
    ULONG (*RtlGetLastWin32Error)(void);
    NTSTATUS (*pNtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);
    NTSTATUS (*pNtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    NTSTATUS (*pNtQueryInformationThread)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    NTSTATUS (*pNtQueryObject)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    NTSTATUS (*pNtQueryInformationFile)(HANDLE, PVOID, PVOID, ULONG, ULONG);
    NTSTATUS (*pNtTerminateProcess)(HANDLE, NTSTATUS);
    NTSTATUS (*pNtOpenThread)(PHANDLE, ACCESS_MASK, PVOID, PVOID);
    NTSTATUS (*pNtShutdownSystem)(ULONG);
    NTSTATUS (*pNtInitiatePowerAction)(ULONG, ULONG, ULONG, BOOLEAN);
    NTSTATUS (*NtIsProcessInJob)(HANDLE, HANDLE, PBOOLEAN);
    NTSTATUS (*NtCreateKey)(PHANDLE, ACCESS_MASK, PVOID, ULONG, PVOID, ULONG, PULONG);
    NTSTATUS (*NtOpenKey)(PHANDLE, ACCESS_MASK, PVOID);
    NTSTATUS (*NtDeleteKey)(HANDLE);
    NTSTATUS (*NtClose)(HANDLE);
    NTSTATUS (*NtOpenFile)(PHANDLE, ACCESS_MASK, PVOID, PVOID, ULONG, ULONG);
    NTSTATUS (*NtQueryDirectoryFile)(HANDLE, HANDLE, PVOID, PVOID, PVOID, PVOID, ULONG, ULONG, BOOLEAN, PVOID, BOOLEAN);
    NTSTATUS (*NtCreateJobObject)(PHANDLE, ACCESS_MASK, PVOID);
    NTSTATUS (*RtlNtStatusToDosError)(NTSTATUS);
    NTSTATUS (*RtlUnicodeStringToAnsiString)(PVOID, PVOID, BOOLEAN);
    NTSTATUS (*RtlAnsiStringToUnicodeString)(PVOID, PVOID, BOOLEAN);
} NTDLL_FUNCTIONS;

extern NTDLL_FUNCTIONS ntdllFunctions;

// Hash tables for dynamic function loading
extern WORD ntdllHASHTable[];
extern WORD user32HASHTable[];

// User32 functions structure (if used)
typedef struct {
    // Add user32 function pointers as needed
    PVOID placeholder;
} USER32_FUNCTIONS;

extern USER32_FUNCTIONS user32Functions;

// Function for dynamic loading
void GetFuncProcAddress(DWORD hModule, PVOID pFunctions, WORD* pHashTable);

// Assembly function declarations (using __stdcall to match original assembly)
extern "C" {
    int __stdcall getstrlen(const char* str);
    int __stdcall getstrlenW(const wchar_t* str);
    DWORD __cdecl asmCalcHash32(const char* str);
    DWORD __stdcall asmIsProcessInJob(HANDLE hProcess, HANDLE hJob, PVOID lpReserved);
    DWORD __stdcall asmGetCurrentProcessId(void);
    PEB* __stdcall asmGetCurrentPeb(void);
    void __stdcall asmMyCreateWindowExW_EndCode(void);
}

// C function declarations  
int copystring(char* dest, const char* src);
void strappend(char* dest, const char* src);
LPVOID LocalAllocAndCopy(LPCSTR str);
void Try_Except_Error(DWORD code);
void WatchProcesses();
BOOL CheckCriticalDependencies();
int MessageIntErr(int errorType);
void Settings_VerifyWndPlacement();

// Text string constants
extern const char* ntdll_dll;
extern const char* user32dll_txt;

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