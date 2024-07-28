#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include "CTBot.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

CTBot myBot;
TBMessage msg;

int m_servo = 15;
#define Heater 33
#define LDR1 34
#define RAIN1 35
#define ENA 27
#define IN1 25
#define IN2 26
#define buzzer 14
#define DHTPIN 17
#define DHTTYPE DHT22
#define LED 16
#define ssid ""
#define pass ""
#define token ""
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

void setup()
{
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  dht.begin();
  Serial.println(F("baca sensor DHT22"));
  myservo.attach(m_servo);
  pinMode(LDR1, INPUT);
  pinMode(RAIN1, INPUT);
  pinMode(Heater, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(LED, OUTPUT);
  myservo.write(0);

  sensor_t sensor;
  // pembacaan suhu
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("°C"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("°C"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // pembacaan kelembaban
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("%"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("%"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("%"));
  Serial.println(F("------------------------------------"));

  delayMS = sensor.min_delay / 1000;

  Serial.println("\nStarting TelegramBot...");
  Serial.print("Menyambungkan ke : ");
  Serial.println(ssid);
  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);

  if (myBot.testConnection())
  {
    digitalWrite(buzzer, HIGH);
    delay(700);
    digitalWrite(buzzer, LOW);
    Serial.print("Terhubung dengan : ");
    Serial.println(ssid);
  }
  else
  {
    Serial.println("\nError.");
  }
}

int cahaya_s1()
{
  return analogRead(LDR1);
}
int cuaca_s1()
{
  return analogRead(RAIN1);
}

void loop()
{
  int light1 = cahaya_s1();
  int weather1 = cuaca_s1();

  if (light1 < 0 || light1 > 4095 || weather1 < 0 || weather1 > 4095)
    return;
  sensors_event_t event;

  if (light1 >= 3600 && weather1 >= 3800)
  {

    myservo.write(0); // tutup servonya
    delay(500);
    Serial.print("servo menutup");
    myBot.sendMessage(msg.sender.id, "Cahaya Gelap dan Tidak Hujan, MALAM HARI");

    if (isnan(event.temperature >= 55 && event.relative_humidity <= 40))
    {                   // Jagung kering
      myservo.write(0); // menutup
      Serial.print("servo menutup");
      myBot.sendMessage(msg.sender.id, "Jagung sudah kering, MALAM HARI servo menutup");
      delay(500);
      digitalWrite(LED, LOW);
      digitalWrite(Heater, LOW);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(1000);
      tone(buzzer, 2637);
      delay(500);
      tone(buzzer, 2960);
      delay(500);
      digitalWrite(buzzer, LOW);
    }
    else
    {
      myservo.write(0);
      myBot.sendMessage(msg.sender.id, "Jagung masih belum kering, MALAM HARI, Servo menutup");
      delay(500);

      digitalWrite(Heater, HIGH);
      digitalWrite(LED, HIGH);

      analogWrite(ENA, 200);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);

      analogWrite(ENA, 180);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);
    }
  }

  else if (light1 <= 3600 && weather1 >= 3800)
  { // TERANG TAK HUJAN
    myservo.write(110);
    Serial.print("servo membuka");
    myBot.sendMessage(msg.sender.id, "Cahaya Terang dan Tidak Hujan");
    delay(500);

    if (isnan(event.temperature >= 55 && event.relative_humidity <= 40))
    { // Jagung Kering
      myservo.write(0);
      Serial.print("servo menutup");
      myBot.sendMessage(msg.sender.id, "Jagung sudah Kering, servo menutup");
      delay(500);

      digitalWrite(LED, LOW);

      tone(buzzer, 2637);
      delay(500);
      tone(buzzer, 2960);
      delay(500);
      digitalWrite(buzzer, LOW);
    }
    else
    { // Jagung Basah
      myservo.write(110);
      myBot.sendMessage(msg.sender.id, "Jagung Belum kering, servo membuka");
      delay(500);

      digitalWrite(LED, LOW);

      analogWrite(ENA, 200);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);

      analogWrite(ENA, 180);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);
    }
  }

  else if (light1 >= 2400 && light1 <= 3600 && weather1 >= 3800)
  { // MENDUNG dan TIDAK hujan
    myservo.write(70);
    Serial.print("servo membuka");
    myBot.sendMessage(msg.sender.id, "Cahaya mendung dan Tidak Hujan");
    delay(500);

    if (isnan(event.temperature >= 55 && event.relative_humidity <= 40))
    {                   // Jagung Kering
      myservo.write(0); // servo menutup
      myBot.sendMessage(msg.sender.id, "Jagung sudah kering, servo menutup");
      delay(500);

      digitalWrite(LED, LOW);

      tone(buzzer, 2637);
      delay(500);
      tone(buzzer, 2960);
      delay(500);
      digitalWrite(buzzer, LOW);
    }
    else
    {                    // Jagung Basah
      myservo.write(70); // srvo buka
      myBot.sendMessage(msg.sender.id, "Jagung belum kering, servo membuka");
      delay(500);
      digitalWrite(LED, LOW);

      analogWrite(ENA, 200);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);

      analogWrite(ENA, 180);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);
    }
  }

  else if (light1 >= 2400 && light1 >= 3600 && weather1 <= 3800)
  {                   // GELAP DAN HUJAN ATAU Mendung DAN HUJAN
    myservo.write(0); // tutup servonya
    myBot.sendMessage(msg.sender.id, "Cahaya Gelap dan Hujan deras");
    Serial.print("servo menutup");
    delay(500);

    if (isnan(event.temperature >= 55 && event.relative_humidity <= 40))
    {                   // Jagung kering
      myservo.write(0); // tutup servonya
      Serial.print("servo menutup");
      myBot.sendMessage(msg.sender.id, "Jagung sudah kering, servo menutup");
      delay(500);

      digitalWrite(LED, LOW);

      tone(buzzer, 2637);
      delay(500);
      tone(buzzer, 2960);
      delay(500);
      digitalWrite(buzzer, LOW);
    }

    else
    {                   // Jagung basah
      myservo.write(0); // tutup servonya
      myBot.sendMessage(msg.sender.id, "Jagung belum kering, servo menutup, pemanas menyala");
      delay(500);

      digitalWrite(Heater, HIGH);
      digitalWrite(LED, HIGH);

      analogWrite(ENA, 200);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);

      analogWrite(ENA, 180);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);
    }
  }

  else if (light1 <= 2400 && weather1 <= 3800)
  {                    // TERANG DAN HUJAN
    myservo.write(25); // tutup servonya
    myBot.sendMessage(msg.sender.id, "Cahaya Terang dan Hujan deras, servo menutup");
    Serial.print("servo menutup");
    delay(500);

    if (isnan(event.temperature >= 55 && event.relative_humidity <= 40))
    { // Jagung kering
      myservo.write(0);
      Serial.print("servo menutup");
      myBot.sendMessage(msg.sender.id, "Jagung sudah kering, servo menutup");
      delay(500);
      digitalWrite(LED, LOW);

      tone(buzzer, 2637);
      delay(500);
      tone(buzzer, 2960);
      delay(500);
      digitalWrite(buzzer, LOW);
    }
    else
    { // Jagung basah
      myservo.write(25);
      myBot.sendMessage(msg.sender.id, "Jagung belum kering, servo menutup, pemanas menyala");
      delay(500);

      digitalWrite(Heater, HIGH);
      digitalWrite(LED, HIGH);

      analogWrite(ENA, 200);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);

      analogWrite(ENA, 180);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      delay(700);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(ENA, LOW);
      delay(700);
    }
  }

  if (myBot.getNewMessage(msg))
  {
    if (msg.text.equalsIgnoreCase("/start"))
    {
      Serial.println("Mengirim pesan ke Telegram...");
      myBot.sendMessage(msg.sender.id, "Selamat Datang Boss");
    }

    else
    {
      Serial.println("Mengirim pesan ke Telegram...");
      String balasan;
      balasan = (String) "Pesan tidak ada!\n" + (String) "Silahkan cek kembali dengan\n" + (String) "mengirim pesan /start.";
      myBot.sendMessage(msg.sender.id, balasan);
    }
    delay(100);
  }

  Serial.println("LDR");
  Serial.print(light1);
  Serial.println(" OHM");

  Serial.println("RAIN");
  Serial.print(weather1);
  Serial.println(" OHM");

  lcd.setCursor(0, 0);
  lcd.print("L1");
  lcd.setCursor(3, 0);
  lcd.print(light1);
  lcd.setCursor(5, 0);
  lcd.print("R1");
  lcd.setCursor(7, 0);
  lcd.print(weather1);
  delay(500);

  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println(F("Error reading temperature!"));
  }
  else
  {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));

    lcd.setCursor(12, 0);
    lcd.print("T");
    lcd.setCursor(14, 0);
    lcd.print(event.temperature);
    delay(500);
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println(F("Error reading humidity!"));
  }
  else
  {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));

    lcd.setCursor(12, 1);
    lcd.print("H");
    lcd.setCursor(14, 1);
    lcd.print(event.relative_humidity);
    delay(500);
  }
  delay(500);
}
