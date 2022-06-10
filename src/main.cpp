#include <Arduino.h>

#include <SoftwareSerial.h>

#define RX PIN_B0
#define TX PIN_B1

SoftwareSerial mySerial(RX, TX);
int i = 0;
char buf[12];

void setup()
{
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);
  mySerial.begin(19200);
}

void loop()
{
  // if (mySerial.available()>0){
  // buf[i]= mySerial.read();
  // if (int(buf[i])==13 || int(buf[i])==10 ){ //If Carriage return has been reached
  // mySerial.println(buf);
  // for(int x=0;x<=10;x++){
  // buf[x]=' ';
  // }
  // i=0; //start over again
  //
  // } //if enter
  // i++;
  // } //If mySerial.available

  if (mySerial.available() > 0)
  {
    String data = "";

    while (mySerial.available())
    {
      int incomingByte = mySerial.read();
      data += (char)incomingByte;
    }

    mySerial.println(data);
  }
} // LOOP