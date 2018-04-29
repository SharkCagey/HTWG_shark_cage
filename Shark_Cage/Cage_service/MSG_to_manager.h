#pragma once

#include <string>

typedef enum E_MSG_TO_MANAGER {
    MGR_START_PC = 0,       // Start Process (in CageManager)
    MGR_STOP_PC,            // Stop Process (in CageManager)
    MGR_NUMBEROFMESSAGES    // Value that contains the number of messages - NOT A MESSAGE ITSELF !!!
} MSG_TO_MANAGER;

// Names for the message constants to be convertible to string
static const std::string MSG_TO_MANAGER_STRINGS[] = {
    "START_PC",
    "STOP_PC"
};

// To test if there are names for all messages (at compile time)
static_assert((sizeof(MSG_TO_MANAGER_STRINGS)/sizeof(std::string)) == MGR_NUMBEROFMESSAGES, "Error within MessageTypes");

std::string MSG_TO_MANAGER_toString(MSG_TO_MANAGER value) {
    return MSG_TO_MANAGER_STRINGS[value];
}
