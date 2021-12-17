/*
 * Fecha-Hora
 * **Wi-Fi
 * **Consulta de fecha y hora 
 * 
 * Hum_Amb/Temp_Amb - Sensor AM2301, encapsulado diferente para el DHT21
 * Temp_Suelo 1 y 2  - 
 * Hum Suelo 1 y 2 - Sensor resistivo, Fabricante: Irrometer, modelo: Watermark 200ss
 * 
 * 
 * Fecha-Hora, Hum_Amb, Temp_Amb, Hum_Suelo_1, Temp_Suelo_1, Hum_Suelo_2, Temp_Suelo_2  
 * Guardar en SD 
 * Subir a Google Sheet
 */

/*------------------------------Libreria SD------------------------------
 * ************Error al colocar las librerias junto a su modulo (pedazo de codigo)
 */
#include "FS.h"
#include "SD.h"
#include "SPI.h"
 
//------------------------------WiFi------------------------------
//----------------------------------------------------------------
#include <WiFi.h>
const char* ssid = "Tweet";
const char* password = "Hometweet1111";

int conTime = 0;

void initWiFi(){

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("[WiFi] Conectando a red WiFi");
  
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
    conTime++;
    if(conTime==5){
      Serial.println(".");
      Serial.println("[WiFi] Reconectando a red... 5");
      reconnectWiFi();
    }
    if(conTime==15){
      Serial.println(".");
      Serial.println("[WiFi] Reconectando a red... 15");
      reconnectWiFi();
    }
    if(conTime==30){
      Serial.println(".");
      Serial.println("[WiFi] Reconectando a red... 30");
      reconnectWiFi();
    }
    if(conTime==45){
      Serial.println(".");
      Serial.println("[WiFi] Reconectando a red... 45");
      reconnectWiFi();
    }
    if (conTime == 60){
      
      Serial.print(".");
      Serial.println("[WiFi] Reiniciando... 60");
      conTime = 0;
      ESP.restart();
    }
  }
  Serial.println(".");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectWiFi(){
  Serial.println("[WiFi_Reconnect] Reconectando... ");
  WiFi.disconnect();
  WiFi.reconnect();
}

//------------------------------Deep Sleep------------------------------
//----------------------------------------------------------------------
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

void deepSleepMode(int TIME_TO_SLEEP){
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("[DeepSleep] ESP32 Dormira durante " + String(TIME_TO_SLEEP) +
  " Segundos");
  Serial.println("[DeepSleep] A dormir");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
}

//------------------------------Fecha y Hora------------------------------
//------------------------------------------------------------------------
#include <NTPClient.h>
#include <WiFiUdp.h>
//Cliente NTP para hacer request de fecha y tiempo al servidor de NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;
String dateTime;

void getTimeNTP(){
  timeClient.begin();
  timeClient.setTimeOffset(-21600);
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  //Serial.print("[NTP] ");
  //Serial.println(formattedDate);
  
  // Extract date
  
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);

  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

  Serial.print("[NTP] Fecha y hora: ");
  dateTime = dayStamp + " " + timeStamp;
  Serial.println(dateTime);
  delay(1000);
}
//------------------------------Hum/Temp Ambiente------------------------------
//-----------------------------------------------------------------------------
#include "DHT.h"
#define DHTPIN 13    // modify to the pin we connected
#define DHTTYPE DHT21   // AM2301 
DHT dht(DHTPIN, DHTTYPE);

float humAmb = 0;
float tempAmb = 0;

void dhtSensor(){
 
  delay(200);
  
  humAmb = dht.readHumidity();
  tempAmb = dht.readTemperature();
  Serial.print("[DHT] Humedad Ambiente = ");
  Serial.println(humAmb);
  Serial.print("[DHT] Temperatura Ambiente = ");
  Serial.println(tempAmb);
}
//------------------------------Google Drive------------------------------
//-------------------------------------------------------------------
/*
 * Los applets unen servicios, aplicaciones y dispositivos 
 */
#include <HTTPClient.h>
const char* resource = "/trigger/esp32_pluv/with/key/bBJ5MxH_rnj9dkPnqr9LEm";
const char* server = "maker.ifttt.com";

void IFTTT_DateTime(){ 

  Serial.println("[IFTTT]------Archivo para Fecha y Hora------");
  Serial.print("[IFTTT] Conectando al servidor: "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("[IFTTT] Conexion fallida...");
  }
  
  Serial.print("[IFTTT] Solicitando: "); 
  Serial.println(resource);


  
  String jsonObject = String("{\"value1\":\"") + dateTime + "\",\"value2\":\"" + humAmb
                      + "\",\"value3\":\"" + tempAmb + "\"}";
  
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);

  
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("[IFTTT] No responde...");
  }
  while(client.available()){
    Serial.write(client.read());
  }

  Serial.print("[IFTTT] Fecha y hora: ");
  Serial.println(dateTime);

  
  Serial.println("\n[IFTTT] Cerrando conexion");
  client.stop();            
}


void setup() {
  Serial.begin(115200);
  initWiFi();
  delay(200);
  dht.begin();

}



void loop() {

  delay(200);
  Serial.println("------------------Fecha y Hora------------------");
  getTimeNTP();
  Serial.println("------------------Valores del ambiente------------------");
  dhtSensor();
  delay(100);
  Serial.println("------------------Google Drive------------------");
  IFTTT_DateTime();
  delay(200);


}
