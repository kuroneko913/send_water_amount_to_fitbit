#include<M5Stack.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>

// HX711 related pin Settings.  HX711 
#include "HX711.h"
#define LOADCELL_DOUT_PIN 36
#define LOADCELL_SCK_PIN 26

/* display and sprite */
static LGFX lcd;   
static LGFX_Sprite Frame(&lcd);
static LGFX_Sprite MessageBox(&lcd);
static LGFX_Sprite Communication(&lcd);

/* scale */
HX711 scale;
const long LOADCELL_OFFSET = 5018;
const long LOADCELL_DIVIDER = 5895;
const int MUST_COUNT = 5;
const int  MAX_COUNT = 10;
const int TUMBLER_WEIGHT = 184;

/* WiFi connection */
const char *ssid = "your-WiFi-SSID";
const char *password = "your-WiFi-Password";

/* MQTT Settings */
// MQTTの接続先のIP
const char *endpoint = "";
// MQTTのポート
const int port = 1883;
// デバイスID
char *deviceID = "M5Stack";  // デバイスIDは機器ごとにユニークにします
// メッセージを知らせるトピック
char *pubTopic = "/pub/M5Stack";

WiFiClient httpsClient;
PubSubClient mqttClient(httpsClient);

int weightValue[MAX_COUNT];
int weight=0;
int sendWeight = 0;
int count;

char pubMessage[128];

void setup() {
  M5.begin();
  M5.Power.begin();
  lcd.init();

  lcd.setTextSize(2);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
        delay(200);
        lcd.print('.');
  }
  delay(100);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.println("WiFi connect success!!");
  lcd.println(WiFi.localIP());
  delay(2500);

  mqttClient.setServer(endpoint, port);
  connectMQTT();
  
  lcd.clear();
  Frame.createSprite(320,30);
  MessageBox.createSprite(320,60);
  Communication.createSprite(320,40);
  
  Frame.fillScreen(TFT_BLACK);
  Frame.setTextSize(2);
  Frame.drawString("  send the amount of water  ",0,0);
  Frame.drawString("      ver 1.0.0     ",20,30);
  Frame.pushSprite(&lcd,0,0);

  MessageBox.setTextSize(2);
  MessageBox.drawString("  initializing... ", 0, 0);
  MessageBox.pushSprite(0,50);
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);  
  scale.set_scale(LOADCELL_DIVIDER); 
  scale.set_offset(LOADCELL_OFFSET);
  scale.set_scale(214.56f);
  scale.tare();
}

void loop() {
    // 常にチェックして切断されたら復帰できるように
    mqttLoop();
      
    char buffer[32];
    if (sendWeight > 0 && M5.BtnB.isPressed()) {
       Communication.clear();
       Communication.setTextSize(2);
       Communication.drawString("connecting ...", 0, 0);
       Communication.pushSprite(0,120);
       delay(2000);
       sprintf(pubMessage, "{\"water_weight\": %d}", sendWeight);
       mqttClient.publish(pubTopic, pubMessage);
       Communication.drawString("published!", 0, 0);
       Communication.pushSprite(0,120);
       delay(2000);
       Communication.clear();
       Communication.pushSprite(0,120);
       sendWeight = 0;
    }
    
    weight = getScaleValue();
    sendWeight = weight - TUMBLER_WEIGHT;
    MessageBox.clear();
    MessageBox.fillScreen(BLACK);
    MessageBox.setTextSize(1.5);
    sprintf(buffer,"  tumbler is empty ...");
    if (sendWeight > 0) {
      sprintf(buffer, "        Weight:%1d g", sendWeight);
      MessageBox.drawString("reflect to fitbit ?", 40, 30);
      MessageBox.drawString("please push B button a few seconds", 0, 50);
    }
    MessageBox.setTextSize(2);
    MessageBox.drawString(buffer, 0, 0);
    MessageBox.pushSprite(0,50);
    M5.update();
}

// 計測結果が安定したら値を返す
int getScaleValue() {
  int i=0;
  int count=0;
  while(i<MUST_COUNT && count<MAX_COUNT) {
      weight = scale.get_units(5);
      if (weight > 0) {
        i++;
      }
      count++;
      if (i >= MUST_COUNT) return weight;
  }
  return 0;
} 

void mqttLoop() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();
}

void connectMQTT() {
  while (!mqttClient.connected()) {
     if (mqttClient.connect(deviceID)) {
            lcd.println("Connected.");
        } else {
            lcd.print("Failed. Error state=");
            lcd.print(mqttClient.state());
            // Wait 5 seconds before retrying
            delay(5000);
        }
  }  
}
