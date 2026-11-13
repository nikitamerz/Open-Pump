void RedLED(int LED) {
  leds[LED] = CRGB( 50, 0, 0);
  FastLED.show();
  PrevMil2 = millis()+LEDOffDelay;
}

void GreenLED(int LED) {
  leds[LED] = CRGB( 0, 50, 0);
  FastLED.show();
  PrevMil2 = millis()+LEDOffDelay;
}

void BlueLED(int LED) {
  leds[LED] = CRGB( 0, 0, 50);
  FastLED.show();
  PrevMil2 = millis()+LEDOffDelay;
}

void OrangeLED(int LED) {
  leds[LED] = CRGB( 50, 50, 0);
  FastLED.show();
  PrevMil2 = millis()+LEDOffDelay;
}

void WhiteLED(int LED) {
  leds[LED] = CRGB( 10, 10, 10);
  FastLED.show();
  PrevMil2 = millis()+LEDOffDelay;
}
