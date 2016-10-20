#include <DFPlayer_Mini_Mp3.h>
#include <SPI.h>
#include <WiFi.h>
#include <DS3231.h> // incluyo esta que es mas liviana TODO: ver licencias
#include <SoftwareSerial.h>
//#include <Wire.h>
//#include <Time.h>
//#include <DS1307RTC.h>


DS3231  rtc(SDA, SCL);
SoftwareSerial serialMp3(10, 11);
const byte INTERVALO_RELOJ = 100;
const int MEDIA_ESCALA = 512;
const byte ALERTA_VERDE = 0;
const byte ALERTA_AMARILLA = 1;
const byte ALERTA_NARANJA = 2;
const byte ALERTA_ROJA = 3;
const byte DEMORA_ENCENDIDO = 4;
const byte TIEMPO_ENCENDIDO = 60;
const byte TIEMPO_BAJAR_ALERTA = 255; //tiempo sin disparos para relajar el nivel de alerta
const byte TIEMPO_SUBIR_ALERTA = 2; //tiempo minimo entre eventos para subir la alerta (por si es un solo ruido largo)
const byte MICROFONO = A0;
const byte SENSIBILIDAD = A1;
const byte SALIDA0 = 3;
const byte ZUMBADOR = 8;

byte nivelAlerta = 0;
int lecturaAnterior = 0;
unsigned long milisegundosReloj;
long encender0 = 0, apagar0 = 0;
long tiempoActual;
long tiempoUltimoDisparo = 0;

void setup() {
  Serial.begin(9600);
  serialMp3.begin(9600);
  mp3_set_serial(serialMp3);  //set softwareSerial for DFPlayer-mini mp3 module
  delay(1);  //wait 1ms for mp3 module to set volume
  mp3_set_volume (15);  // value 0~30

  rtc.begin(); //inicializo el reloj de tiempo real TODO: sera realmente util cuando haya eventos que dependan de la hora

  randomSeed(millis());

  pinMode(ZUMBADOR, OUTPUT);
  pinMode(SALIDA0, OUTPUT);
}

void loop() {
  if (millis() - milisegundosReloj > INTERVALO_RELOJ) {
    // procesar eventos de tiempo
    milisegundosReloj = millis();
    tiempoActual = rtc.getUnixTime(rtc.getTime());

    if (encender0 <= tiempoActual && tiempoActual <= apagar0) {
      digitalWrite(SALIDA0, LOW); //VALOR_ALTO_0
    }
    else {
      digitalWrite(SALIDA0, HIGH);
    }

    if (tiempoActual - tiempoUltimoDisparo >= TIEMPO_BAJAR_ALERTA)
      //relajo alerta
      nivelAlerta = nivelAlerta > ALERTA_VERDE ? nivelAlerta-- : ALERTA_VERDE;

  }

  int valor = 0;
  valor = analogRead(MICROFONO);
  //  Serial.println(String(valor) +" "+ String(tiempo) + " "+String(encender0)+" "+String(apagar0));
  // Serial.println(valor);

  delay(10); //TODO: este delay no se si sirve de algo, si no sirve borrarlo

  int limite = analogRead(SENSIBILIDAD) / 20;

  if (valor > MEDIA_ESCALA + limite  && lecturaAnterior < MEDIA_ESCALA - limite) {

    tiempoUltimoDisparo = tiempoActual;

    if (tiempoActual - tiempoUltimoDisparo > TIEMPO_SUBIR_ALERTA) {
      //subo alerta
      nivelAlerta = nivelAlerta >= ALERTA_ROJA ? ALERTA_ROJA : nivelAlerta++;
      for (byte i = 0; i < nivelAlerta; i++)
        tone(ZUMBADOR, 100, 100);
      delay(150);

    }

    // en cualquier alerta
    encender0 = tiempoActual + DEMORA_ENCENDIDO; // actua a los cuatro segundos
    apagar0 = tiempoActual + TIEMPO_ENCENDIDO + DEMORA_ENCENDIDO; //por un minuto y apaga

    if (nivelAlerta == ALERTA_AMARILLA) {
      mp3_play(random(9));// del 0 al 9 audios nivel de alerta amarillo
    }

    if (nivelAlerta == ALERTA_NARANJA) {
      mp3_play(10+random(9));// del 10 al 19 audios nivel de alerta naranja
    }

    if (nivelAlerta == ALERTA_ROJA) {
      mp3_play(20+random(9));// del 20 al 29 audios nivel de alerta roja
    }
  }
  lecturaAnterior = valor;

  //este seria el ideal a lograr para modularizacion
  //---------------------------------------------
  //  if (condicion(valor)){
  //   procesarReglas()
  // }

}
