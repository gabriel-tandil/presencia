#include <DFPlayer_Mini_Mp3.h>
#include <SPI.h>
#include <WiFi.h>
//#include <Wire.h>
//#include <Time.h>
//#include <DS1307RTC.h>
#include <DS3231.h> // incluyo esta que es mas liviana
#include <SoftwareSerial.h>


//DS3231  rtc(SDA, SCL);
SoftwareSerial serialMp3(10, 11);
const int INTERVALO_RELOJ = 1000;
long milis;
long encender0, apagar0;
long tiempo;

void setup() {
  Serial.begin(9600);
  serialMp3.begin(9600);
  mp3_set_serial(serialMp3);  //set softwareSerial for DFPlayer-mini mp3 module
  delay(1);  //wait 1ms for mp3 module to set volume
  mp3_set_volume (15);
//  rtc.begin();
  // rtc.setTime(13,29,00);
  //  rtc.setDate(20,4,2016);
  //  rtc.setDOW(3);

pinMode(9, OUTPUT);
    pinMode(12, OUTPUT);
      pinMode(13, OUTPUT);
      digitalWrite(13, HIGH);
      digitalWrite(12, LOW);

      analogReference(INTERNAL);
}

void loop() {
  if (millis() - milis > INTERVALO_RELOJ) {
    // procesar eventos de tiempo
    milis = millis();
  // Time tiempoT = rtc.getTime();
//    Serial.print(tiempoT.hour);
//    Serial.print(":" );
//    Serial.print(tiempoT.min);
//    Serial.print(":");
//    Serial.println(tiempoT.sec );
//    Serial.println(rtc.getTemp());
  //  tiempo=rtc.getUnixTime(tiempoT);

  if (encender0 <= tiempo &&tiempo<=apagar0)
    digitalWrite(0, HIGH); //VALOR_ALTO_0
  else
    digitalWrite(0, LOW);

  }

  int valor;
  valor=analogRead(A1);
   Serial.println(valor);
   delay(1);
//  if (condicion(valor)){
 //   procesarReglas()
 // }


}


