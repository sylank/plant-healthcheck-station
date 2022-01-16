class MeasurementValues
{
private:
    int soilMoistureValue = 0;
    float humidity = 0;
    float temperature = 0;

public:
    MeasurementValues();

    void setSoilMoistureValue(int val);
    void setHumidity(float val);
    void setTemperature(float val);

    int getSoilMoistureValue();
    float getHumidity();
    float getTemperature();

    ~MeasurementValues();
};

MeasurementValues::MeasurementValues()
{
}

void MeasurementValues::setSoilMoistureValue(int val)
{
    soilMoistureValue = val;
}

void MeasurementValues::setHumidity(float val)
{
    humidity = val;
}

void MeasurementValues::setTemperature(float val)
{
    temperature = val;
}

int MeasurementValues::getSoilMoistureValue()
{
    return soilMoistureValue;
}

float MeasurementValues::getHumidity()
{
    return humidity;
}

float MeasurementValues::getTemperature()
{
    return temperature;
}

MeasurementValues::~MeasurementValues()
{
}
