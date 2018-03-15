#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PageBuilder.h>
#include "AutoConnect.h"

ESP8266WebServer server;
AutoConnect      portal(server);

static const char mold_page[] PROGMEM  = {
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
    ".menu {"
    "text-align:right;"
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
  "<p class=\"menu\">" AUTOCONNECT_LINK(BAR_32) "</p>"
  "BUILT-IN LED<br>"
  "GPIO({{LED}}) : <span style=\"font-weight:bold;color:{{COLOR}}\">{{GPIO}}</span>"
  "<p><a class=\"button\" href=\"/io?v=low\">low</a><a class=\"button\" href=\"/io?v=high\">high</a></p>"
  "</body>"
"</html>"
};

String getLEDPort(PageArgument& args) {
  return String(BUILTIN_LED);
}

String setColor(PageArgument& args) {
  return digitalRead(BUILTIN_LED) ? "Tomato" : "SlateBlue";
}

String readLEDPort(PageArgument& args) {
  return digitalRead(BUILTIN_LED) ? "HIGH" : "LOW";
}

PageElement elm_gpio(mold_page, {
  {"LED", getLEDPort},
  {"COLOR", setColor},
  {"GPIO", readLEDPort}
});
PageBuilder root("/", { elm_gpio });


String gpio(PageArgument& args) {
  if (args.arg("v") == "low")
    digitalWrite(BUILTIN_LED, LOW);
  else if (args.arg("v") == "high")
    digitalWrite(BUILTIN_LED, HIGH);
  sendRedirect("/");
  return "";
}

PageElement elm_io("{{IO}}", { {"IO", gpio} });
PageBuilder io("/io", { elm_io });

void sendRedirect(String uri) {
  server.sendHeader("Location", uri, true);
  server.send(302, "text/plain", "");
  server.client().stop();
  io.cancel();
}

bool atDetect(IPAddress softapIP) {
  Serial.println("Captive portal started, SoftAP IP:" + softapIP.toString());
  return true;
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  pinMode(BUILTIN_LED, OUTPUT);

  // Put the home location of the web site.
  // But in usually, setting the home uri is not needed cause default location is "/".
  //portal.home("/");   
  
  // Starts user web site included the AutoConnect portal.
  portal.onDetect(atDetect);
  if (portal.begin()) {
    // Register the page request handlers.
    root.insert(server);
    io.insert(server);
    Serial.println("Started, IP:" + WiFi.localIP().toString());
  }
  else {
    Serial.println("Connection failed.");
    while (true) { yield(); }
  }
}

void loop() {
  portal.handleClient();
  if (WiFi.status() == WL_IDLE_STATUS) {
    ESP.reset();
    delay(1000);
  }
}