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

//----niveles de alerta
const byte ALERTA_VERDE = 0;
const byte ALERTA_AMARILLA = 1;
const byte ALERTA_NARANJA = 2;
const byte ALERTA_ROJA = 3;

//----constantes de tiempo
const byte DEMORA_ENCENDIDO = 4;//segundos, lo que tarda en actuar luego de detectar el evento
const byte TIEMPO_ENCENDIDO = 60;//segundos, lo que dura activo el actuador
const byte TIEMPO_BAJAR_ALERTA = 250; //segundos, tiempo sin disparos para relajar el nivel de alerta
const byte TIEMPO_SUBIR_ALERTA = 2; //segundos, tiempo minimo entre eventos para subir la alerta (por si es un solo ruido largo)
const byte INTERVALO_RELOJ = 100; //milisegundos, demora para procesamiento de eventos de reloj (para que no ejecute en todas las iteraciones del loop y conseguir mejor sensado de eventos)

//----pines de los dispositivos conectados
const byte MICROFONO = A0;
const byte SENSIBILIDAD = A1;
const byte SALIDA0 = 3;
const byte ZUMBADOR = 8;
const byte ESTADO_REPRODUCTOR = 4;

//----otras
const int MEDIA_ESCALA = 512; // supuesto medio de la señal de entrada analógica del microfono

byte nivelAlerta = 0;
int lecturaAnterior = 0;
unsigned long milisegundosReloj;
long encender0 = 0, apagar0 = 0, reproducirAudio = 0;
long tiempoActual;
long tiempoUltimoDisparo = 0;

void setup() {
  Serial.begin(9600);

  serialMp3.begin(9600);
  mp3_set_serial(serialMp3);  //set softwareSerial for DFPlayer-mini mp3 module
  delay(1);  //wait 1ms for mp3 module to set volume
  mp3_set_volume (15);  // value 0~30

  rtc.begin(); //inicializo el reloj de tiempo real TODO: sera realmente util cuando haya eventos que dependan de la hora

  randomSeed(millis()); // inicializo la semilla aleatoria con el reloj interno

  // establezco los modos de los pines
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

    if (reproducirAudio > 0 && reproducirAudio <= tiempoActual) {
      reproducirAudio = 0;

      while (digitalRead(ESTADO_REPRODUCTOR) == LOW); // espero a que el reproductor acabe antes de reproducir el nuevo audio

      // del 0 al 9 audios nivel de alerta verde del 10 al 19 audios nivel de alerta amarillo, del 20 al 29 audios nivel de alerta naranja y del 30 al 39 audios nivel de alerta roja
      byte numAudio = random(9) + nivelAlerta * 10;
      mp3_play(numAudio);
    }

    //si pasa un tiempo sin eventos relajo alerta
    if (tiempoActual - tiempoUltimoDisparo >= TIEMPO_BAJAR_ALERTA)
      nivelAlerta = nivelAlerta > ALERTA_VERDE ? nivelAlerta-- : ALERTA_VERDE;

  }

  int valor = 0;
  valor = analogRead(MICROFONO);

  delay(10); //TODO: este delay no se si sirve de algo, si no sirve borrarlo

  int limite = analogRead(SENSIBILIDAD) / 20;

  // disparo de evento
  if (valor > MEDIA_ESCALA + limite  && lecturaAnterior < MEDIA_ESCALA - limite) {

    tiempoUltimoDisparo = tiempoActual;

    if (tiempoActual - tiempoUltimoDisparo > TIEMPO_SUBIR_ALERTA) {
      //subo alerta
      nivelAlerta = nivelAlerta >= ALERTA_ROJA ? ALERTA_ROJA : nivelAlerta++;

      for (byte i = 0; i < nivelAlerta; i++) { //hago pitar el zumbador tantas veces como nivel de alerta
        tone(ZUMBADOR, 100, 100);
        delay(150);
      }
    }

    encender0 = tiempoActual + DEMORA_ENCENDIDO; // actua luego de la demora constante
    apagar0 = tiempoActual + TIEMPO_ENCENDIDO + DEMORA_ENCENDIDO; //durante el tiempo establecido y apaga

    reproducirAudio = tiempoActual + DEMORA_ENCENDIDO; // reproducira el audio acorde al nivel de alerta

  }

  lecturaAnterior = valor;

  //este seria el ideal a lograr para modularizacion
  //---------------------------------------------
  //  if (condicion(valor)){
  //   procesarReglas()
  // }

}
