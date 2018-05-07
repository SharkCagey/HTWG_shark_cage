#pragma once

#include <string>

typedef enum E_MSG_TO_SERVICE {
    START_CM = 0,       // Start CageManager
    STOP_CM,            // Stop CageManager
    START_PC,           // Start Process (in CageManager)
    STOP_PC,            // Stop Process (in CageManager)
    NUMBEROFMESSAGES    // Value that contains the number of messages - NOT A MESSAGE ITSELF !!!
} MSG_TO_SERVICE;

// Names for the message constants to be convertible to string
static const std::string MSG_TO_SERVICE_STRINGS[] = {
    "START_CM",
    "STOP_CM",
    "START_PC",
    "STOP_PC"
};

// To test if there are names for all messages (at compile time)
static_assert((sizeof(MSG_TO_SERVICE_STRINGS)/sizeof(std::string)) == NUMBEROFMESSAGES, "Error within MessageTypes");

std::string MSG_TO_SERVICE_toString(MSG_TO_SERVICE value) {
    return MSG_TO_SERVICE_STRINGS[value];
}
