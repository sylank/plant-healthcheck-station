//Libraries
#include <DHT.h>

// #define DEBUG true

//Constants
#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

const int MOISTURE_ANALOGIC_IN = A0; // Arduino's analogic pin
const int BOARD_RESOLUTION = 1024; // The analogic board resolution, for example Arduino Uno is 10 bit (from 0 to 1023)
const float OPERATIONAL_VOLTAGE = 5.0; // The default Arduino voltage
const float MAX_SENSOR_VOLTAGE = 3.0; // The maximum voltage that the sensor can output
const float SENSOR_READ_RATIO = OPERATIONAL_VOLTAGE / MAX_SENSOR_VOLTAGE; // The ratio betwent the two voltages

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Plant Healthcheck station");
  Serial.println("Setup completed...");
}

void loop() {
  int moistureAnalogicVal = analogRead(MOISTURE_ANALOGIC_IN) * SENSOR_READ_RATIO; // Read the analogic data and convert it to [0, 1023] range
  float hum = dht.readHumidity();
  float temp= dht.readTemperature();

  Serial.print("Moisture level: ");
  if (moistureAnalogicVal < BOARD_RESOLUTION * 0.1) {
    Serial.println("emergency: really dry"); // EMERGENCY
    blinkEmergency();
  } else if (moistureAnalogicVal < BOARD_RESOLUTION * 0.33) {
    Serial.println("enfo: not wet"); // INFO
    blinkInfo();
  } else if (moistureAnalogicVal < BOARD_RESOLUTION * 0.66) {
    Serial.println("ok: ok"); // OK
    blinkOk();
  } else if (moistureAnalogicVal < BOARD_RESOLUTION * 0.9) {
    Serial.println("emergency: really wet"); // EMERGENCY
    blinkLed(100, 1);
  } else {
    Serial.println("panic: get drowned"); // PANIC
    blinkPanic();
  }

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
  Serial.println("");
  delay(5000);
}


void blinkOk() {
  blinkLed(500, 1);
}

void blinkInfo() {
  blinkLed(100, 2);
}

void blinkEmergency() {
  blinkLed(100, 4);
}

void blinkPanic() {
  blinkLed(50, 8);
}

void blinkLed(int delayMsec, int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayMsec);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayMsec);
  }
}
