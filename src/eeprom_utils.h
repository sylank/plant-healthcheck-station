#include "store.h"

#define DATA_ADDRESS 0
#define CLEAR_BIT 255

class EepromUtils
{
private:
    bool read = true;
    Store stored;

public:
    EepromUtils();

    bool isPresent();
    void saveData(Store store);
    Store readData();

    void clearEEPROM();

    ~EepromUtils();
};
