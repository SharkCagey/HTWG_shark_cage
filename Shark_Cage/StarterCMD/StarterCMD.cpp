// StarterCMD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Cage service/NetworkManager.h"
#include "../Cage service/MSG_Service.h"
#include <Windows.h>
#include <Commdlg.h>

int main() {
    // Display a message box that asks wheter to start the cage manager or not
	int msgboxID = MessageBox(
		NULL,
		(LPCWSTR)L"Do you want to start the Cage manager through the Cage service?",
		(LPCWSTR)L"Start Cage Manager",
		MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO
	);

    NetworkManager mgr(UI);
	switch (msgboxID) {
	case IDNO:
		return -1; // Just quit
	case IDYES:
        // Send message to service to start cage manager
        mgr.send(MSG_TO_SERVICE_toString(START_CM));
		break;
	}
    
    // Init filters for the second dialog
    TCHAR filename[MAX_PATH] = {0};
    TCHAR filter[MAX_PATH] = 
    {
        TEXT("Executables (.exe)\0*.exe\0") // Only .exe files
        TEXT("All Files\0*.*\0")            // All files
    };

    // Init buffers for the for the file name
    OPENFILENAME ofn = {0};
    TCHAR _sbuf[MAX_PATH];
    TCHAR *initDir = _sbuf;

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.lpstrFilter         = filter;   // Set filters
    ofn.nFilterIndex        = 1;        // Default filter
    ofn.lpstrFile           = filename; // Buffer for the file name
    ofn.nMaxFile            = MAX_PATH; // Max length of the path to the executable
    ofn.lpstrInitialDir     = initDir;  // Pass empty directory path to display last user selected directory
    ofn.lpstrTitle          = L"Select program to run in Cage Manager"; // Title with description
    ofn.Flags               = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_NOREADONLYRETURN | OFN_FILEMUSTEXIST;

    // Display the file selector dialog
    if (!GetOpenFileName(&ofn)) {
        if (CommDlgExtendedError() == 0) {
            return 0;
        }
        return -1;
    }
    std::cout << "Selected file: " << filename << std::endl;

    ///////////////// A bit too simple, should be replaced
    std::string convertedFilename;
    for (char c : filename) {
        convertedFilename += c;
    }
    ////////////////

    mgr.send(MSG_TO_SERVICE_toString(START_PC) + " " + convertedFilename);

    return 0;
}
