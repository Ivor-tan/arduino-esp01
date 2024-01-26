
#include <SoftwareSerial.h>

SoftwareSerial mySerial(1, 0); // 0 RX, 1 TX

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  mySerial.begin(9600);

}

void loop()
{
  if (mySerial.available())
  {
    Serial.write(mySerial.read());
      Serial.println("========>"+mySerial.read());
  }
  if (Serial.available())
  {
    mySerial.write(Serial.read());
    Serial.println("========>"+Serial.read());
  }
  // delay(1000);
}
