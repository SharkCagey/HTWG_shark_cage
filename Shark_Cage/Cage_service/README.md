# Cage Service
The Cage Service is a program that is registered to Windows as a service. It has to run in the background and listens for essages from the UI. According to the messages it starts/stops the Cage Manager or forwards messages to the Manager.

## Files

### Headers

#### CageService.h
Header file for the class that encapsulates starting and stoping the Cage Manager.

#### MSG_Service.h
This header contains the messages to be received by the Cage Service. The enum for type safety, the strings to send through network connection and assert to make sure that for every message type exists a corresponding string.

#### MSG_to_manager.h
Contains messages to be received by the Cage Manager.

#### NetworkManager.h
Simplifies the network communication by providing a send() and listen() function. Has to be instanciated differently for each component (with provided constats).

#### StatusManager.h
Contains the statuses of the Cage Service.

### Source

#### _testMain.cpp
Unimportant file for testing purposes.

#### CageService.cpp
Contains functions to start and stop the Cage Manager.

#### CageServiceMain.cpp
This file contains the entry point for the Windows service and the fields and functions that allow Windows to start and stop the service.

_tmain(), ServiceMain() and ServiceCtrlHandler() are template functions that have to be the same for mostly every service.

ServiceWorkerThread() holds the loop of the service to listen to messages coming from the UI.

startCageManager() starts an instance of the Cage Manager in the session of the currently logged-on user.

stopCageManager terminates the Cage Manager created by startCageManager().

onReceive() parses a message, that was received in the main loop. It tests if the message begins with a certain string and does an action acording to that.

beginsWith() tests if a string begins with a prefix. Returns true if the string begins with the prefix or if they are the same.
