#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

STARTUPINFO si = { sizeof(si) };
PROCESS_INFORMATION pi;

// Crea processo sospeso
bool CreateProcessSuspended() {
    if (!CreateProcess("crackme.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        printf("Errore CreateProcess: %d\n", GetLastError());
        return false;
    }
    printf("Processo creato sospeso! PID: %lu\n", pi.dwProcessId);
    return true;
}

// Alloca una stringa nel processo target
LPVOID AllocRemoteString(const char* str) {
    size_t strLen = strlen(str) + 1;
    SIZE_T bytesWritten;
    LPVOID remoteAddr = VirtualAllocEx(pi.hProcess, NULL, strLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(pi.hProcess, remoteAddr, str, strLen, &bytesWritten);
    return remoteAddr;
}

// Prepara lo shellcode
LPVOID PrepareShellcode(LPVOID remoteFmt, DWORD returnAddr) {
    SIZE_T bytesWritten;
    BYTE shellcode[64];
    ZeroMemory(shellcode, sizeof(shellcode));
    int offset = 0;

    // mov [esp+60h+Block], eax
    shellcode[offset++] = 0x89;
    shellcode[offset++] = 0x44;
    shellcode[offset++] = 0x24;
    shellcode[offset++] = 0x58;


    // push [esp+60h+Block]
    shellcode[offset++] = 0xFF;
    shellcode[offset++] = 0x74;
    shellcode[offset++] = 0x24; 
    shellcode[offset++] = 0x58;

    // push offset fmt ("%s")
    shellcode[offset++] = 0x68;
    *(DWORD*)(shellcode + offset) = (DWORD)remoteFmt;
    offset += 4;

    // call printf
    DWORD printfAddr = (DWORD)GetProcAddress(GetModuleHandleA("msvcrt.dll"), "printf");
    shellcode[offset++] = 0xE8;
    int callOffsetIndex = offset;
    *(DWORD*)(shellcode + offset) = 0;
    offset += 4;

    // add esp,8 (pulizia stack)
    shellcode[offset++] = 0x83; 
    shellcode[offset++] = 0xC4; 
    shellcode[offset++] = 0x08;

    // jmp 0x401510
    shellcode[offset++] = 0xE9;
    int jmpOffsetIndex = offset;
    *(DWORD*)(shellcode + offset) = 0;
    offset += 4;

    // Alloca shellcode nel target
    LPVOID remoteShell = VirtualAllocEx(pi.hProcess, NULL, offset, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(pi.hProcess, remoteShell, shellcode, offset, &bytesWritten);

    // Patch call printf
    DWORD relCall = printfAddr - ((DWORD)remoteShell + callOffsetIndex + 4);
    WriteProcessMemory(pi.hProcess, (LPVOID)((DWORD)remoteShell + callOffsetIndex), &relCall, 4, &bytesWritten);

    // Patch jmp ritorno
    DWORD relJmp = returnAddr - ((DWORD)remoteShell + jmpOffsetIndex - 1);
    WriteProcessMemory(pi.hProcess, (LPVOID)((DWORD)remoteShell + jmpOffsetIndex), &relJmp, 4, &bytesWritten);

    return remoteShell;
}

// Patch istruzione originale con jmp verso shellcode
void PatchOriginalInstruction(LPVOID shellAddr, DWORD instrAddr) {
    SIZE_T bytesWritten;
    BYTE patch[5] = { 0xE9 };
    DWORD relAddr = (DWORD)shellAddr - (instrAddr + 5);
    *(DWORD*)(patch + 1) = relAddr;
    WriteProcessMemory(pi.hProcess, (LPVOID)instrAddr, patch, 5, &bytesWritten);
}

// ────────────── Funzione principale ──────────────

int main() {
    if (!CreateProcessSuspended()) return 1;

    // Alloca stringa formato "%s\n"
    LPVOID remoteFmt = AllocRemoteString("%s\n");
    printf("Remote format allocated at: %p\n", remoteFmt);

    // Prepara shellcode e ritorno a 0x401510
    LPVOID remoteShell = PrepareShellcode(remoteFmt, 0x401510);
    printf("Shellcode injected at: %p\n", remoteShell);

    // Patch istruzione originale (0x40150C) per jmp verso shellcode
    PatchOriginalInstruction(remoteShell, 0x40150C);

    // Riprendi thread sospeso
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
