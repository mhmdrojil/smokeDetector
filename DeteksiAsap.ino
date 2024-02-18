#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Fuzzy.h>

#define BLYNK_TEMPLATE_ID "TMPL6CbUk4hV7"
#define BLYNK_TEMPLATE_NAME "Skripsi"
#define BLYNK_AUTH_TOKEN "zbJbmkRKQznBXnSz7PDHbsVOXDgpsQgw"
#define ssid "Redmi Note 10S"
#define pass "11111111"
#define DHTPIN 13 //D7
#define DHTTYPE DHT11   // DHT 11
#define mq2 A0


WidgetLCD lcd(V0);
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial mySerial(4, 5);
DFRobotDFPlayerMini myDFPlayer;
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput Asap
FuzzySet *tipis = new FuzzySet(-10, -2.5, -2.5, 5);
FuzzySet *sedang = new FuzzySet(4, 8, 8, 12);
FuzzySet *tebal = new FuzzySet(10, 15, 15, 400);

// FuzzyInput Suhu
FuzzySet *normal = new FuzzySet(0, 15, 15, 30);
FuzzySet *hangat = new FuzzySet(28, 35, 35, 42);
FuzzySet *panas = new FuzzySet(40, 45, 45, 50);

// FuzzyOutput
FuzzySet *off = new FuzzySet(0, 0, 0, 0);
FuzzySet *on = new FuzzySet(1, 1, 1, 1);

int threshold = 0;
BLYNK_WRITE(V4){
  int val = param.asInt();
  if(val ==1){
    updateThreshold();
  }
}

void setup(){
  Serial.begin(9600);
  mySerial.begin(9600);
  myDFPlayer.begin(mySerial);
  myDFPlayer.volume(30);
  konfigFuzzy();
  dht.begin();
  WiFi.disconnect(true);WiFi.mode(WIFI_OFF);
  WiFi.begin(ssid, pass); WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("");
  Serial.print("Terhubung ke WiFi. IP Address: ");
  Serial.println(WiFi.localIP());
  myDFPlayer.play(1); delay(3000);
  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str(),"blynk.cloud", 8080);
  Serial.println("Terhubung ke server Blynk!");
  myDFPlayer.play();
  hitungThreshold();
  delay(3000);

}

void loop(){
  Blynk.run();
  lcd.clear();
  int asap = deteksiAsap();
  float suhu = dht.readTemperature();
  
  Serial.print("\nAsap : ");
  Serial.print(asap);
  Serial.print("\tSuhu : ");
  Serial.print(suhu);

  Blynk.virtualWrite(V1, asap);
  Blynk.virtualWrite(V2, suhu);

  fuzzy->setInput(1, asap);
  fuzzy->setInput(2, suhu);
  fuzzy->fuzzify(); 
  bacaStatusKeanggotaan();
  float output = fuzzy->defuzzify(1);
  Serial.print("\tOutput :");
  Serial.print(output);
  Blynk.virtualWrite(V3, output);
  hasilFuzzy(output);
  if(asap <= -20) updateThreshold();
  delay(1000);
}

int deteksiAsap(){
  int nilaiAnalog = analogRead(mq2);
  return nilaiAnalog - threshold;
}

void updateThreshold(){
  Serial.println("Menghitung Threshold");
  lcd.clear();lcd.print(0, 0, "Menghitung"); lcd.print(0, 1, "Threshold");
  myDFPlayer.play(2);
  hitungThreshold();
}

int hitungThreshold(){
  int totalData = 0;
  int jumlahHitung = 20;

  for(int a = 0; a < jumlahHitung; a++){
    totalData += analogRead(mq2);
    delay(500);
  } 
  return threshold = totalData / jumlahHitung;
}

void hasilFuzzy(float output){
  if(output == 1){
    myDFPlayer.play(3);
    Serial.print("\tPeringatan Aktif");
    Blynk.setProperty(V0, "color", "#FF0000");
    lcd.print(0, 1, "Peringatan Aktif");
    delay(3000);
  }
  else{
    Serial.print("\tStatus Aman");
    Blynk.setProperty(V0, "color", "#00FF00");
    lcd.print(0, 1, "Status Aman");
  }
}

void konfigFuzzy(){
  FuzzyInput *asap = new FuzzyInput(1);
    asap->addFuzzySet(tipis);
    asap->addFuzzySet(sedang);
    asap->addFuzzySet(tebal);
  fuzzy->addFuzzyInput(asap);

  FuzzyInput *suhu = new FuzzyInput(2);
    suhu->addFuzzySet(normal);
    suhu->addFuzzySet(hangat);
    suhu->addFuzzySet(panas);
  fuzzy->addFuzzyInput(suhu);

  FuzzyOutput *hasil = new FuzzyOutput(1);
    hasil->addFuzzySet(off);
    hasil->addFuzzySet(on);
  fuzzy->addFuzzyOutput(hasil);

  FuzzyRuleAntecedent *ifTipisAndNormal = new FuzzyRuleAntecedent();
  ifTipisAndNormal->joinWithAND(tipis, normal);
  FuzzyRuleConsequent *thenOff1 = new FuzzyRuleConsequent();
  thenOff1->addOutput(off);
  FuzzyRule *fuzzyRule01 = new FuzzyRule(1, ifTipisAndNormal, thenOff1);
  fuzzy->addFuzzyRule(fuzzyRule01);

  FuzzyRuleAntecedent *ifTipisAndHangat = new FuzzyRuleAntecedent();
  ifTipisAndHangat->joinWithAND(tipis, hangat);
  FuzzyRuleConsequent *thenOff2 = new FuzzyRuleConsequent();
  thenOff2->addOutput(off);
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, ifTipisAndHangat, thenOff2);
  fuzzy->addFuzzyRule(fuzzyRule02);

  FuzzyRuleAntecedent *ifTipisAndPanas = new FuzzyRuleAntecedent();
  ifTipisAndPanas->joinWithAND(tipis, panas);
  FuzzyRuleConsequent *thenOn1 = new FuzzyRuleConsequent();
  thenOn1->addOutput(on);
  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, ifTipisAndPanas, thenOn1);
  fuzzy->addFuzzyRule(fuzzyRule03); 
  
  FuzzyRuleAntecedent *ifSedangAndNormal = new FuzzyRuleAntecedent();
  ifSedangAndNormal->joinWithAND(sedang, normal);
  FuzzyRuleConsequent *thenOff3 = new FuzzyRuleConsequent();
  thenOff3->addOutput(off);
  FuzzyRule *fuzzyRule04 = new FuzzyRule(4, ifSedangAndNormal, thenOff3);
  fuzzy->addFuzzyRule(fuzzyRule04);

  FuzzyRuleAntecedent *ifSedangAndHangat = new FuzzyRuleAntecedent();
  ifSedangAndHangat->joinWithAND(sedang, hangat);
  FuzzyRuleConsequent *thenOff4 = new FuzzyRuleConsequent();
  thenOff4->addOutput(off);
  FuzzyRule *fuzzyRule05 = new FuzzyRule(5, ifSedangAndHangat, thenOff4);
  fuzzy->addFuzzyRule(fuzzyRule05);

  FuzzyRuleAntecedent *ifSedangAndPanas = new FuzzyRuleAntecedent();
  ifSedangAndPanas->joinWithAND(sedang, panas);
  FuzzyRuleConsequent *thenOn2 = new FuzzyRuleConsequent();
  thenOn2->addOutput(on);
  FuzzyRule *fuzzyRule06 = new FuzzyRule(6, ifSedangAndPanas, thenOn2);
  fuzzy->addFuzzyRule(fuzzyRule06);

  FuzzyRuleAntecedent *ifTebalAndNormal = new FuzzyRuleAntecedent();
  ifTebalAndNormal->joinWithAND(tebal, normal);
  FuzzyRuleConsequent *thenOn3 = new FuzzyRuleConsequent();
  thenOn3->addOutput(on);
  FuzzyRule *fuzzyRule07 = new FuzzyRule(7, ifTebalAndNormal, thenOn3);
  fuzzy->addFuzzyRule(fuzzyRule07);

  FuzzyRuleAntecedent *ifTebalAndHangat = new FuzzyRuleAntecedent();
  ifTebalAndHangat->joinWithAND(tebal, hangat);
  FuzzyRuleConsequent *thenOn4 = new FuzzyRuleConsequent();
  thenOn4->addOutput(on);
  FuzzyRule *fuzzyRule08 = new FuzzyRule(8, ifTebalAndHangat, thenOn4);
  fuzzy->addFuzzyRule(fuzzyRule08);
  
  FuzzyRuleAntecedent *ifTebalAndPanas = new FuzzyRuleAntecedent();
  ifTebalAndPanas->joinWithAND(tebal, panas);
  FuzzyRuleConsequent *thenOn5 = new FuzzyRuleConsequent();
  thenOn5->addOutput(on);
  FuzzyRule *fuzzyRule09 = new FuzzyRule(9, ifTebalAndPanas, thenOn5);
  fuzzy->addFuzzyRule(fuzzyRule09);
}

void bacaStatusKeanggotaan(){
  float derajat_tipis = tipis->getPertinence();
  float derajat_sedang = sedang->getPertinence();
  float derajat_tebal = tebal->getPertinence();

  float derajat_normal = normal->getPertinence();
  float derajat_hangat = hangat->getPertinence();
  float derajat_panas = panas->getPertinence();

  String statusAsap = "Tipis";
  if (derajat_sedang> 0.5) {
    statusAsap = "Sedang";
  } else if (derajat_tebal> 0.5) {
    statusAsap = "Tebal";
  }


  String statusSuhu = "Normal";
  if (derajat_hangat> 0.5) {
    statusSuhu = "Hangat";
  } else if (derajat_panas> 0.5) {
    statusSuhu = "Panas";
  }
  Serial.print("\t"); Serial.print(statusAsap); lcd.print(0, 0, statusAsap);
  Serial.print("\t"); Serial.print(statusSuhu); lcd.print(7, 0, statusSuhu);
}