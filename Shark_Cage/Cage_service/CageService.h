#pragma once

#include "NetworkManager.h"
#include <Windows.h>
#include <string>

class CageService {
public:
    CageService();
    ~CageService();

    bool beginsWith(const std::string string, const std::string prefix);

    bool cageManagerRunning();
    /*
    * Starts the Cage Manager in a new process on the normal desktop in the given session.
    *
    * @sessionId The sessionId of the user.
    * @return The process ID of the started process.
    */
    DWORD startCageManager(DWORD sessionId);

    /*
     * Starts the executable in a new process on the normal desktop in the given session.
     *
     * @appName The path to the execuatble.
     * @sessionId The sessionId of the user.
     * @return The process ID of the started process.
     */
    DWORD startCageManager(LPCTSTR appName, DWORD sessionId);

    /*
     * Starts the executable in a new process on the respective desktop in the given session.
     *
     * @appName The path to the execuatble.
     * @desktopName The name of the desktop to start the program in.
     * @sessionId The sessionId of the user.
     * @return The process ID of the started process.
     */
    DWORD startCageManager(LPCTSTR appName, LPTSTR desktopName, DWORD sessionId);

    void stopCageManager();

    /*
     * Parses a message and does the action according to content of the message.
     * The message must be in the form of:
     * "MSG_TO_SERVICE.constant absolute/path/to/executable"
     */
    void handleMessage(std::string message, NetworkManager* mgr);

    /*
    * Reads the specifies configuration file and set the value of the image index accordingly.
    * config.txt must be in the directory of the service executable.
    */
    void readConfigFile();

    int getImageIndex(void);

private:
    // Process ID of the Cage Manager (Used for closing the Cage Manager)
    DWORD cageManagerProcessId;

    // Process ID of the Icon-Select-Dialog (used for waiting until dialog closed)
    DWORD dialogProcessId;

    std::string CageService::GetLastErrorAsString(DWORD errorID);

    int imageIndex = -1;    // Initialize with error code

    /*
     * Accepts strings that are more than 8 characters long and returns the number at the end as int.
     */
    int getPictureIndexFromLine(std::string line);
};
