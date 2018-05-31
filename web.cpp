#include "web.h"
#include "matrix7219.h"

extern web              server;
extern ledDisplay       ld;

void handleDisplay(void);
void handleNotFound(void);

// web page styles
const char style[] PROGMEM = R"=====(
<style>
body  {background-color:powderblue;}
input {background-color:powderblue;}
select {background-color:powderblue;}
t1    {color: black; align-text: center; font-size: 200%;}
h1    {color: blue;  padding: 5px 0; align-text: center; font-size: 150%; font-weight: bold;}
h2    {color: black; padding: 0 0; align-text: center; font-size: 120%; font-weight: bold;}
th    {vertical-align: middle; text-align: center; font-weight: normal;}
td    {vertical-align: middle; text-align: center;}
ul    {list-style-type: none; margin: 0; padding: 0; overflow: hidden; background-color: #333;}
li    {float: left; border-right:1px solid #bbb;}
li:last-child {border-right: none;}
li a  {display: block; color: white; text-align: center; padding: 14px 16px; text-decoration: none;}
li a:hover:not(.active) {background-color: #111;}
.active {background-color: #6aa7e3;}
form {padding: 20px;}
form .field {padding: 4px; margin: 1px;}
form .field label {display: inline-block; width:240px; margin-left:5px; text-align: left;}
form .field input {display: inline-block; size=20;}
form .field select {display: inline-block; size=20;}
.myframe {width:700px; -moz-border-radius:5px; border-radius:5px; -webkit-border-radius:5px;}
.myheader {margin-left:10px; margin-top:10px;}
th,td {padding:10px;}
</style>
</head>
)=====";

FontItem font_list[] ={ {.name = "Cronyx-Times-Medium-R-Normal--10-100",    .id = u8g_font_cronyx2t},
                        {.name = "Cronyx-Courier-Medium-R-Normal--12-120",  .id = u8g_font_cronyx3c},
                        {.name = "Cronyx-Helvetica-Medium-R-Normal--12-120",.id = u8g_font_cronyx3h},
                        {.name = "Cronyx-Helvetica-Bold-R-Normal--12-120",  .id = u8g_font_cronyx3hb},
                        {.name = "Cronyx-Times-Medium-R-Normal--12-120",    .id = u8g_font_cronyx3t},
                        {.name = "Cronyx-Times-Bold-R-Normal--12-120",      .id = u8g_font_cronyx3tb},
                        {.name = "Cronyx-Fixed-Medium-R-Normal--13-120",    .id = u8g_font_koi8x13},
                        {.name = "Cronyx-Fixed-Medium-R-Normal--15-140",    .id = u8g_font_koi9x15}
};
const uint8_t font_list_size = sizeof(font_list)/sizeof(FontItem);

bool web::setupAP(void) {
  const char *ssid = "creeping-line";
  bool stat = WiFi.softAP(ssid);                // use default IP address, 192.168.4.1
  if (stat) {
    ESP8266WebServer::on("/", handleDisplay);
    ESP8266WebServer::on("/message", handleDisplay);
    ESP8266WebServer::onNotFound(handleNotFound);
    ESP8266WebServer::begin();
  }
  return stat;
}

void header(const String title, bool refresh = false) {
    String hdr = "<!DOCTYPE HTML>\n<html><head>\n";
    if (refresh) hdr += "<meta http-equiv='refresh' content='40'/>";
    hdr += "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>";
    hdr += title;
    hdr += "</title>\n";
    server.sendContent(hdr);
    server.sendContent_P(style);
}

void handleDisplay(void) {
    uint8_t  scr_bright = ld.intencity(16);
    uint16_t scr_speed  = 510 - ld.delayMS(0);
    uint8_t  f_index    = 0;
    String   new_msg = "";
    if (server.args() > 0) {                    // Setup new message to be displayed
        String fn   = server.arg("font");
        f_index     = fn.toInt();
        if (f_index < font_list_size) {
            ld.setFont(font_list[f_index].id);
        }
        new_msg = server.arg("msg");
        ld.utf8text(new_msg);
        String br  = server.arg("bright");
        scr_bright = constrain(br.toInt(), 0, 15);
        scr_bright = ld.intencity(scr_bright);
        String sp  = server.arg("speed");
        scr_speed  = constrain(sp.toInt(), 0, 490);
        ld.delayMS(510-scr_speed);
    }
    header("set message");
    String body = "<body>";
    body += "<br>\n<form action='/message'>\n";
    body += "<div align=\"center\"><p>Шрифт: <select name='font'>\n";
    for (uint8_t i = 0; i < font_list_size; ++i) {
        body += "<option value='";
        body += i;
        if (i == f_index) {
            body += "' selected>";
        } else {
            body += "'>";
        }
        body += font_list[i].name;
        body += "</option>\n";
    }
    body += "</select>\n</p></div>";
    body += "<div align=\"center\">Введите сообщение</div><br>";
    body += "<div align=\"center\"><input type='text' name='msg' maxlength='128' size='64' value='";
    body += new_msg;
    body += "'></div>";
    body += "<div align='center'>Яркость<input name='bright' type='range' min='0' max='15' step='1' value='";
    body += scr_bright;
    body += "'></div><br>";
    body += "<div align='center'>Скорость<input name='speed' type='range' min='0' max='490' step='10' value='";
    body += scr_speed;
    body += "'></div><br>";
    body += "<div align=\"center\"><input type=\"submit\" value=\"сохранить\"></div>\n</form>\n</body>\n</html>";
    server.sendContent(body);
}

void handleNotFound(void) {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (byte i = 0; i < server.args(); ++i) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    server.send(404, "text/plain", message);
}
