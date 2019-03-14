#include "ButtonDebounce.h"
#include "fauxmoESP.h"
 
#define HOSTNAME "AlexaSwitch"

Button button;
fauxmoESP fauxmo;

const int relais_pin = 12;
const int led_pin = 13;
bool relais_state = HIGH;

void setup()
{
  Serial.begin(9600);
  Serial.println("test!");
  pinMode(relais_pin, OUTPUT);
  pinMode(led_pin, OUTPUT);
  digitalWrite(relais_pin, relais_state);

  //Blink led
  button.begin(0); //D3->gpio0
  button.addShortPressCallback(&toogleRelais);
  button.addLongPressCallback(&factoryReset, 10000);

  setupWiFi();
  if (!getApMode())
  {
    initAlexa();
  }
  
}

void loop()
{
  if (!getApMode())
  {
    fauxmo.handle();
  }
  button.buttonLoop();
  handleWebInterface();
}

void toogleRelais()
{
  relais_state = !relais_state;
  digitalWrite(relais_pin, relais_state);
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
        digitalWrite(relais_pin, LOW);
      }
      else
      {
        digitalWrite(relais_pin, HIGH);
      }
    }
  });
  Serial.println("Alexa init ok");
}
