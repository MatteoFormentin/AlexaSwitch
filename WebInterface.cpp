#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>

#define BUFFER_SIZE 30

ESP8266WebServer webInterface(8080);
ESP8266HTTPUpdateServer httpUpdater;

boolean ap_mode = false;

struct APConf
{
  char ssid[BUFFER_SIZE];
  char password[BUFFER_SIZE];
};

IPAddress ip_conf(192, 168, 1, 1);
IPAddress mask_conf(255, 255, 255, 0);

void setupWiFi()
{
  Serial.println("INIT");
  SPIFFS.begin();
  WiFi.hostname(HOSTNAME);
  WiFi.persistent(false);
  struct APConf ap = readApConfiguration();

  if (strcmp(ap.ssid, "") != 0)
  {
    Serial.println("try to connect to ap");
    WiFi.begin(ap.ssid, ap.password);
    WiFi.mode(WIFI_STA);

    WiFi.waitForConnectResult();
    if (WiFi.status() != WL_CONNECTED)
    {
      ap_mode = true;
      WiFi.disconnect();
    }
    else
    {
      Serial.println("");
      Serial.println(F("WiFi connected"));
      Serial.println(F("IP address: "));
      Serial.println(WiFi.localIP());
    }
  }
  else
  {
    ap_mode = true;
  }

  if (ap_mode) //If timeout or ssid invalid, enable configuration ap
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("AlexaSwitch Config", "password");
    WiFi.softAPConfig(ip_conf, ip_conf, mask_conf);
    Serial.println();
    Serial.println("Switched to AP mode");
    digitalWrite(led_pin, HIGH);
  }
  delay(100);
}

bool checkWiFi(){
  if (WiFi.status() != WL_CONNECTED)
  {
    return false;
  }
  return true;
}

void handleWebInterface()
{
  webInterface.handleClient();
}

void setupWebInterface()
{
  webInterface.serveStatic("/", SPIFFS, "/ap_conf.html");
  webInterface.on("/ap_conf_submit", handleConfSubmit);
  webInterface.begin();
}

void handleConfSubmit()
{
  String ap_SSID = webInterface.arg("SSID");
  String ap_password = webInterface.arg("password");
  webInterface.send(200, "text/plain", "Device will reboot and try to connect to the given Access Point. You will be disconnected. You can close this page.");
  struct APConf ap;
  ap_SSID.toCharArray(ap.ssid, BUFFER_SIZE);
  ap_password.toCharArray(ap.password, BUFFER_SIZE);
  updateApConfiguration(ap);
}

void updateApConfiguration(struct APConf ap)
{
  Serial.println("Start updating conf");
  if (SPIFFS.exists("/ap.conf"))
  {
    SPIFFS.remove("/ap.conf");
  }
  File conf_file = SPIFFS.open("/ap.conf", "w");
  if (!conf_file)
  {
    Serial.println("error creating file");
  }

  conf_file.println(ap.ssid);
  conf_file.println(ap.password);
  conf_file.close();
  Serial.println("Done");
  ESP.restart();
}

struct APConf readApConfiguration()
{
  Serial.println("Start reading");
  struct APConf ap;
  if (SPIFFS.exists("/ap.conf"))
  {
    File conf_file = SPIFFS.open("/ap.conf", "r");
    if (!conf_file)
    {
      Serial.println("error opening file");
    }
    int read = conf_file.readBytesUntil('\r', ap.ssid, BUFFER_SIZE);
    ap.ssid[read] = '\0';
    conf_file.seek(1, SeekCur);
    read = conf_file.readBytesUntil('\r', ap.password, BUFFER_SIZE);
    ap.password[read] = '\0';
    conf_file.close();
  }
  else
  {
    Serial.println("file not exist");
    ap.ssid[0] = '\0';
    ap.password[0] = '\0';
  }
  return ap;
}

bool getApMode()
{
  return ap_mode;
}

void factoryReset()
{
  Serial.println("Performing FACTORY RESET");
  digitalWrite(led_pin, HIGH);

  if (SPIFFS.exists("/ap.conf"))
  {
    SPIFFS.remove("/ap.conf");
  }
  if (SPIFFS.exists("/dev_name.conf"))
  {
    SPIFFS.remove("/dev_name.conf");
  }
  ESP.restart();
}
