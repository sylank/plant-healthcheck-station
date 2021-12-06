#include "store.h"
#include <EEPROM.h>

#define DATA_ADDRESS 0
#define CLEAR_BIT 255

class EepromUtils
{
private:
    /* data */
public:
    EepromUtils();

    bool isPresent();
    void saveData(Store store);
    Store readData();

    void clearEEPROM();

    ~EepromUtils();
};
