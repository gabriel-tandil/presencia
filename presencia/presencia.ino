#include <DFPlayer_Mini_Mp3.h>
#include <SPI.h>
#include <WiFi.h>
//#include <Wire.h>
//#include <Time.h>
//#include <DS1307RTC.h>
#include <DS3231.h> // incluyo esta que es mas liviana TODO: ver licencias
#include <SoftwareSerial.h>


DS3231  rtc(SDA, SCL);
SoftwareSerial serialMp3(10, 11);
const int INTERVALO_RELOJ = 100;
const int MEDIA_ESCALA=512;
const int ALERTA_VERDE=0;
const int ALERTA_AMARILLA=1;
const int ALERTA_NARANJA=2;
const int ALERTA_ROJA=3;

int nivelAlerta=0;
long milis;
long encender0 = 0, apagar0 = 0;
long tiempo;
int anterior = 0;

void setup() {
  Serial.begin(9600);
  serialMp3.begin(9600);
  mp3_set_serial(serialMp3);  //set softwareSerial for DFPlayer-mini mp3 module
  delay(1);  //wait 1ms for mp3 module to set volume
  mp3_set_volume (15);
  rtc.begin();
  // rtc.setTime(13,29,00);
  // rtc.setDate(20,4,2016);
  // rtc.setDOW(3);
  pinMode(8, OUTPUT);//buzz
  pinMode(3, OUTPUT); //rele
}

void loop() {
  if (millis() - milis > INTERVALO_RELOJ) {
    // procesar eventos de tiempo
    milis = millis();
    Time tiempoT = rtc.getTime();
    //    Serial.print(tiempoT.hour);
    //    Serial.print(":" );
    //    Serial.print(tiempoT.min);
    //    Serial.print(":");
    //    Serial.println(tiempoT.sec );
    // Serial.println(rtc.getTemp());
    tiempo = rtc.getUnixTime(tiempoT);

    if (encender0 <= tiempo && tiempo <= apagar0) {
      digitalWrite(3, LOW); //VALOR_ALTO_0
    }
    else {
      digitalWrite(3, HIGH);
    }
  }

  int valor = 0;
  valor = analogRead(A0);
  //  Serial.println(String(valor) +" "+ String(tiempo) + " "+String(encender0)+" "+String(apagar0));
  Serial.println(valor);

  delay(10);

  int limite = analogRead(A1)/20;

  if (valor > MEDIA_ESCALA + limite  && anterior < MEDIA_ESCALA - limite) {
    // Serial.println(String(valor) +" "+ String(tiempo) + " "+String(encender0)+" "+String(apagar0));

    encender0 = tiempo + 4;
    apagar0 = tiempo + 60;
    tone(8,1000,100);
  }
  anterior = valor;
  //  if (condicion(valor)){
  //   procesarReglas()
  // }


}
