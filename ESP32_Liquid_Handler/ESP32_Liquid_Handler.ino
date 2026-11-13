// Liquid handler V2.0 Made by Alain Gervasi
//
// Licensed under the Creative Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//Required hardware modification : Cut the PIN12 on the Shield and solder it with the adjacent PIN13

//================================
#include <Wire.h>
#include <Arduino.h>
long unsigned int PrevMil = 0;
long unsigned int PrevMil2 = 0;
int StepOffDelay = 5000; //Turn off the motors after X ms
int LEDOffDelay = 1000; //Turn off the LED after X ms
int BackPush = 1000; //Tensioning after syringe homing (in steps)
int MaxWiFiTry = 50; //Max tries before disconnecting the WiFi
int calstep = 0;
String out;
bool WiFiConnected = false;
String SMot = "N";
String Cmd = "0";
volatile bool EndStop = false;
//================================

//Available GPIO pin on the Shield
//============================================================
#define ZPLUS 23 //(VSPI MOSI)
#define YPLUS 5 //(VSPI CS0)
#define SPNEN 19 //(VSPI MISO)
#define SPNDIR 18 //(VSPI CLK)
#define COOEN 34 //(InputOnly/RTC_GPIO4/ADC1_CH6)
#define ABORT 2 //(ADC2_CH1/Touch2/RTC_GPIO12)
#define HOLD 4 // (ADC2_CH0/Touch0/RTC_GPIO10)
#define RESUME 35 //(InputOnly/RTC_GPIO5/ADC1_CH7)
#define SDA 36 //(InputOnly/Sensor_VN/RTC_GPIO3/ADC1_CH3)
#define SCL 39 //(InputOnly/RTC_GPIO5/ADC1_CH7)
#define RST 0 //(ADC2_CH1/Touch1/RTC_GPIO11)
//============================================================

//WiFi and MQTT
//===============================
#include <WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
//===============================

//SPIFF
//=============================================================================================================================
#include <ArduinoJson.h> //ArduinoJSON6
#include "SPIFFS.h" //The SPIFFS memory is used to save the wifi.JSON and config.JSON files 
DynamicJsonDocument ModulesInfos(2048);
DynamicJsonDocument WiFiJSON(2048);
const char *ssid, *password, *mqtt_server, *mqtt_user, *mqtt_pass, *TopicINFO, *TopicDEBUG, *TopicCONFIG, *TopicCMD, *HostName;
//=============================================================================================================================

//Debug LED
//===================
#include <FastLED.h>
#define NUM_LEDS 3
#define DATA_PIN 5
CRGB leds[NUM_LEDS];
//===================

//========================================================================================================================
class StepperDriver {
  public:
    short dir_pin, step_pin, en_pin;
    int delay_step = 500;
    StepperDriver(): dir_pin(0), step_pin(0), en_pin(0) {}
    StepperDriver(short dir_pin, short step_pin, short en_pin) : dir_pin(dir_pin), step_pin(step_pin), en_pin(en_pin) {}

    void begin(int rpm) {
      pinMode(dir_pin, OUTPUT);
      pinMode(step_pin, OUTPUT);
      pinMode(en_pin, OUTPUT);
      delay_step = (60000000 / (200 * rpm)) / 2; //Conversion from RPM to µs/step
    }

    void enable(void) {
      digitalWrite(en_pin, LOW);
    }

    void disable(void) {
      digitalWrite(en_pin, HIGH);
    }

    void move(long steps) {
      PrevMil = millis();
      if (steps > 0) {
        digitalWrite(dir_pin, LOW);
      }
      else {
        digitalWrite(dir_pin, HIGH);
        steps = -steps;
      }
      for (int i = 0; i < steps; i++) {
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(delay_step);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(delay_step);
        client.loop(); //Maintain the MQTT connection even if the motor is activated for a long period of time
      }
    }

    long calibrate(int endstop_pin, bool dir = true) {
      //dir = true => Clockwise calibration / dir = flase => Counter clockwise calibration
      digitalWrite(en_pin, LOW);
      bool endstop = true;
      long calstep = 0;
      if (dir == true) {
        digitalWrite(dir_pin, LOW);
      }
      else {
        digitalWrite(dir_pin, HIGH);
      }
      while (endstop == true) {
        if (digitalRead(endstop_pin) == LOW) {
          endstop = false;
        }
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(delay_step);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(delay_step);
        calstep += 1;
        client.loop(); //Maintain the MQTT connection even if the motor is activated for a long period of time
      }
      digitalWrite(en_pin, LOW);
      return (calstep);
    }
};
#define RPM 200
StepperDriver StepperX(16, 26, 13);
StepperDriver StepperY(27, 25, 13);
StepperDriver StepperZ(14, 17, 13);
//========================================================================================================================

//Syringe EndStop interrupt (button connected to ZPlus)
void IRAM_ATTR HomeEnd() {
  StepperX.disable();
  EndStop = false;
  StepperX.enable();
  leds[2] = CRGB( 0, 0, 50);
  PrevMil2 = millis() + LEDOffDelay;
}

void EnsureConfigFiles() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed, formatting...");
    SPIFFS.format();
    if (!SPIFFS.begin(true)) { Serial.println("SPIFFS dead"); return; }
  }
  
  // ВСЕГДА перезаписываем wifi.json с правильными настройками
  File f = SPIFFS.open("/wifi.json", "w");
  if (f) {
    f.print("{\"SSID\":\"MERCUSYS_34E4\",\"PASS\":\"14402357\",\"SERVER\":\"192.168.1.105\",\"MQTTUser\":\"\",\"MQTTPass\":\"\",\"TopicINFO\":\"botabot/lab/liquidhanler/01/INFO\",\"TopicDEBUG\":\"botabot/lab/liquidhanler/01/DEBUG\",\"TopicCONFIG\":\"botabot/lab/liquidhanler/01/CONFIG\",\"TopicCMD\":\"botabot/lab/liquidhanler/01/CMD\",\"HostName\":\"ESP32_LIQUID_HANDLER_01\"}");
    f.close();
    Serial.println("wifi.json created/updated with SERVER: 192.168.1.105");
  }
  
  if (!SPIFFS.exists("/config.json")) {
    File f = SPIFFS.open("/config.json", "w");
    if (f) {
      f.print("{\"X\":\"P\",\"XC\":-1,\"Y\":\"N\",\"YC\":-1,\"Z\":\"N\",\"ZC\":-1,\"SC\":1.25,\"SM\":8000}");
      f.close();
      Serial.println("config.json created");
    }
  }
}

//SETUP===========================================================
void setup() {
  Serial.begin(115200);
  EnsureConfigFiles();
  Serial.println();
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

  //Get config from SPIFFS memory
  //----------------
  GetConfigModule();
  GetWiFiJSON();
  setup_wifi();
  //----------------

  //MQTT
  //----------------------------------
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //----------------------------------

  //EndStop
  //-----------------------------
  pinMode(ZPLUS, INPUT_PULLUP);
  //-----------------------------

  //Steppers
  //-------------------
  StepperX.begin(RPM);
  StepperY.begin(RPM);
  StepperZ.begin(RPM);
  //-------------------
}
//SETUP===========================================================

//LOOP============================================================
void loop() {
  //MQTT Управление подключением (УПРОЩЁННОЕ И НАДЁЖНОЕ)
  //---------------------------
  // Если Wi-Fi подключен, пытаемся держать связь с MQTT
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect(); // Эта функция теперь умная и не блокирует код
    }
    client.loop(); // Обработка входящих сообщений
  }
  //---------------------------

  //Disable stepper after StepOffDelay ms to prevent unnecessary power consumption
  //---------------------------------------
  if (millis() - PrevMil > StepOffDelay) {
    PrevMil = millis();
    StepperX.disable();
    StepperY.disable();
    StepperZ.disable();
  }
  //---------------------------------------

  //Turn off debug LED
  //--------------------------
  if (millis() > PrevMil2) {
    leds[0] = CRGB( 0, 0, 0);
    leds[2] = CRGB( 0, 0, 0);
    FastLED.show();
  }
  //--------------------------

  //Read commands sent by serial
  //--------------------------------------------------
  if (Serial.available()) {
    String input = Serial.readString();
    String topic = input.substring(0, 1);
    String command = input.substring(1);

    if (topic == "S") {
      response(TopicCONFIG, command);
    }
    else if (topic == "C") {
      response(TopicCMD, command);
    }
    else if (topic == "H") {
      Serial.println("====================================");
      Serial.println("Add tool : \"S\" + Slot + Tool");
      Serial.println("Example : SXP, SYS or SZN (N = null)");
      Serial.println("====================================");
      Serial.println();
      Serial.println("====================================================");
      Serial.println("Pump calibration step 1 : \"C\" + Slot + \"C\" Ex : CXC");
      Serial.println("Example : CXC");
      Serial.println();
      Serial.println("Pump calibration step 2 : \"C\" + Slot + \"C\" + Volume");
      Serial.println("Example : CXC10 (if calibration volume = 10ml)");
      Serial.println("====================================================");
      Serial.println();
      Serial.println("========================================================================");
      Serial.println("Syringe calibration : \"C\" + Slot + \"C\" ");
      Serial.println("Example : CYC (a calibration volume is not applicable for the syringe)");
      Serial.println("========================================================================");
    }
    else if (topic == "I") {
      ConfigModules("INFO");
    }
    else if (topic == "M") { // RAW-тест моторов
      String slot = command.substring(0, 1);
      long steps = command.substring(1).toInt();
      if (slot == "X") { StepperX.enable(); StepperX.move(steps); Serial.println("X moved"); }
      else if (slot == "Y") { StepperY.enable(); StepperY.move(steps); Serial.println("Y moved"); }
      else if (slot == "Z") { StepperZ.enable(); StepperZ.move(steps); Serial.println("Z moved"); }
      else { Serial.println("ERROR ! Slot unknown"); }
    }
    else {
      Serial.println("ERROR ! Unrecognized command");
    }
  }
  //--------------------------------------------------

  yield();
}
//LOOP============================================================