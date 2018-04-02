#include <FS.h>                   //this needs to be first, or it all crashes and burns...

// --- WIFI
#include <ESP8266WiFi.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

WiFiManager wifiManager;
WiFiClient nClient;

/// --- MQTT
#include <PubSubClient.h>
const char* mqttBroker = "io.adafruit.com";
const char* mqttUser = "ayrtonkrizan";
const char* mqttKey = "666972716b8047a694671a70fddca3a9";
const char* mqttTopic = "ayrtonkrizan/feeds/mobcontrol";
PubSubClient client(nClient);


/// --- IRRemote
#include <IRremoteESP8266.h> //INCLUSÃO DE BIBLIOTECA 
IRsend irsend(14); //FUNÇÃO RESPONSÁVEL PELO MÉTODO DE ENVIO DO SINAL IR / UTILIZA O GPIO14(D5)

/// --- IR Remte Functions
void sendHexa(String hexa, int type, int freq = 32){
  switch(type){
    case 1:
      irsend.sendRC5(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 2:
      irsend.sendRC6(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 3:
      irsend.sendNEC(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 4:
      irsend.sendSony(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 5:
      irsend.sendPanasonic(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 6:
      irsend.sendJVC(strtoul(hexa.c_str(), 0, 16), freq, true);
      break;
    case 7:
      irsend.sendSAMSUNG(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 8:
      irsend.sendWhynter(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 9: //AYWA
      irsend.sendNEC(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 10:
      irsend.sendLG(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 11: //SANYO
      irsend.sendNEC(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 12: //Mitsubishi
      irsend.sendNEC(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 13:
      irsend.sendDISH(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 14:
      irsend.sendSharpRaw(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 15://Denon
      irsend.sendNEC(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 16://PRONTO
      irsend.sendNEC(strtoul(hexa.c_str(), 0, 16), freq);
      break;
    case 17://LegoPowerFunctions
      irsend.sendNEC(strtoul(hexa.c_str(), 0, 16), freq);
      break;
  }
    delay(40);
}


/// --- MQQT Functions
void mqttReconnect() {
  while (!client.connected()) {
    Serial.println("Tentando conexão no servidor MQQT...");
    if(client.connect("teste", mqttUser, mqttKey))
    {
      Serial.println("Conectado com sucesso.");
      client.subscribe(mqttTopic);
    }
    else
    {
      Serial.print("Failed ...");
      Serial.println(client.state());
      delay(3000);
    }
    
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String hexa = "";
  String type = "";
  String freq = "";
  for (int i = 0; i < length; i++) {
    if( i <2)
      type += (char)payload[i];
    else if (i < 4)
      freq += (char)payload[i];
    else
      hexa += (char)payload[i];
  }
  Serial.println("--------------");
  Serial.print("Hexa");
  Serial.println(hexa);
  Serial.print("Type");
  Serial.println(type);
  Serial.print("Frequency");
  Serial.println(freq);
  

  // Switch on the LED if an 1 was received as first character
  if (length>0 && freq.length() > 0) {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    
    int cmdLen = freq.toInt()/4;
    int qtCmd = hexa.length()/cmdLen;
    for(int i = 0; i<qtCmd; i++)
    {
      String cmd = hexa.substring(i*cmdLen, (i+1)*cmdLen);
      sendHexa(cmd, type.toInt(), freq.toInt());
      Serial.println("--------------");
      Serial.println(cmd);
    }
  }
  digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
}


/// --- wifi Functions
void wifiConnect()
{
  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Falha ao conectar nas configuracoes salvas... resetando");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Conectado a WiFi :)");


  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}


/// --- Default Functions
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  irsend.begin(); //INICIALIZA A FUNÇÃO
  Serial.begin(115200);
  wifiConnect();
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  mqttReconnect();
  client.loop();
  delay(500); //Delay of 5 seconds
}
