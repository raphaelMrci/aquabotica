#include "EEPROMManager.hpp"
#include <EEPROM.h>

#define EEPROM_SIZE 8

void EEPROMManager::init()
{
    EEPROM.begin(EEPROM_SIZE);
}

void EEPROMManager::writeString(int addrOffset, String data)
{
    int strLen = data.length();

    for (int i = 0; i < strLen; i++) {
        EEPROM.write(addrOffset + i, data[i]);
    }
    EEPROM.write(addrOffset + strLen, '\0');
    EEPROM.commit();
}

String EEPROMManager::readString(int addrOffset)
{
    String data = "";
    char c = EEPROM.read(addrOffset);
    int i = 0;

    while (c != '\0') {
        data += c;
        c = EEPROM.read(addrOffset + ++i);
    }

    return data;
}

void EEPROMManager::writeInt(int addrOffset, int data)
{
    EEPROM.write(addrOffset, data);
    EEPROM.commit();
}

int EEPROMManager::readInt(int addrOffset)
{
    return EEPROM.read(addrOffset);
}
