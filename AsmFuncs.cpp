// AsmFuncs.cpp - C++ replacements for assembly functions
// This file replaces the assembly implementations in AsmFuncs.asm and myFuncs.asm

#include <windows.h>
#include <intrin.h>

// Global variables referenced by assembly code
DWORD _Enable_SetTop = 0; // Define the variable that was extern in assembly

// String length function for ANSI strings (replacement for getstrlen)
extern "C" int getstrlen(const char* str)
{
    if (!str) return 0;
    
    const char* start = str;
    while (*str) {
        str++;
    }
    return (int)(str - start);
}

// String length function for Unicode strings (replacement for getstrlenW)
extern "C" int getstrlenW(const wchar_t* str)
{
    if (!str) return 0;
    
    const wchar_t* start = str;
    while (*str) {
        str++;
    }
    return (int)((str - start) * sizeof(wchar_t)); // Return byte length like original asm
}

// 32-bit hash calculation (replacement for asmCalcHash32)
extern "C" DWORD asmCalcHash32(const char* str)
{
    if (!str) return 0;
    
    const char* ptr = str;
    DWORD hash = 0;
    DWORD hash_by_bytes = 0;
    DWORD len = 0;
    
    // First pass: calculate hash by bytes and get length
    DWORD first_dword = *(DWORD*)ptr;
    hash = first_dword;
    
    while (*ptr) {
        hash = _rotr(hash, 7);  // Rotate right by 7 bits
        hash_by_bytes ^= hash;
        ptr++;
        len++;
    }
    
    // Second pass: add hash by dwords
    ptr = str;
    hash = len;  // Start with string length
    
    // Add DWORDs until near the end
    while (ptr + 4 <= str + len - 4) {
        hash += *(DWORD*)ptr;
        ptr += 4;
    }
    
    // Add last 4 bytes
    if (len >= 4) {
        hash += *(DWORD*)(str + len - 4);
    }
    
    // XOR the two hashes
    return hash ^ hash_by_bytes;
}

// Get current process ID from TEB (replacement for asmGetCurrentProcessId)
extern "C" DWORD asmGetCurrentProcessId(void)
{
    // Access TEB (Thread Environment Block) at fs:[0x18], then get PID at offset 0x20
#ifdef _M_IX86
    DWORD* teb = (DWORD*)__readfsdword(0x18);
    return *(DWORD*)((BYTE*)teb + 0x20);
#else
    // Fallback to Windows API for non-x86
    return GetCurrentProcessId();
#endif
}

// Get Process Environment Block from TEB (replacement for asmGetCurrentPeb)
extern "C" PVOID asmGetCurrentPeb(void)
{
    // Access TEB at fs:[0x18], then get PEB at offset 0x30
#ifdef _M_IX86
    DWORD* teb = (DWORD*)__readfsdword(0x18);
    return *(PVOID*)((BYTE*)teb + 0x30);
#else
    // For non-x86, we need to use alternate method
    // This is a simplified fallback - in real scenario might need more complex handling
    return (PVOID)0; // Placeholder - would need proper PEB access for 64-bit
#endif
}

// Process job check (replacement for asmIsProcessInJob)
extern "C" DWORD asmIsProcessInJob(HANDLE hProcess, HANDLE hJob, PVOID lpReserved)
{
    // Call the NT function passed in lpReserved (which should be NtIsProcessInJob)
    typedef NTSTATUS (WINAPI *NtIsProcessInJobFunc)(HANDLE, HANDLE, PBOOLEAN);
    
    if (!lpReserved) return 0;
    
    NtIsProcessInJobFunc ntFunc = (NtIsProcessInJobFunc)lpReserved;
    BOOLEAN isInJob = FALSE;
    
    NTSTATUS status = ntFunc(hProcess, hJob, &isInJob);
    
    if (status < 0) return status; // Return error code if failed
    
    // Check if result is 0x123 (special value from original assembly)
    if (status == 0x123) return 1;
    return isInJob ? 1 : 0;
}

// Window creation hook code (replacement for asmMyCreateWindowExW_EndCode)
// This function needs to be compiled as raw machine code for injection
extern "C" void asmMyCreateWindowExW_EndCode()
{
    // This function is used for binary injection and needs to be implemented
    // as inline assembly to maintain the exact machine code structure
    // expected by HookCreateWindow.cpp
    
#ifdef _M_IX86
    __asm {
        push eax
        push ebx
        mov ebx, _Enable_SetTop
        test ebx, ebx
        jz _quitAsmSetTop
        push 3                  // SWP_NOSIZE | SWP_NOMOVE
        xor ebx, ebx
        push ebx                // y
        push ebx                // x
        push ebx                // cy
        push ebx                // cx
        push 0FFFFFFFFh         // HWND_TOPMOST
        push eax                // hwnd
        mov eax, 0BAADDEADh     // Placeholder for SetWindowPos address
        call eax
    _quitAsmSetTop:
        pop ebx
        pop eax
        ret 030h                // Return and clean up stack (0x30 = 48 bytes)
    }
#else
    // For non-x86 architectures, this injection method won't work
    // This is a placeholder to avoid compilation errors
    return;
#endif
}