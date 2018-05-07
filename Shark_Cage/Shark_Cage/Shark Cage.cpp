// Shark Cage.cpp : Entry point for the program.

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include "ProcessHandler.h"

// Start as LocalSystem
int main(int argc, char* argv[]) {
    // Create CageService
    ProcessHandler pc;
    LPTSTR desktopName = L"desktopName";
    LPTSTR appName = _tcsdup(TEXT("CageManager.exe"));
    DWORD processId = GetCurrentProcessId();
    DWORD sessionId;
    if (ProcessIdToSessionId(processId, &sessionId)) {
        // CageService starts CageManager
        //pc.startCageManager(appName, desktopName, sessionId);
    } else {
        std::cout << "Unable to get sessionID\n";
        return -1;
    }

    return 0;
}


void startProcessInNewDesktop() {
    // Save the current desktop
    HDESK currentDesktop = GetThreadDesktop(GetCurrentThreadId());

    // Create new desktop and switch to it
    LPTSTR desktopName = L"New Desktop";
    HDESK newDesktop = CreateDesktop(desktopName, NULL, NULL, 0, GENERIC_ALL, NULL);
    SwitchDesktop(newDesktop);

    Sleep(500); // Wait to set up desktop

                // Create new process on the new desktop
    ProcessHandler pc;
    pc.createProcess(desktopName);

    Sleep(5000);

    // Return the old desktop and close the new one
    SwitchDesktop(currentDesktop);
    CloseDesktop(newDesktop);
}