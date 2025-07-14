// AsmFuncs.cpp - C++ replacements for assembly functions
// This file replaces the assembly implementations in AsmFuncs.asm and myFuncs.asm

#include <windows.h>
#include <intrin.h>

// Global variables referenced by assembly code
DWORD _Enable_SetTop = 0; // Define the variable that was extern in assembly

// String length function for ANSI strings (replacement for getstrlen)
extern "C" int __stdcall getstrlen(const char* str)
{
    if (!str) return 0;
    
    const char* start = str;
    while (*str) {
        str++;
    }
    return (int)(str - start);
}

// String length function for Unicode strings (replacement for getstrlenW)
extern "C" int __stdcall getstrlenW(const wchar_t* str)
{
    if (!str) return 0;
    
    const wchar_t* start = str;
    while (*str) {
        str++;
    }
    return (int)((str - start) * sizeof(wchar_t)); // Return byte length like original asm
}

// 32-bit hash calculation (replacement for asmCalcHash32)
extern "C" DWORD __cdecl asmCalcHash32(const char* str)
{
    if (!str) return 0;
    
    const unsigned char* ptr = (const unsigned char*)str;
    DWORD hash_by_bytes = 0;
    DWORD length = 0;
    
    // Load first 4 bytes as hash (like assembly: mov eax,[esi])
    DWORD hash = *(DWORD*)ptr;
    
    // First pass: calculate hash by bytes and get length
    while (*ptr) {
        hash = _rotr(hash, 7);        // ror eax,7
        hash_by_bytes ^= hash;        // xor [esp],eax
        ptr++;                        // lodsb (increment ptr)
        length++;                     // inc edi
    }
    
    // Second pass: hash by dwords (starting fresh from string)
    ptr = (const unsigned char*)str;
    hash = length;                    // mov eax,edi (start with string length)
    
    // Calculate end address for last 4 bytes (sub edi,5; add edi,esi)
    const unsigned char* end_ptr = ptr + length - 4;
    
    // Add dwords while ptr < end_ptr
    while (ptr < end_ptr) {
        hash += *(DWORD*)ptr;         // add eax,[esi]
        ptr += 4;                     // add esi,4
    }
    
    // Add the last 4 bytes (add eax,[edi])
    if (length >= 4) {
        hash += *(DWORD*)end_ptr;
    }
    
    // Final XOR: hash_by_bytes XOR hash_by_dwords
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
extern "C" DWORD __stdcall asmIsProcessInJob(HANDLE hProcess, HANDLE hJob, PVOID lpReserved)
{
    // Call the NT function passed in lpReserved (which should be NtIsProcessInJob)
    typedef NTSTATUS (WINAPI *NtIsProcessInJobFunc)(HANDLE, HANDLE, PBOOLEAN);
    
    if (!lpReserved) return 0;
    
    NtIsProcessInJobFunc ntFunc = (NtIsProcessInJobFunc)lpReserved;
    BOOLEAN isInJob = FALSE;
    
    NTSTATUS status = ntFunc(hProcess, hJob, &isInJob);
    
    if (status < 0) return status; // Return error code if failed (jl short quit)
    
    // Assembly logic: xor ecx,ecx; cmp eax,0123h; setnz cl; mov eax,ecx
    // This means: return 1 if status is NOT 0x123, otherwise return 0
    return (status != 0x123) ? 1 : 0;
}

// Window creation hook code (replacement for asmMyCreateWindowExW_EndCode)
// This function needs to be compiled as raw machine code for injection
extern "C" void __stdcall asmMyCreateWindowExW_EndCode()
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
        mov eax, 0BAADDEADh     // Placeholder for SetWindowPos address (corrected)
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