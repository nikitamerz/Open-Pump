//Get the WiFi settings from the SPIFFS memory
//======================================================
void GetWiFiJSON() {
  File wifi_file = SPIFFS.open("/wifi.json", "r");
  if (!wifi_file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  String wifi_string = wifi_file.readString();
  deserializeJson(WiFiJSON, wifi_string);
  wifi_file.close();

  ssid = (const char*)WiFiJSON["SSID"];
  password = (const char*)WiFiJSON["PASS"];
  mqtt_server = (const char*)WiFiJSON["SERVER"];
  mqtt_user = (const char*)WiFiJSON["MQTTUser"];
  mqtt_pass = (const char*)WiFiJSON["MQTTPass"];
  TopicINFO = (const char*)WiFiJSON["TopicINFO"];
  TopicDEBUG = (const char*)WiFiJSON["TopicDEBUG"];
  TopicCONFIG = (const char*)WiFiJSON["TopicCONFIG"];
  TopicCMD = (const char*)WiFiJSON["TopicCMD"];
  HostName = (const char*)WiFiJSON["HostName"];
  
  Serial.println("WiFi JSON loaded");
  // ДОБАВИЛИ ОТЛАДКУ: чтобы точно видеть, какой IP читает плата из файла!
  Serial.print("MQTT Server from JSON: ");
  Serial.println(mqtt_server); 
}
//======================================================

//Connect to the WiFi
//=======================================
void setup_wifi() {
  delay(10);
  Serial.println();
  leds[0] = CRGB( 0, 0, 0);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HostName);
  WiFi.begin(ssid, password);
  
  // МЫ УБРАЛИ цикл MaxWiFiTry. 
  // Теперь плата просто дает команду на подключение. 
  // Всю проверку и бесконечные попытки берет на себя функция reconnect()
  Serial.println("WiFi connection process started...");
}
//=======================================

//Reconnect to the WiFi and MQTT (НЕ БЛОКИРУЮЩАЯ ВЕРСИЯ)
//=========================================================
void reconnect() {
  // 1. Сначала проверяем Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("WiFi lost. Reconnecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    WiFiConnected = false;
    delay(1000); 
    return; // Выходим из функции, чтобы не блокировать основной цикл. Попробуем снова через 1 сек.
  }

  // 2. Если Wi-Fi есть, проверяем MQTT (используем if, а НЕ while!)
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.println(mqtt_server);
    WiFiConnected = true;
    
    if (client.connect(HostName, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(TopicCMD);
      client.subscribe(TopicCONFIG);
      leds[1] = CRGB( 0, 10, 0); // Зеленый: всё подключено
      FastLED.show();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      leds[1] = CRGB( 10, 0, 0); // Красный: ошибка
      FastLED.show();
      delay(5000); // Ждем 5 секунд, но функция завершается и возвращает управление в loop()
    }
  }
}
//=========================================================