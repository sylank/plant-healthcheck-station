#include "eeprom_utils.h"

#include <EEPROM.h>

EepromUtils::EepromUtils(/* args */)
{
    read = true;
}

bool EepromUtils::isPresent()
{
    return EEPROM.read(DATA_ADDRESS) != CLEAR_BIT; // Arduino Uno
}

void EepromUtils::saveData(Store store)
{
    EEPROM.put(DATA_ADDRESS, store);
    read = true;
}

Store EepromUtils::readData()
{
    if (read)
    {
        read = false;
        Store val; //Variable tso store custom object read from EEPROM.
        Store tmp = EEPROM.get(DATA_ADDRESS, val);

        stored = tmp;
    }

    return stored;
}

void EepromUtils::clearEEPROM()
{
    for (uint16_t i = 0; i <= EEPROM.length(); i++)
    {
        EEPROM.write(i, CLEAR_BIT);
    }

    read = true;
}

EepromUtils::~EepromUtils()
{
}
