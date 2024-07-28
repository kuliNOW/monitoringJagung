#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include "CTBot.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Fuzzy.h>
#include <FuzzyComposition.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzyOutput.h>
#include <FuzzyRule.h>
#include <FuzzySet.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

CTBot myBot;
TBMessage msg;

int m_servo = 15;
#define Heater 33
#define LDR 34
#define RAIN 35
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

// object library
Fuzzy *fuzzy = new Fuzzy();

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
  pinMode(LDR, INPUT);
  pinMode(RAIN, INPUT);
  pinMode(Heater, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(LED, OUTPUT);
  myservo.write(0);

  // Define fuzzy sets
  FuzzySet *cerah = new FuzzySet(0, 0, 1200, 2400);
  FuzzySet *mendung = new FuzzySet(1200, 2400, 2400, 3600);
  FuzzySet *gelap = new FuzzySet(2400, 3600, 4095, 4095);

  FuzzySet *hujan = new FuzzySet(0, 0, 3000, 3900);
  FuzzySet *tidakhujan = new FuzzySet(3000, 3900, 4095, 4095);

  FuzzySet *buka = new FuzzySet(60, 90, 110, 180);
  FuzzySet *tutup = new FuzzySet(0, 0, 40, 60);

  // Define fuzzy inputs
  FuzzyInput *cahaya = new FuzzyInput(1);
  cahaya->addFuzzySet(cerah);
  cahaya->addFuzzySet(mendung);

  cahaya->addFuzzySet(gelap);
  fuzzy->addFuzzyInput(cahaya);

  FuzzyInput *cuaca = new FuzzyInput(2);
  cuaca->addFuzzySet(hujan);
  cuaca->addFuzzySet(tidakhujan);
  fuzzy->addFuzzyInput(cuaca);

  // Define fuzzy outputs
  FuzzyOutput *buka_o = new FuzzyOutput(1);
  buka_o->addFuzzySet(buka);
  buka_o->addFuzzySet(tutup);

  fuzzy->addFuzzyOutput(buka_o);

  // Define rules
  // Rule 1
  FuzzyRuleAntecedent *ifCahayaCerahlAndCuacaIsHujan = new FuzzyRuleAntecedent();
  ifCahayaCerahlAndCuacaIsHujan->joinWithAND(cerah, hujan);
  FuzzyRuleConsequent *thenbuka_o1 = new FuzzyRuleConsequent();
  thenbuka_o1->addOutput(tutup);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, ifCahayaCerahlAndCuacaIsHujan, thenbuka_o1);
  fuzzy->addFuzzyRule(fuzzyRule1);

  // Rule 2
  FuzzyRuleAntecedent *ifCahayaCerahAndCuacaIsTidakHujan = new FuzzyRuleAntecedent();
  ifCahayaCerahAndCuacaIsTidakHujan->joinWithAND(cerah, tidakhujan);
  FuzzyRuleConsequent *thenbuka_o2 = new FuzzyRuleConsequent();
  thenbuka_o2->addOutput(buka);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, ifCahayaCerahAndCuacaIsTidakHujan, thenbuka_o2);
  fuzzy->addFuzzyRule(fuzzyRule2);

  // Rule 3
  FuzzyRuleAntecedent *ifCahayaMendungAndCuacaIsHujan1 = new FuzzyRuleAntecedent();
  ifCahayaMendungAndCuacaIsHujan1->joinWithAND(mendung, hujan);
  FuzzyRuleConsequent *thenbuka_o3 = new FuzzyRuleConsequent();
  thenbuka_o3->addOutput(tutup);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, ifCahayaMendungAndCuacaIsHujan1, thenbuka_o3);
  fuzzy->addFuzzyRule(fuzzyRule3);

  // Rule 4
  FuzzyRuleAntecedent *ifCahayaMendungAndCuacaIstidakHujan1 = new FuzzyRuleAntecedent();
  ifCahayaMendungAndCuacaIstidakHujan1->joinWithAND(mendung, tidakhujan);
  FuzzyRuleConsequent *thenbuka_o4 = new FuzzyRuleConsequent();
  thenbuka_o4->addOutput(buka);
  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, ifCahayaMendungAndCuacaIstidakHujan1, thenbuka_o4);
  fuzzy->addFuzzyRule(fuzzyRule4);

  // Rule 5
  FuzzyRuleAntecedent *ifCahayaGelapAndCuacaIsHujan = new FuzzyRuleAntecedent();
  ifCahayaGelapAndCuacaIsHujan->joinWithAND(gelap, hujan);
  FuzzyRuleConsequent *thenbuka_o5 = new FuzzyRuleConsequent();
  thenbuka_o5->addOutput(tutup);
  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, ifCahayaGelapAndCuacaIsHujan, thenbuka_o5);
  fuzzy->addFuzzyRule(fuzzyRule5);

  // Rule 6
  FuzzyRuleAntecedent *ifCahayaGelapAndCuacaIstidakHujan = new FuzzyRuleAntecedent();
  ifCahayaGelapAndCuacaIstidakHujan->joinWithAND(gelap, tidakhujan);
  FuzzyRuleConsequent *thenbuka_o6 = new FuzzyRuleConsequent();
  thenbuka_o6->addOutput(tutup);
  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, ifCahayaGelapAndCuacaIstidakHujan, thenbuka_o6);
  fuzzy->addFuzzyRule(fuzzyRule6);

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
  return analogRead(LDR);
}
int cuaca_s1()
{
  return analogRead(RAIN);
}

void loop()
{
  // put your main code here, to run repeatedly:

  int light1 = cahaya_s1();
  int weather1 = cuaca_s1();

  if (light1 < 0 || light1 > 4095 || weather1 < 0 || weather1 > 4095)
    return;

  sensors_event_t event;

  // Fuzzification
  fuzzy->setInput(1, light1);
  fuzzy->setInput(2, weather1);
  fuzzy->fuzzify();

  // Defuzzification
  int output = fuzzy->defuzzify(1);

  if (light1 >= 3600 && weather1 >= 3800)
  {                   // Gelap dan Tidak Hujan
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

  else if (light1 <= 2500 && weather1 >= 3800)
  { // TERANG DAN TIDAK HUJAN
    myservo.write(output);
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
  else if (light1 >= 2400 && light1 <= 3600 || weather1 >= 3800)
  { // MENDUNG dan TIDAK hujan
    myservo.write(output);
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
    {                        // Jagung Basah
      myservo.write(output); // srvo buka
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

  else if (light1 >= 2400 && light1 <= 3600 || light1 >= 3600 && weather1 <= 3800)
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

  else if (light1 <= 2400 || weather1 <= 3800)
  {                        // TERANG DAN HUJAN
    myservo.write(output); // tutup servonya
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
      myservo.write(output);
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
      balasan = (String) "Pesan tidak ada!\n" +
                (String) "Silahkan cek kembali dengan\n" +
                (String) "mengirim pesan /start.";
      myBot.sendMessage(msg.sender.id, balasan);
    }
    delay(100);
  }
  Serial.print(" => output");
  Serial.print(output);
  Serial.println();
  lcd.setCursor(0, 0);
  lcd.print("O:");
  lcd.setCursor(0, 1);
  lcd.print(output);
  delay(500);

  Serial.println("LDR");
  Serial.print(light1);
  Serial.println(" OHM");

  Serial.println("RAIN");
  Serial.print(weather1);
  Serial.println(" OHM");

  lcd.setCursor(3, 0);
  lcd.print("L");
  lcd.setCursor(5, 0);
  lcd.print(light1);
  lcd.setCursor(3, 1);
  lcd.print("R");
  lcd.setCursor(5, 1);
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

    lcd.setCursor(10, 0);
    lcd.print("T");
    lcd.setCursor(12, 0);
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

    lcd.setCursor(10, 1);
    lcd.print("H");
    lcd.setCursor(12, 1);
    lcd.print(event.relative_humidity);
    delay(500);
  }
  delay(500);
}