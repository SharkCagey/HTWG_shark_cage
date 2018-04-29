
#include "stdafx.h"
#include "windows.h"
#include <string>
#include <iostream>
#include "ProcessHandler.h"


ProcessHandler::ProcessHandler() {
}


ProcessHandler::~ProcessHandler() {
}

void ProcessHandler::createProcess(LPTSTR desktopName/*SECURITY_DESCRIPTOR *sd*/) {
    // Create process in the windowstation of the user

	// Create Process
	LPTSTR szCmdline = _tcsdup(TEXT("C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe"));
	STARTUPINFO si = { sizeof si };
	si.lpDesktop = desktopName;
	PROCESS_INFORMATION pi;
	/*
	SECURITY_ATTRIBUTES sa = {
		sizeof SECURITY_ATTRIBUTES,
		sd,
		FALSE // Does not inherit the handle
	};*/

	bool success = CreateProcess(NULL, szCmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	// Wait until child process exits.
	//WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	//CloseHandle(pi.hProcess);
	//CloseHandle(pi.hThread);
}
