#include "CageService.h"

#include "MSG_Service.h"

#include <iostream>
#include <tchar.h>


CageService::CageService() {
    cageManagerProcessId = 0;
}


CageService::~CageService() {
}

bool CageService::cageManagerRunning() {
    return cageManagerProcessId > 0;
}

DWORD CageService::startCageManager(DWORD sessionId) {
    LPTSTR szCmdline = _tcsdup(TEXT("C:\\sharkcage\\CageManager.exe"));
    return startCageManager(szCmdline, sessionId);
}


DWORD CageService::startCageManager(LPCTSTR appName, DWORD sessionId) {
    return startCageManager(appName, nullptr, sessionId);
}


std::string CageService::GetLastErrorAsString(DWORD errorID) {
    //Get the error message, if any.
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL,
                                 errorID,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPSTR)&messageBuffer,
                                 0,
                                 NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

// Must be part of the service
DWORD CageService::startCageManager(LPCTSTR appName, LPTSTR desktopName, DWORD sessionId) {
    HANDLE hServiceToken;
    HANDLE hUserSessionToken;

    STARTUPINFO si = { sizeof si };
    si.lpDesktop = desktopName;
    PROCESS_INFORMATION pi;
    DWORD processId = -1;

    // Use nwe token with privileges for the trudting vomputinh base
    if (ImpersonateSelf(SecurityImpersonation)) {
        std::cout << "ImpersonateSelf was successful\n";
        if (OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, false, &hServiceToken)) {
            std::cout << "OpenThreadToken was successful\n";
            if (DuplicateTokenEx(hServiceToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hUserSessionToken)) {
                std::cout << "DuplicateTokenEx was successful\n";
                if (SetTokenInformation(hUserSessionToken, TokenSessionId, &sessionId, sizeof DWORD)) {
                    std::cout << "SetTokenInformation was successful\n";
                    if (CreateProcessAsUser(hUserSessionToken,
                                            appName,
                                            NULL,
                                            NULL,  // <- Process Attributes
                                            NULL,  // Thread Attributes
                                            false, // Inheritaion flags
                                            0,     // Creation flags
                                            NULL,  // Environment
                                            NULL,  // Current directory
                                            &si,   // Startup Info
                                            &pi)) {
                        std::cout << "CreateProcess (" << appName << ") was succesful\n";
                        processId = pi.dwProcessId;
                        CloseHandle(&si);
                        CloseHandle(&pi);
                    } else {
                        std::cout << "CreateProcess (" << appName << ") failed (" << GetLastError() << "): " << GetLastErrorAsString(GetLastError());
                    }
                } else {
                    std::cout << "SetTokenInformation failed (" << GetLastError() << "): " << GetLastErrorAsString(GetLastError());
                }
            } else {
                std::cout << "DuplicateTokenEx failed (" << GetLastError() << "): " << GetLastErrorAsString(GetLastError());
            }
        } else {
            std::cout << "OpenThreadToken failed (" << GetLastError() << "): " << GetLastErrorAsString(GetLastError());
        }
    } else {
        std::cout << "ImpersonateSelf failed (" << GetLastError() << "): " << GetLastErrorAsString(GetLastError());
    }
    return processId;
}

void CageService::stopCageManager() {
    // Concider a "graceful" shutdown, i.e. sending a message/signal and then wait for the process terminating itself
    HANDLE cageManagerHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, cageManagerProcessId);
    if (cageManagerHandle != NULL) {
        TerminateProcess(cageManagerHandle, 55);
        std::cout << "Terminated CageManager: 55" << std::endl;
    } else {
        std::cout << "Could not stop CageManager: Invalid Process Handle" << std::endl;
    }

}


void CageService::handleMessage(std::string message, NetworkManager* mgr) {
    if(beginsWith(message, MSG_TO_SERVICE_toString(START_CM))) {
        // Start Process
        if (cageManagerProcessId == 0) {
            // Get session id from loged on user
            DWORD sessionId = WTSGetActiveConsoleSessionId();
            cageManagerProcessId = startCageManager(sessionId);
        }
    } else if (beginsWith(message, MSG_TO_SERVICE_toString(STOP_CM))) {
        // Stop Process
        stopCageManager();
    } else if (beginsWith(message, MSG_TO_SERVICE_toString(START_PC))) {
        // Forward to cage manager
        (*mgr).send(message);

        HANDLE cageManagerHandle = OpenProcess(SYNCHRONIZE, TRUE, cageManagerProcessId);
        WaitForSingleObject(cageManagerHandle, INFINITE);
        cageManagerProcessId = 0;

    } else if (beginsWith(message, MSG_TO_SERVICE_toString(STOP_PC))) {
        // Forward to cage manager
        (*mgr).send(message);
    } else {
        // Unrecognized message - Do nothing
    }
}


bool CageService::beginsWith(const std::string string, const std::string prefix) {
    if (prefix.length() > string.length()) {
        return false;
        // Throw Exception "Bad parameters: prefix longer than the actual string"
    } else {
        if (string.compare(0, prefix.length(), prefix) == 0) {
            return true;
        } else {
            return false;
        }
    }
}