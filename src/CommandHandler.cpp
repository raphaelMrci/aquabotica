#include "CommandHandler.hpp"

// Constructor
CommandHandler::CommandHandler(Stream &serialStream)
    : serial(serialStream), numRoutes(0), bufferIndex(0)
{
    memset(commandBuffer, 0, sizeof(commandBuffer));
}

// Register a command and its handler
bool CommandHandler::registerRoute(const String &command,
                                   CommandFunction handler)
{
    if (numRoutes >= MAX_COMMANDS) {
        return false; // Command table full
    }
    routes[numRoutes].command = command;
    routes[numRoutes].handler = handler;
    numRoutes++;
    return true;
}

// Parse and execute an incoming command (non-blocking)
void CommandHandler::handleIncomingCommand()
{
    while (serial.available()) {
        char incomingChar = serial.read();

        // Handle newline character (end of command)
        if (incomingChar == '\n') {
            commandBuffer[bufferIndex] = '\0'; // Null-terminate the buffer
            String command = String(commandBuffer);
            command.trim();        // Remove leading/trailing whitespace
            command.toUpperCase(); // Convert to uppercase

            // Parse command and arguments
            int spaceIndex = command.indexOf(' ');
            String cmd = command.substring(0, spaceIndex);
            String args =
                (spaceIndex != -1) ? command.substring(spaceIndex + 1) : "";

            // Find and execute the corresponding handler
            for (int i = 0; i < numRoutes; i++) {
                String toCompare = routes[i].command;

                toCompare.trim();
                toCompare.toUpperCase();

                if (toCompare == cmd) {
                    routes[i].handler(args);
                    break;
                }
            }

            // Reset the buffer for the next command
            bufferIndex = 0;
            memset(commandBuffer, 0, sizeof(commandBuffer));
        } else {
            // Add character to buffer with overflow protection
            if (bufferIndex < COMMAND_BUFFER_SIZE - 1) {
                commandBuffer[bufferIndex++] = incomingChar;
            } else {
                // Reset buffer on overflow to prevent undefined behavior
                bufferIndex = 0;
                memset(commandBuffer, 0, sizeof(commandBuffer));
            }
        }
    }
}

// Send a command with optional arguments
void CommandHandler::sendCommand(const String &command, const String &args)
{
    String fullCommand = command;
    if (!args.isEmpty()) {
        fullCommand += " ";
        fullCommand += args;
    }
    serial.println(fullCommand);
}
