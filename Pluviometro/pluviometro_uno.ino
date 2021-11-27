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
    Serial.print("[NTP] ");
    Serial.println(formattedDate);
    
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


int lluviaTotal = 0;

//------------------------------Google Drive------------------------------
//-------------------------------------------------------------------

#include <HTTPClient.h>
const char* resource = "/trigger/esp32_pluv/with/key/bBJ5MxH_rnj9dkPnqr9LEm";
const char* server = "maker.ifttt.com";

void IFTTT_Data(){ 

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


  
  String jsonObject = String("{\"value1\":\"") + dateTime + "\"}" + String("{\"value2\":\"") + lluviaTotal + "\"}";

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





#define RAIN_PIN 2 //Pin que recibe la interrupcion
#define CALC_INTERVALO 1000 // Incrementos por medicion
#define DEBOUNCE_TIME 15 // time * 1000 en microsegundos requerido para el ruido

// Resolution .2mm
// Accuracy 500 mm per hour +/- 1%
// Range 700mm per hour
// Average Switch closure Time 135 ms
// Temperatura de operacion 0° a 70° C
// Bounce Settling Time 0.75 ms

unsigned long nextCalc;
unsigned long timer;

/*
Typically global variables are used to pass data between an ISR and the main program. 
To make sure variables shared between an ISR and the main program are updated correctly, 
declare them as volatile.
*/

volatile unsigned int triggerSwitch = 0;
volatile unsigned long last_micros_rg;

void countingRain(){
  /*
    Comprueba si el tiempo desde la última llamada de interrupción es mayor que 
    el tiempo de rebote. Si es así, entonces la última llamada de interrupción es 
    a través del período ruidoso del rebote del interruptor de láminas, por lo que 
    podemos incrementar en uno.   

    El tiempo que ha transcurrido - el tiempo que ha transcurrido desde el ultimo valor
  */
  if ((long)(micros() - last_micros_rg)>= DEBOUNCE_TIME*1000){
    triggerSwitch += 1;
    last_micros_rg = micros(); 
  }
}



void setup() {
  Serial.begin(115200);
  initWiFi();
  delay(200);
  //attachInterrupt(digitalPinToInterrupt(pin), ISR, mode);
  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), countingRain, RISING);
  pinMode(RAIN_PIN, INPUT);
  nextCalc = millis() + CALC_INTERVALO;
  // Pin que recibe la interrupcion -> digitalPinToInterrupt(), 
  
  //Pines digitales para Arduino Uno 2 y 3
  /*
  sólo uno puede ejecutarse a la vez, otras interrupciones se ejecutarán después 
  de que la actual termine en un orden que depende de la prioridad que tienen. 
  
  millis() se basa en las interrupciones para contar, por lo que nunca se incrementará 
  dentro de un ISR. 
  
  Dado que 
    delay() requiere interrupciones para funcionar, no funcionará si se llama dentro de un ISR. 
  
  micros() funciona inicialmente pero empezará a comportarse erráticamente después de 1-2 ms. 
  
  delayMicroseconds() no utiliza ningún contador, por lo que funcionará normalmente.
  */

  /*
  *About Interrupt Service Routines

  ISRs are special kinds of functions that have some unique limitations most other functions 
  do not have. 
  
  An ISR cannot have any parameters, and they shouldn’t return anything.
  */

  /*
  mode

    LOW to trigger the interrupt whenever the pin is low,

    CHANGE to trigger the interrupt whenever the pin changes value

    RISING to trigger when the pin goes from low to high,

    FALLING for when the pin goes from high to low.

The Due, Zero and MKR1000 boards allow also:

    HIGH to trigger the interrupt whenever the pin is high.


  */
  if((long)(micros() - last_micros_rg) >= DEBOUNCE_TIME * 1000) { 
    triggerSwitch += 1;
    last_micros_rg = micros();
  }  
}

void loop() {
 
  timer = millis();
  if(timer > nextCalc){
    nextCalc = timer + CALC_INTERVALO;
    Serial.print("Total Tips: ");
    Serial.println((float) triggerSwitch);
  }
  getTimeNTP();
  IFTTT_Data();
  deepSleepMode(10);
  /*
   * Seria necesario ponerle interrupciones para despertar al micro
      cuando reciba un pulso y determinar cuanto tiempo maximo
      debe esperar al siguiente pulso para volver a dormir y antes 
      de dormir, reportar lo acumulado 
   */
  
}
