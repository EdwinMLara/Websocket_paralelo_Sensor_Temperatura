#include <WiFi.h>
#include <Wire.h>
#include <WebSocketsClient.h>
#include <stdlib.h>
#include <string.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_PCF8574.h>



//const char* ssid = "INFINITUM3B3D";
//const char* password = "Valiant.Michel35";

//const char* ssid = "INFINITUM90F7_2.4";
//const char* password = "UEfHk82dGw";

const char* ssid = "INSOELIoT";
const char* password = "d3bd47b3ad";

//const char* ssid = "Casa-Home BT";
//const char* password = "elRK16_ma8L";


unsigned long startmillis, Tsampl, k;
unsigned long currentmillis;
unsigned long tiempo = 1000; 

WebSocketsClient  webSocket;
LiquidCrystal_PCF8574 lcd(0x27);

int count=0;

IPAddress local_IP(192, 168, 4, 9);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

bool isConnected = false;
bool isarray = false;
bool datos = false;
bool Humedad = false;

void config_lcd(){
  int error;

  while (! Serial);

  Serial.println("Dose: check for LCD");
  
  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  Serial.print("Error: ");
  Serial.print(error);

  if (error == 0) {
    Serial.println(": LCD found.");

  } else {
    Serial.println(": LCD not found.");
  } 

  lcd.begin(16, 4);
  lcd.setBacklight(100);
  lcd.clear();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) { 
  switch (type) {
    case WStype_DISCONNECTED:             
      Serial.printf("Desconectado!\n");
      lcd.setCursor(0, 0);
      lcd.print("Desconectado");
      isConnected = false;
      break;
    case WStype_CONNECTED:         
        Serial.printf("Conectado a servidor\n");
        lcd.setCursor(0, 0);
        lcd.print("Conectado a Servidor");
        isConnected = true;               
        break;
    case WStype_TEXT:
        byte n = 0;                     
        const char s[2] = ":";
        char *token;
        char *aux;
        char *msj = (char *)payload;

        Serial.println(msj);
        
        if ((strchr(msj, ':'))){
          token = strtok(msj,s);
          aux = token;
          
          while (token != NULL){
            token = strtok(NULL,s);
            switch (n){
              case 0:
                tiempo = atoi(token)*1000;
                n += 1;
                break;
              default:
                break;  
            }
            lcd.clear();
            lcd.setCursor(0, 3);
            lcd.print("Enviando");
          }  
          
          if(strcmp(aux,"Iniciar") == 0){
            datos = true;
            startmillis = millis();
            Tsampl=millis();
            k=0;
            lcd.setCursor(0, 1);
            lcd.print("Inicia Servidor");
          }
        }else{
          if(strcmp(msj,"Humedad") == 0){
            Humedad = true;
          }
        }
        
        break;
  }
}

void connectar_to_ssid(){

  config_lcd();
  
  Serial.print("Conectado a : ");
  Serial.println(ssid);

  Serial.println(WiFi.begin(ssid,password) ? "Iniciando la conextión" : "Fallo el inicio");
  Serial.println("Empezando a conectar al WiFi");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  pinMode(2,OUTPUT);
  connectar_to_ssid();

  webSocket.begin("192.168.1.64", 8080,"/socket_paralelo/WebsocketTemperatura");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  
}

void led_protocol_envio(byte num_pul){
  for(int r=1;r<num_pul;r++){
          digitalWrite(2,LOW);
          delay(10);
          digitalWrite(2,HIGH);
          delay(10);
        }
}

char aux[12];

void loop() {
  int Celda[10];
  for(byte i=0;i<10;i++){
    Celda[i]=random(1,100);
  }
  
  if(isConnected){
    digitalWrite(2,HIGH);
  }

 
  if(datos == true ){
    StaticJsonDocument<800> doc;
    JsonArray array = doc.to<JsonArray>();
    if(abs(millis() - Tsampl) >= 100){
      Tsampl=millis();
      digitalWrite(2,1); 
      array.add(millis());
      array.add(k++);     
        for(byte j=0;j<10;j++){
          int c = Celda[j];        
          array.add(c);
          Serial.println(Celda[j]);
        }
      String aux_json;
      serializeJson(doc, aux_json);
      webSocket.sendTXT(aux_json);    
    }digitalWrite(2,0);
    
    currentmillis = millis();  
    if(currentmillis - startmillis >= tiempo){
      datos = false;
      webSocket.sendTXT("Fin");
      webSocket.disconnect();
    }


    
   }
   webSocket.loop();
 }
 
