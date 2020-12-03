 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

#define EEPROM_LENGTH 1024

char eRead[30];
byte len;

char ssid[30] = "732-2.4G";
char password[30] = "kpu123456!";

bool captive = true;
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

ICACHE_RAM_ATTR void GPIO0(){
  SaveString(0, "");
  ESP.restart();
}

String responseHTML = ""
 "<!DOCTYPE html><html><head><title>2020_CP_COP</title></head><body><center>"
 "<p>CaptivePotal & Relay</p>"
 "<form action='/button'>"
 "<p><input type='text' name='ssid' placeholder='SSID' onblur='this.value=removeSpaces(this.value);'></p>"
 "<p><input type='text' name='password' placeholder='WLAN Password'></p>"
 "<p><input type='submit' value='Submit'></p></form>"
 "<p>You can connect to relay server.</p></center></body>"
 "<script>function removeSpaces(string) {"
 " return string.split(' ').join('');"
 "}</script></html>";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  EEPROM.begin(EEPROM_LENGTH);

  ReadString(0, 30);
  if(!strcmp(eRead, "")){
    setup_captive();
  }else{
    captive = false;
    strcpy(ssid, eRead);
    ReadString(30, 30);
    strcpy(password, eRead);
    setup_runtime();
  }
}
void setup_runtime(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  int i = 0;
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    if (i++ > 15){
      captive = true;
      setup_captive();
      return;
    }
  }

  Serial.println("");
  Serial.print("Connected to "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  if (MDNS.begin("kimjunsoo")){
    Serial.println("MDNS responder started");
  }

  webServer.on("/", [](){
    webServer.send(200, "text/plain", "jisung_barlra\n");
  });
  webServer.on("/control", control);
  webServer.on("/off", OFF);

  webServer.onNotFound(handleNotFound);

  webServer.begin();
  attachInterrupt(0, GPIO0, FALLING);
  pinMode(15, OUTPUT);
  Serial.println("HTTP server started");
}

void setup_captive(){
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("kimjunsoo");

  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/button", button);

  webServer.onNotFound([](){
    webServer.send(200, "text/html", responseHTML);
  });
  webServer.begin();
  Serial.println("Captive Portal Started");
}

void button(){
 Serial.println("button pressed");
 Serial.println(webServer.arg("ssid"));
 SaveString( 0, (webServer.arg("ssid")).c_str());
 SaveString(30, (webServer.arg("password")).c_str());
 webServer.send(200, "text/plain", "OK");
 ESP.restart();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (captive){
    dnsServer.processNextRequest();
  }
  webServer.handleClient();
}

void SaveString(int startAt, const char* id) {
 for (byte i = 0; i <= strlen(id); i++) {
 EEPROM.write(i + startAt, (uint8_t) id[i]);
 }
 EEPROM.commit();
}

void ReadString(byte startAt, byte bufor) {
 for (byte i = 0; i <= bufor; i++) {
  eRead[i] = (char)EEPROM.read(i + startAt);
 }
 len = bufor;
}
void control(){
  char survoselect[100]="123";
  char survoangle[100]="123";

  String controlHTML = ""
  "<!DOCTYPE html><html><head><title>start</title></head><body><center>"
  "<p>survocontrol</p>"
  "<form action='/jisung'>"
  "<p><input type='text' name='ssid' placeholder='servorselect' onblur='this.value=removeSpaces(this.value);'></p>"
  "<p><input type='text' name='password' placeholder='delaytime'></p>"
  "<p><input type='submit' value='Submit'></p></form>"
  "<p>You can control each finger .</p></center></body>"
  "<script>function removeSpaces(string) {"
  " return string.split(' ').join('');"
   "}</script></html>";
  
  webServer.send(200, "text/html",controlHTML );
  while(!strcmp(eRead, ""));
  ReadString(0, 30);
  strcpy(survoselect, eRead);
  ReadString(30, 30);
  strcpy(survoangle, eRead);
  Serial.println("success");
  
  
}

void OFF(){
  char mBuf[500];
  char tmplt[] = "<html><head><meta charset=\"utf-8\">"
                 "<title>Relay OFF</title></head>"
                 "<body>"
                 "<script></script>"
                 "<center><p>"
                 "<head>Relay OFF"
                 "</center>"
                 "</body></html>";
  sprintf(mBuf, tmplt);
  digitalWrite(15, LOW); 
  Serial.println("OFF");
  webServer.send(200, "text/html", mBuf);  
}

void handleNotFound(){
 String message = "File Not Found\n\n";
 webServer.send(404, "text/plain", message);
}
