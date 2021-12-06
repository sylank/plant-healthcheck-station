#include "eeprom_utils.h"

EepromUtils::EepromUtils(/* args */)
{
}

bool EepromUtils::isPresent()
{
    return EEPROM.read(DATA_ADDRESS) != CLEAR_BIT; // Arduino Uno
}

void EepromUtils::saveData(Store store)
{
    EEPROM.put(0, store);
}

Store EepromUtils::readData()
{
    Store val; //Variable tso store custom object read from EEPROM.
    return EEPROM.get(DATA_ADDRESS, val);
}

void EepromUtils::clearEEPROM()
{
    for (uint16_t i = 0; i <= EEPROM.length(); i++)
    {
        EEPROM.write(i, CLEAR_BIT);
    }
}

EepromUtils::~EepromUtils()
{
}
