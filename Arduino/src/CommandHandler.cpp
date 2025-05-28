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

            // Serial.print("Received command: ");
            // Serial.println(command);

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
                    // Serial.print("Executing handler for command: ");
                    // Serial.println(cmd);
                    routes[i].handler(args);
                    break;
                }
            }

            // Reset the buffer for the next command
            bufferIndex = 0;
            memset(commandBuffer, 0, sizeof(commandBuffer));
        } else if (bufferIndex < (sizeof(commandBuffer) - 1)) {
            // Add character to buffer (prevent overflow)
            commandBuffer[bufferIndex++] = incomingChar;
        }
    }
}

// Send a command with optional arguments
void CommandHandler::sendCommand(const String &command, const String &args)
{
    String fullCommand = command;
    if (args.length() > 0) {
        fullCommand += " " + args;
    }

    fullCommand += '\n'; // Add newline character

    serial.print(fullCommand);

    // Serial.print("Sending command: ");
    // Serial.print(fullCommand);
    // Serial.print('\n');
}
