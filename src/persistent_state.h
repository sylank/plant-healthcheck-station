struct PersistentState
{
    unsigned airValue;
    unsigned waterValue;
    bool calculatedSend;

    void Reset()
    {
        // Set default values in case there is nothing
        // stored in the eeprom yet.
        airValue = 615;
        waterValue = 249;
    }
};