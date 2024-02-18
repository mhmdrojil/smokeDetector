#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_TEMPLATE_ID "TMPL6CbUk4hV7"
#define BLYNK_TEMPLATE_NAME "Skripsi"
#define BLYNK_AUTH_TOKEN "zbJbmkRKQznBXnSz7PDHbsVOXDgpsQgw"
#define ssid "Redmi Note 10S"
#define pass "11111111"

WidgetLCD lcd(V0);
SoftwareSerial mySerial(4, 5);
DFRobotDFPlayerMini myDFPlayer;

int threshold = 0;

void setup(){
  Serial.begin(9600);
  mySerial.begin(9600);
  myDFPlayer.begin(mySerial);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  myDFPlayer.begin(mySerial);
  myDFPlayer.volume(30);

}

void loop(){
  Blynk.run();
  lcd.clear();
  myDFPlayer.volume(30);
  for(int a = 1; a < 6; a++){
    myDFPlayer.play(a);
    Serial.println(a);
    delay(3000);
  }
}
