#include <ESP8266WiFi.h>
#include <NeoPixelFX.h>
#include <ESPAsyncWebServer.h>

#define LED_COUNT 71
#define LED_PIN 5

NeoPixelFX fx = NeoPixelFX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "";
const char* password = "";

AsyncWebServer server(80);
String header;

void process_command(const char * cmd) {
  if (strcmp(cmd,"b+") == 0) {
    fx.increaseBrightness(25);
    Serial.print(F("Increased brightness by 25 to: "));
    Serial.println(fx.getBrightness());
  }

  if (strcmp(cmd,"b-") == 0) {
    fx.decreaseBrightness(25); 
    Serial.print(F("Decreased brightness by 25 to: "));
    Serial.println(fx.getBrightness());
  }

  if (strncmp(cmd,"b ",2) == 0) {
    uint8_t b = (uint8_t)atoi(cmd + 2);
    fx.setBrightness(b);
    Serial.print(F("Set brightness to: "));
    Serial.println(fx.getBrightness());
  }

  if (strcmp(cmd,"s+") == 0) {
    fx.setSpeed(fx.getSpeed() * 1.2);
    Serial.print(F("Increased speed by 20% to: "));
    Serial.println(fx.getSpeed());
  }

  if (strcmp(cmd,"s-") == 0) {
    fx.setSpeed(fx.getSpeed() * 0.8);
    Serial.print(F("Decreased speed by 20% to: "));
    Serial.println(fx.getSpeed());
  }

  if (strncmp(cmd,"s ",2) == 0) {
    uint16_t s = (uint16_t)atoi(cmd + 2);
    fx.setSpeed(s); 
    Serial.print(F("Set speed to: "));
    Serial.println(fx.getSpeed());
  }

  if (strncmp(cmd,"m ",2) == 0) {
    uint8_t m = (uint8_t)atoi(cmd + 2);
    fx.setMode(m);
    Serial.print(F("Set mode to: "));
    Serial.print(fx.getMode());
    Serial.print(" - ");
    Serial.println(fx.getMode());
  }

  if (strncmp(cmd,"c ",2) == 0) {
    uint32_t c = (uint32_t)strtoul(cmd + 2, NULL, 16);
    fx.setColor(c);
    Serial.print(F("Set color to: 0x"));
    Serial.println(fx.getColor(), HEX);
  }
}


void setup() {
  Serial.begin(9600);
  fx.init();
  Serial.println(fx.getSpeed());
  fx.setColor(0x00FF00);
  fx.setMode(0);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting.");
  }

  Serial.println("Connected!");

  Serial.println(WiFi.localIP());

  server.on("/sendCommand", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (strcmp(p->name().c_str(), "command") == 0) {
        process_command(p->value().c_str());
        break;
      } else {
        request->send(4040, "text/plain", "Invalid Arg");
      }
    }
    request->send(200, "text/plain", "Command executed");
  });

  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
    request->send(200, "text/plain", "pong");
  });
  Serial.println(fx.getSpeed());
  server.begin();
}

void loop() {
  fx.service();
}