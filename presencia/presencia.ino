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
const byte DEMORA_ENCENDIDO_MINIMA = 3;//segundos, lo que tarda en actuar luego de detectar el evento
const byte DEMORA_ENCENDIDO_MAXIMA = 7;//segundos, lo que tarda en actuar luego de detectar el evento
const byte TIEMPO_ENCENDIDO_MAXIMO = 80;//segundos, lo que dura activo el actuador
const byte TIEMPO_ENCENDIDO_MINIMO = 50;//segundos, lo que dura activo el actuador
const byte TIEMPO_BAJAR_ALERTA = 200; //segundos, tiempo sin disparos para relajar el nivel de alerta
const byte TIEMPO_SUBIR_ALERTA = 7; //segundos, tiempo minimo entre eventos para subir la alerta (por si es un solo ruido largo, mas largo que el audio de alerta mas largo)
const int INTERVALO_RELOJ = 350; //milisegundos, demora para procesamiento de eventos de reloj (para que no ejecute en todas las iteraciones del loop y conseguir mejor sensado de eventos)
const byte TIEMPO_NO_ESCUCHARSE = 80; //milisegundos, demora para no escuchar propios sonidos
//----pines de los dispositivos conectados
const byte MICROFONO = A0;
const byte SENSIBILIDAD = A1;
const byte SALIDA0 = 3;
const byte ZUMBADOR = 8;
const byte ESTADO_REPRODUCTOR = 4;
//----otras
const byte DIVISOR_SENSIBILIDAD_LIMITE = 3;

byte nivelAlerta = 0;
int lecturaAnterior = 1023; //no quiero que entre en la primera iteración
int limite = 100;
byte siguienteAudio = 0;

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

  siguienteAudio = random(9);

  // establezco los modos de los pines
  pinMode(ZUMBADOR, OUTPUT);
  pinMode(SALIDA0, OUTPUT);

  digitalWrite(SALIDA0, HIGH); // que arranque en apagado la salida
  tone(ZUMBADOR, 1000, 100); //pitido de encendido
  delay(150); // no quiero que el micro detecte mi pitido
}

void loop() {
  if (millis() - milisegundosReloj > INTERVALO_RELOJ) {
    // procesar eventos de tiempo
    milisegundosReloj = millis();
    tiempoActual = rtc.getUnixTime(rtc.getTime());

    limite = analogRead(SENSIBILIDAD) / DIVISOR_SENSIBILIDAD_LIMITE;
    //     Serial.print(" ");
    // Serial.println(limite);

    if (encender0 <= tiempoActual && tiempoActual <= apagar0) {
      if (digitalRead(SALIDA0) == HIGH) {
        delay(TIEMPO_NO_ESCUCHARSE);
        digitalWrite(SALIDA0, LOW); //VALOR_ALTO_0
        delay(TIEMPO_NO_ESCUCHARSE); //asi no escucha el rele
        lecturaAnterior = analogRead(MICROFONO);
      }
    }
    else if (tiempoActual > apagar0) {
      if (digitalRead(SALIDA0) == LOW) {
        delay(TIEMPO_NO_ESCUCHARSE);
        digitalWrite(SALIDA0, HIGH);
        delay(TIEMPO_NO_ESCUCHARSE); //asi no escucha el rele
        lecturaAnterior = analogRead(MICROFONO);
      }
    }
    if (reproducirAudio > 0 && reproducirAudio <= tiempoActual) {
      reproducirAudio = 0;
      while (digitalRead(ESTADO_REPRODUCTOR) == LOW); // espero a que el reproductor acabe, aunque deberia estar apagado
      // del 0 al 9 audios nivel de alerta verde del 10 al 19 audios nivel de alerta amarillo, del 20 al 29 audios nivel de alerta naranja y del 30 al 39 audios nivel de alerta roja
      mp3_play(siguienteAudio + nivelAlerta * 10);
      siguienteAudio = siguienteAudio == 9 ? 0 : siguienteAudio + 1;

      delay(100);
      while (digitalRead(ESTADO_REPRODUCTOR) == LOW); // espero a que el reproductor acabe
    }

    //si pasa un tiempo sin eventos relajo alerta
    if (tiempoActual - tiempoUltimoDisparo >= TIEMPO_BAJAR_ALERTA)
      nivelAlerta = nivelAlerta > ALERTA_VERDE ? nivelAlerta - 1 : ALERTA_VERDE;
  }

  int valor = analogRead(MICROFONO);

  // log
  //Serial.println(abs(lecturaAnterior-valor));
  // Serial.print(" ");
  // Serial.print(limite);

  if ( abs(lecturaAnterior - valor) > limite   ) {
    // disparo de evento

    // prendo y apago el led
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    if (tiempoActual - tiempoUltimoDisparo > TIEMPO_SUBIR_ALERTA) {

      tiempoUltimoDisparo = tiempoActual;

      //subo alerta
      nivelAlerta = nivelAlerta >= ALERTA_ROJA ? ALERTA_ROJA : nivelAlerta + 1;
      //   Serial.println("alerta: ");
      //   Serial.println(nivelAlerta);
      for (byte i = 0; i < nivelAlerta; i++) { //hago pitar el zumbador tantas veces como nivel de alerta
        tone(ZUMBADOR, 100, 100);
        delay(150); // no quiero que el micro detecte mi pitido
      }
    }

    encender0 = tiempoActual + DEMORA_ENCENDIDO_MINIMA + random(DEMORA_ENCENDIDO_MAXIMA - DEMORA_ENCENDIDO_MINIMA); // actua luego de la demora constante
    apagar0 = encender0 + TIEMPO_ENCENDIDO_MINIMO + random(TIEMPO_ENCENDIDO_MAXIMO - TIEMPO_ENCENDIDO_MINIMO); //durante el tiempo establecido y apaga

    reproducirAudio = tiempoActual + DEMORA_ENCENDIDO_MINIMA + random(DEMORA_ENCENDIDO_MAXIMA - DEMORA_ENCENDIDO_MINIMA); // reproducira el audio acorde al nivel de alerta
  }

  lecturaAnterior = valor;

}
