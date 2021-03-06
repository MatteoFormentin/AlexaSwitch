#include "ButtonDebounce.h"
#include "fauxmoESP.h"

#define HOSTNAME "AlexaSwitch"

Button button;
fauxmoESP fauxmo;

const int relais_pin = 12;
const int led_pin = 13;
bool relais_state = LOW;

unsigned long int start_time;

void setup()
{
  Serial.begin(9600);
  Serial.println("test!");
  pinMode(relais_pin, OUTPUT);
  pinMode(led_pin, OUTPUT);
  digitalWrite(relais_pin, relais_state);
  digitalWrite(led_pin, !relais_state);

  button.begin(0); //D3->gpio0
  button.addShortPressCallback(&toogleRelais);
  button.addLongPressCallback(&factoryReset, 10000);

  setupWiFi();
  setupWebInterface();
  if (!getApMode())
  {
    initAlexa();
  }

  start_time = millis();
}

void loop()
{
  if (getApMode())
  {
    if (millis() - start_time > 36000)
    {
      ESP.restart();
    }
  } else {
    start_time = millis();
  }

  if (!getApMode())
  {
    fauxmo.handle();
  }
  button.buttonLoop();
  handleWebInterface();
  if (!checkWiFi())
  {
    setupWiFi();
  }
}

void toogleRelais()
{
  relais_state = !relais_state;
  digitalWrite(relais_pin, relais_state);
  digitalWrite(led_pin, !relais_state);
}

void initAlexa()
{
  fauxmo.createServer(true);
  fauxmo.setPort(80);
  fauxmo.enable(true);
  fauxmo.addDevice(HOSTNAME);
  fauxmo.onSetState([](unsigned char device_id, const char *device_name, bool state, unsigned char value) {
    if (strcmp(device_name, HOSTNAME) == 0)
    {
      if (state == 1)
      {
        relais_state = HIGH;
      }
      else
      {
        relais_state = LOW;
      }

      digitalWrite(relais_pin, relais_state);
      digitalWrite(led_pin, !relais_state);
    }
  });

  Serial.println("Alexa init ok");
}
