#include "EmonLib.h"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

EnergyMonitor emon1;;

uint32_t durasi;
long waktu_start = 0;
long periode = 0;

const char* ssid = "waifu";
const char* password = "sandi";

const char* host = "host.firebaseapp.com";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "E5 4E 5F CA 79 3E 5C C8 51 D0 F7 EC FC CA 28 E8 77 C1 04 C1";

void setup() {

  Serial.begin(115200);

  emon1.current(0, 111.1);             // Current: input pin, calibration.
  
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}


void loop() {

  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  float U = 220;
  float cospi = 0.85;
  

  // To calculate the power we need voltage multiplied by current
  float P = U * Irms * cospi; 

  durasi = millis();
  periode = durasi - waktu_start;

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }
    //url syntax : /update/watt/ampere/delay
  Serial.print("ARUS : " + String(Irms));
  String url = "/update/"+String(P) + "/"+String(Irms) + "/"+String(periode);
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  delay(2000);
   waktu_start = durasi;
}
