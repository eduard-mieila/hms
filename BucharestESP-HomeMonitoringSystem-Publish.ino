// Copyright 2022 (C) MIEILA Eduard-Robert
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// Pini
#define DHTPIN D3
#define DHTTYPE DHT22
#define PHOTORH A0
#define WIFI_LED D0
#define PIR D1
#define BUZZER D2
#define LED_R D5
#define LED_G D6
#define LED_B D7


// Token de autentificare pentru Blynk
char auth[] = "Put Your Token Here";
 
// WiFi LoginData
char ssid[] = "NSA Internal Network";
char pass[] = "TopSecretPassword";



// Variabile de stare
unsigned long lastMove;
int alarma;
int notificari;
int sonerie;

// ServerNTP
WiFiUDP ntpUDP2, ntpUDP3;
const long gmt2 = 7200, gmt3 = 10800;
NTPClient timeClient2(ntpUDP2, "pool.ntp.org", gmt2);
NTPClient timeClient3(ntpUDP3, "pool.ntp.org", gmt3);

// Blynk
WidgetLED led(V3);
WidgetLED movement(V2);
BlynkTimer timer;

// Senzor DHT
DHT dht(DHTPIN, DHTTYPE);


// Scrie date pe pini virtuali despre temperatura, umiditate si lumina
void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
 
  if (isnan(h) || isnan(t)) {
    return;
  }

  // Scrie Temperatura si Umiditate
  Blynk.virtualWrite(V5, t);
  Blynk.virtualWrite(V6, h);

  // Scrie valoare Photorezistor
  Blynk.virtualWrite(V4, analogRead(PHOTORH));

  if (analogRead(A0) >= 450) {
    led.on();
  } else {
    led.off();
  }
}


// Function that gets current epoch time for GMT+2
unsigned long getTime2() {
  timeClient2.update();
  unsigned long now = timeClient2.getEpochTime();
  return now;
}


// Function that gets current epoch time for GMT +3
unsigned long getTime3() {
  timeClient3.update();
  unsigned long now = timeClient3.getEpochTime();
  return now;
}


// Functie pentru a activa alarma de times ori, cu pauze de duration
// intre activari/dezactivari. Durata se da in ms.
void buzzerRing(unsigned int times, unsigned int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER, HIGH);
    Blynk.virtualWrite(V10, HIGH);
    delay(duration);
    digitalWrite(BUZZER, LOW);
    Blynk.virtualWrite(V10, LOW);
    delay(duration);
  } 
}


void setup() {
  // LED-ul albastru va sta permanent aprins pana cand se va
  // finaliza procedura de boot.
  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, HIGH);

  // Initializare variabile pentru timp
  timeClient2.begin();
  timeClient3.begin();
  lastMove = getTime2();

  // Initializare senzor temperatura/umiditate
  dht.begin();

  // Conectare la serverele Blynk si trimitere notificare
  Blynk.begin(auth, ssid, pass);
  Blynk.notify("BucharestESP is up!");
 
  // Vom apela functie sendSensor care trimite datele de la senzori
  // de 10 ori pe secunda
  timer.setInterval(100, sendSensor);

  // Setare pini de I/O
  pinMode(PHOTORH, INPUT);
  pinMode(PIR, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Semnalizam faptul ca s-a incheiat procedura de boot printr-un
  // beep dat de alarma plus prin stingerea LED-ului albastru
  buzzerRing(1, 500);
  digitalWrite(WIFI_LED, LOW);
}


// Actualizare stare alarma
BLYNK_WRITE(V1)
{
  alarma = param.asInt();

  // Pentru alarma activeaza si notificarile
  if (alarma) {
    Blynk.virtualWrite(V11, HIGH);
    notificari = 1;
  } else {
    Blynk.virtualWrite(V11, LOW);
    notificari = 0;
  }
  if (timeClient2.getHours() < 23 && timeClient2.getHours() > 7) {
    buzzerRing(2, 200);
  } else {
    for (int i = 0; i < 2; i++) {
      digitalWrite(WIFI_LED, HIGH);
      delay(200);
      digitalWrite(WIFI_LED, LOW);
      delay(200);
    }
  }
}


// Actualizare stare sonerie
BLYNK_WRITE(V10)
{
  sonerie = param.asInt();

  if (sonerie) {
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
  }
}


// Actualizare stare notificari miscare
BLYNK_WRITE(V11)
{
  notificari = param.asInt();
}


bool miscare = false;

void loop() {
  Blynk.run();
  timer.run();

  if (digitalRead(PIR)) {
    // S-a detectat miscare
    miscare = true;
    if (getTime2() - lastMove < 15) {
      // Daca ultima miscare a avut loc cu 15 secunde in urma
      // trimite notificare sau porneste alarma daca este cazul
      if (notificari) {
        String text = "S-a detectat miscare! Verifica PIR!";
        text.concat("\n");
        getTime2();
        text.concat(timeClient2.getFormattedTime());
        text.concat("(GMT+2) | ");
        getTime3();
        text.concat(timeClient3.getFormattedTime());
        text.concat("(GMT+3)");
        Blynk.notify(text.c_str());
      }
      
      if (alarma) {
        for (int i = 0; i < 3; i++) {
          // Alarma va suna intre 7 si 22(GMT+2) sau 8 si 23(GMT+3) altfel LED
          if (timeClient2.getHours() < 23 && timeClient2.getHours() > 7) {
            digitalWrite(BUZZER, HIGH);
          } else {
            digitalWrite(WIFI_LED, HIGH);
          }
          Blynk.virtualWrite(V10, HIGH);
          delay(100);
          digitalWrite(BUZZER, LOW);
          Blynk.virtualWrite(V10, LOW);
          digitalWrite(WIFI_LED, LOW);
          delay(100);
        }
      }

      Blynk.virtualWrite(V3, HIGH);
      // Actualizeaza timpul ultimei miscari
      lastMove = getTime2();
    }
    // LED miscare Blynk
    movement.on();
  } else {
    // Daca ultima stare a senzorului PIR a fost miscare
    // si nu se mai detecteaza miscare, seteaza ca false
    // si actualizeaza timpul ultimei miscari
    if (miscare) {
      lastMove = getTime2();
      miscare = false;
      Blynk.virtualWrite(V3, LOW);
    }
    // LED miscare Blynk
    movement.off();
  }
}
