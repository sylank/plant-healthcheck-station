#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#include <SoftwareSerial.h>

#define DEBUG true

//Constants
#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
 
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
SoftwareSerial ESPserial(2, 3); // RX | TX
 
const int AirValue = 615;   //you need to replace this value with Value_1
const int WaterValue = 249;  //you need to replace this value with Value_2
int soilMoistureValue = 0;
int soilmoisturepercent=0;


void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  dht.begin();
  ESPserial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();

  Serial.println("Plant Healthcheck station");
  Serial.println("Setup completed...");
}
void loop() {
  soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  float hum = dht.readHumidity();
  float temp= dht.readTemperature();

  if(soilmoisturepercent >= 100)
  {
    Serial.println("100 %");
    
    display.setCursor(0,0);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Soil moisture: 100 %");
  }
  else if(soilmoisturepercent <=0)
  {
    Serial.println("0 %");
    
    display.setCursor(0,0);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Soil moisture: 0 %");
  }
  else if(soilmoisturepercent >0 && soilmoisturepercent < 100)
  {
    Serial.print(soilmoisturepercent);
    Serial.println("%");
    
    display.setCursor(0,0);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Soil moisture: " + String(soilmoisturepercent) + " %");
  }

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
  Serial.println("");

  display.setCursor(0,10);  //oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Humidity: " + String(hum) + " %");

  display.setCursor(0,20);  //oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Temperature: " + String(temp) + " C");

  display.display();
  delay(3000);
  display.clearDisplay();

  readFromSoftwareSerial();
  connectToTCPServer("192.168.88.207","3333");
  readFromSoftwareSerial();
  sendData("aa-1;0;1.1;2.2;3.11");
  readFromSoftwareSerial();
   sendData2("aa-1;0;"+String(temp)+";"+ String(hum)+";"+String(soilmoisturepercent));
  readFromSoftwareSerial();
  closeConnection();
  readFromSoftwareSerial();
  
  
  // Wifi Status
  display.setCursor(0,0);  //oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("WiFi status");

  display.setCursor(0,10);  //oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("disconnected");

  display.display();
  delay(2000);
  display.clearDisplay();
}


void pingModule() {
   ESPserial.write("AT\r\n");
}

void setupBaudRateOnModule() {
   ESPserial.write("AT+UART_DEF=9600,8,1,0,0\r\n");
}

void clientMode() {
   ESPserial.write("AT+CWMODE=1\r\n");
}

void listWifiAPs() {
   ESPserial.write("AT+CWLAP\r\n");
}

void connectToWifiAP(String name, String pwd) {
   writeToESPModule("AT+CWJAP=\""+name+ "\", \""+pwd+"\"");
}

void connectToTCPServer(String host, String port) {
   writeToESPModule("AT+CIPSTART=\"TCP\",\""+host+"\","+port+"");
}

void sendData(String data) {
  writeToESPModule("AT+CIPSEND=21");
}

void sendData2(String data) {
  writeToESPModule2(data);
}

void closeConnection() {
  delay(2000);
  ESPserial.write("AT+CIPCLOSE\r\n");
}

void writeToESPModule(String data) {
  String command = data + "\r\n";
  ESPserial.write(command.c_str());
}


void writeToESPModule2(String data) {
  String command = data + "\n";
  ESPserial.write(command.c_str());
}

void readFromSoftwareSerial() {
  delay(1000);
  while ( ESPserial.available() ) {
    Serial.write( ESPserial.read() ); 
  }
}
//AT+UART_DEF=9600, 8, 1, 0, 0

//AT+CWMODE=1 client mode

//AT+CWLAP list wifi

//AT+CWJAP="Liberty Media", "nlj-hyiyk187" connect

//AT+CIFSR get IP

//AT+CIPSTART="TCP","192.168.88.207",3333 connect to tcp

//AT+CIPSEND=19 send n bytes

//aa-1;0;1.1;2.2;3.11

//AT+CIPCLOSE
