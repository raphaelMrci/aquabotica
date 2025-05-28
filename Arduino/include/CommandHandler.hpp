#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <Arduino.h>

// Define the maximum number of commands and buffer size
#define MAX_COMMANDS 20
#define COMMAND_BUFFER_SIZE 64

// Define a type for command handler functions
typedef void (*CommandFunction)(const String &);

class CommandHandler
{
  private:
    struct CommandRoute {
        String command;
        CommandFunction handler;
    };

    CommandRoute routes[MAX_COMMANDS];
    int numRoutes;

    Stream &serial;

    // Buffer for accumulating incoming command data
    char commandBuffer[COMMAND_BUFFER_SIZE];
    int bufferIndex;

  public:
    CommandHandler(Stream &serialStream);

    // Register a command and its handler
    bool registerRoute(const String &command, CommandFunction handler);

    // Parse and execute an incoming command (non-blocking)
    void handleIncomingCommand();

    // Send a command with optional arguments
    void sendCommand(const String &command, const String &args = "");
};

#endif
