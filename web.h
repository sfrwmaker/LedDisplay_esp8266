#ifndef WEB_H
#define WEB_H
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>

class web : public ESP8266WebServer {
  public:
    web(uint16_t port) : ESP8266WebServer(port) { }
    bool setupAP(void);
};

#endif
