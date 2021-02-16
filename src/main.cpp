#include <Arduino.h>


#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>


void handleNotFound();
bool loadFromSdCard(String path);
void printDirectory(File dir, int numTabs);


bool hasSD = false;
const int CS_SDcard = 5;

const byte DNS_PORT = 53;
IPAddress apIP(172, 217, 28, 1);
DNSServer dnsServer;
WebServer webServer(80);


void setup() {

  Serial.begin(9600);
  Serial.println("");

  Serial.print("Waiting for SD card to initialise...");
    if (!SD.begin(CS_SDcard)){hasSD = false; Serial.println("Initialising failed!");}
    else {hasSD = true; Serial.println("->OK");}


/*
    File root = SD.open("/");
    root.rewindDirectory();
    printDirectory(root, 0); //Display the card contents
    root.close();
*/

  Serial.println("-------------------------------------------");
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("DNSServer CaptivePortal example");
  delay(2000); // VERY IMPORTANT
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("Server Started");;
}




void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}



void handleNotFound(){
  if(hasSD && loadFromSdCard(webServer.uri())) return;
  Serial.print("handleNotFound: uri");
  Serial.print(webServer.uri());

  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i=0; i<webServer.args(); i++){
    message += " NAME:"+webServer.argName(i) + "\n VALUE:" + webServer.arg(i) + "\n";
  }
  webServer.send(404, "text/plain", message);
  Serial.print(message);
}



bool loadFromSdCard(String path){

  Serial.println("-------------------------------------------");
  Serial.print("Requested File:");
  Serial.println(path);

  String dataType = "text/plain";
  if(path.endsWith("/generate_204")) path = "/index.html"; 
  if(path.endsWith("/ncsi.txt")) path = "/index.html"; 
  if(path.endsWith("/connecttest.txt")) path = "/index.html";
  if(path.endsWith("/redirect")) path = "/index.html";
  if(path.endsWith("/")) path += "index.html";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";

  File dataFile = SD.open(path.c_str());
  if(dataFile.isDirectory()){
    path += "/index.html";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
  }

  if (!dataFile)
    return false;

  if (webServer.hasArg("download")) dataType = "application/octet-stream";

  if (webServer.streamFile(dataFile, dataType) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
  Serial.print("repply: ");
  Serial.println(path);
  Serial.println("-------------------------------------------");
  return true;
}


void printDirectory(File dir, int numTabs)
{

    while (true)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            // no more files
            break;
        }
        if (numTabs > 0)
        {
            for (uint8_t i = 0; i <= numTabs; i++)
            {
                Serial.print('\t');
            }
        }
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}
