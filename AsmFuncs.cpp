// AsmFuncs.cpp - C++ replacements for assembly functions
// This file replaces the assembly implementations in AsmFuncs.asm and myFuncs.asm

// Note: This file is included in PROWiSe.cpp, so we don't include headers here

// Ensure _rotr is available for older compilers
#ifndef _rotr
#define _rotr(value, shift) (((value) >> (shift)) | ((value) << (32 - (shift))))
#endif

// Global variables referenced by assembly code (defined in Variables.h)
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
    
    const unsigned char* esi = (const unsigned char*)str;
    DWORD hash_by_bytes = 0;
    DWORD edi = 0;  // string length counter
    
    // Load first 4 bytes as initial hash: mov eax,[esi]
    DWORD eax = *(DWORD*)esi;
    
    // Push 0 to make space for hash_by_bytes on stack: push eax (where eax=0)
    // hash_by_bytes serves as [esp] in the assembly
    
    // First pass: calculate hash by bytes and get length
    // _CalcHash32 loop
    do {
        eax = _rotr(eax, 7);           // ror eax,7
        hash_by_bytes ^= eax;          // xor [esp],eax
        unsigned char al = *esi++;     // lodsb (load byte and increment)
        edi++;                         // inc edi (increment length counter)
    } while (edi > 0 && *(esi-1) != 0); // test al,al; jnz _CalcHash32
    
    // Adjust edi to actual string length (excluding null terminator)
    edi--;
    
    // Reset esi to string start: mov esi,dword ptr [esp+4+8+4]
    esi = (const unsigned char*)str;
    
    // Start with string length: mov eax,edi
    eax = edi;
    
    // Calculate last dword address: sub edi,5; add edi,esi
    const unsigned char* end_ptr = esi + (edi >= 5 ? edi - 5 : 0);
    
    // Second pass: hash by dwords
    // _AddDword32 loop
    while (esi < end_ptr) {
        eax += *(DWORD*)esi;      // add eax,[esi]
        esi += 4;                 // add esi,4
    }
    
    // Add last 4 bytes: add eax,[edi]
    if (edi >= 5) {
        eax += *(DWORD*)end_ptr;
    }
    
    // Final XOR: pop edi; xor eax,edi
    // edi now contains hash_by_bytes
    return eax ^ hash_by_bytes;
}

// Get current process ID from TEB (replacement for asmGetCurrentProcessId)
extern "C" DWORD __stdcall asmGetCurrentProcessId(void)
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
extern "C" PVOID __stdcall asmGetCurrentPeb(void)
{
    // Access TEB at fs:[0x18], then get PEB at offset 0x30
#ifdef _M_IX86
    DWORD* teb = (DWORD*)__readfsdword(0x18);
    return (PVOID)*(PVOID*)((BYTE*)teb + 0x30);
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
    typedef LONG (WINAPI *NtIsProcessInJobFunc)(HANDLE, HANDLE, UCHAR*);
    
    if (!lpReserved) return 0;
    
    NtIsProcessInJobFunc ntFunc = (NtIsProcessInJobFunc)lpReserved;
    UCHAR isInJob = 0;
    
    LONG status = ntFunc(hProcess, hJob, &isInJob);
    
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
        mov eax, 0BAADDEADh     // Placeholder for SetWindowPos address
        call eax
    _quitAsmSetTop:
        pop ebx
        pop eax
        // __stdcall automatically cleans up stack parameters (0x30 bytes = 12*4 = 48 bytes)
    }
#else
    // For non-x86 architectures, this injection method won't work
    // This is a placeholder to avoid compilation errors
    return;
#endif
}