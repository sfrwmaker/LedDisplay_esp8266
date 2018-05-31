#include "web.h"
#include "matrix7219.h"
#include "fonts.h"

const uint16_t  web_port    = 80;
const uint8_t   cs_pin      = 15;
const uint8_t   width       = 64;
const uint8_t   height      = 16;

ledDisplay      ld(cs_pin, width, height);
web             server(web_port);
  
void setup() {
    delay(1000);
    Serial.begin(115200);
    Serial.println("\n\n\n");

    server.setupAP();
    ld.init();
    ld.intencity(1);
    ld.setFont(u8g_font_cronyx3t);
    ld.eolSize(56);
    ld.utf8text("Please connect. WiFi SSID: creeping-line; http://192.168.4.1");
}

void loop() {
    yield();
    ld.scrollLeft();
    ld.show();
    server.handleClient();
}
