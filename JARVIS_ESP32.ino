#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WebServerClass = ESP8266WebServer;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <SPIFFS.h>
using WebServerClass = WebServer;
#endif
#include <FS.h>
#include <AutoConnect.h>
#define TITLE       "J.A.R.V.I.S."
#define HOST_NAME       "jarvis"
#define HOME_URI        "/"
#define PARAM_FILE      "/settings.json" //not adding in data folder because everytime you upload data it wil reset you configured values
#define PAGE_SAVE      "/PAGE_SAVE.json"
#define PAGE_SETTINGS   "/PAGE_SETTINGS.json"
#define AUTOCONNECT_MENULABEL_CONFIGNEW   "Setup WIFI"
           
WebServerClass  server;
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux  settingsAux;
AutoConnectAux  saveAux;
#include "MusicLED.h"     
void handleRoot() {
  String page = PSTR(
"<html>"
"</head>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<style type=\"text/css\">"
    "body {"
    "-webkit-appearance:none;"
    "-moz-appearance:none;"
    "font-family:'Arial',sans-serif;"
    "text-align:center;"
    "}"
    ".menu > a:link {"
    "position: absolute;"
    "display: inline-block;"
    "right: 12px;"
    "padding: 0 6px;"
    "text-decoration: none;"
    "}"
    ".button {"
    "display:inline-block;"
    "border-radius:7px;"
    "background:#73ad21;"
    "margin:0 10px 0 10px;"
    "padding:10px 20px 10px 20px;"
    "text-decoration:none;"
    "color:#000000;"
    "}"
  "</style>"
"</head>"
"<body>"
  "<div class=\"menu\">" AUTOCONNECT_LINK(BAR_24) "</div>"
  "BUILT-IN LED<br>"
  "GPIO(");
  page += String(BUILTIN_LED);
  page += String(F(") : <span style=\"font-weight:bold;color:"));
  page += digitalRead(BUILTIN_LED) ? String("Tomato\">HIGH") : String("SlateBlue\">LOW");
  page += String(F("</span>"));
  page += String(F("<p><a class=\"button\" href=\"/io?v=low\">low</a><a class=\"button\" href=\"/io?v=high\">high</a></p>"));
  page += String(F("</body></html>"));
  server.send(200, "text/html", page);
}

void handleGPIO() {
  if (server.arg("v") == "low")
    digitalWrite(BUILTIN_LED, LOW);
  else if (server.arg("v") == "high")
    digitalWrite(BUILTIN_LED, HIGH);
  sendRedirect("/");
}

void sendRedirect(String uri) {
  server.sendHeader("Location", uri, true);
  server.send(302, "text/plain", "");
  server.client().stop();
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("JARVIS running on core ");
  Serial.println(xPortGetCoreID());
  // Responder of root page handled directly from WebServer class.
  server.on("/", handleRoot);
  server.on("/io", handleGPIO);
  // Load a custom web page described in JSON as PAGE_SETTINGS and
  // register a handler. This handler will be invoked from
  // AutoConnectSubmit named the Load defined on the same page.
  SPIFFS.begin();
   File page_settings = SPIFFS.open(PAGE_SETTINGS, "r");
      if (page_settings) {
        settingsAux.load(page_settings);
        page_settings.close();
      }
   File param = SPIFFS.open(PARAM_FILE, "r");
      if (param) {
        settingsAux.loadElement(param, { "sub_heading", "no_of_leds", "relay_config", "device1", "device2", "device3", "device4", "device5", "device6", "device7", "device8" } );
        param.close();
      }
  SPIFFS.end();
  settingsAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    if (portal.where() == "/settings") {
      // Use the AutoConnect::where function to identify the referer.
      // Since this handler only supports AutoConnectSubmit called the
      // Load, it uses the uri of the custom web page placed to
      // determine whether the Load was called me or not.
      SPIFFS.begin();
      File param = SPIFFS.open(PARAM_FILE, "r");
      if (param) {
        aux.loadElement(param, { "sub_heading", "no_of_leds", "relay_config", "device1", "device2", "device3", "device4", "device5", "device6", "device7", "device8" } );
        param.close();
      }
      SPIFFS.end();
    }
    return String();
  });
  SPIFFS.begin();
      File page_save = SPIFFS.open(PAGE_SAVE, "r");
      if (page_save) {
        saveAux.load(page_save);
        page_save.close();
      }
  SPIFFS.end();
  saveAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    // You can validate input values ​​before saving with
    // AutoConnectInput::isValid function.
    // Verification is using performed regular expression set in the
    // pattern attribute in advance.
   // AutoConnectInput& no_of_leds = settingsAux["no_of_leds"].as<AutoConnectInput>();
    //aux["validated"].value = no_of_leds.isValid() ? String() : String("Number of LEDs can be number only.");

    // The following line sets only the value, but it is HTMLified as
    // formatted text using the format attribute.
    aux["caption"].value = PARAM_FILE;

    SPIFFS.begin();
    File param = SPIFFS.open(PARAM_FILE, "w");
    if (param) {
      // Save as a loadable set for parameters.
       Serial.println(param);

      settingsAux.saveElement(param, { "sub_heading", "no_of_leds", "relay_config", "device1", "device2", "device3", "device4", "device5", "device6", "device7", "device8" });
      param.close();
      // Read the saved elements again to display.
      param = SPIFFS.open(PARAM_FILE, "r");
      aux["echo"].value = "Setting Saved"; //param.readString();
      param.close();
    }
    else {
      aux["echo"].value = "Failed to open.";
    }
    SPIFFS.end();
    return String();
  });

  portal.join({ settingsAux, saveAux });
  config.ticker = true;
  config.title = TITLE;
  config.apid = TITLE;
  config.homeUri = HOME_URI;
  config.hostName = HOST_NAME;
  config.portalTimeout = 60000;                 // Sets timeout value for the captive portal
  config.retainPortal = true;                   // Retains the portal function after timed-out
  portal.config(config);
   if (portal.begin()) {
    if (MDNS.begin(HOST_NAME)) {
      MDNS.addService("http", "tcp", 80);
    }
  }


//// MUSIC LED
//  AutoConnectInput& noOfLEDs = settingsAux.getElement<AutoConnectInput>("no_of_leds");
//  pixelCount = noOfLEDs.value.toInt();
//  strip = Adafruit_NeoPixel(pixelCount, pixelPin, NEO_GRB + NEO_KHZ800);

  LoadMusicLED();
  // MUSIC LED END
}

void loop() {
  portal.handleClient();
}
