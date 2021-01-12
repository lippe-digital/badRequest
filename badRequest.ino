#include <ESP8266WiFi.h>
#include <CertStoreBearSSL.h>
#include <time.h>
#include <FS.h>
#include <LittleFS.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PW"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

// A single, global CertStore which can be used by all
// connections.  Needs to stay live the entire time any of
// the WiFiClientBearSSLs are present.
BearSSL::CertStore certStore;

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

// Try and connect using a WiFiClientBearSSL to specified host:port and make a POST request
void post(BearSSL::WiFiClientSecure *client, const char *host, const uint16_t port, const char *path) {
  if (!path) {
    path = "/";
  }

  Serial.printf("Trying: %s...", host);
  client->connect(host, port);
  if (!client->connected()) {
    Serial.printf("*** Can't connect. ***\n-------\n");
    return;
  }
  Serial.printf("Connected!\n-------\n");
  client->write("POST ");
  client->write(path);
  client->write(" HTTP/1.0\r\nHost: ");
  client->write(host);
  client->write("\r\n");
  client->write("Content-Type: application/json\r\n");
  client->write("{\"message\": \"**Backup** was successfully finished.\",\"priority\": 5,\"title\": \"Backup\"}\r\n");
  client->write("\r\n");
  uint32_t to = millis() + 5000;
  if (client->connected()) {
    do {
      char tmp[32];
      memset(tmp, 0, 32);
      int rlen = client->read((uint8_t*)tmp, sizeof(tmp) - 1);
      yield();
      if (rlen < 0) {
        break;
      }
      Serial.print(tmp);
    } while (millis() < to);
  }
  client->stop();
  Serial.printf("\n-------\n");
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();

  LittleFS.begin();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  setClock(); // Required for X.509 validation

  int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.printf("Number of CA certs read: %d\n", numCerts);
  if (numCerts == 0) {
    Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
    return; // Can't connect to anything w/o certs!
  }
}

void loop() {
 
  BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
  // Integrate the cert store with this connection
  bear->setCertStore(&certStore);
  post(bear, site.c_str(), 443, "/gotify/message?token=TokenID");
  delete bear;
  delay(30000);
}
