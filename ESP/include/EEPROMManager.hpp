#pragma once

#include <Arduino.h>

class EEPROMManager
{
  public:
    EEPROMManager() = default;

    void init();

    void writeString(int addrOffset, String data);
    String readString(int addrOffset);

    void writeInt(int addrOffset, int data);
    int readInt(int addrOffset);

  private:
    int _addrOffset = 0;
};
