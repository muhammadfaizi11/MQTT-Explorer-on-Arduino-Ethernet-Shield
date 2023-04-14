//Library Ethernet & MQTT Explorer
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

//Library LCD
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//Variabel LCD
LiquidCrystal_I2C lcd(0x27, 20,4); 

//Deklarasi Library Sensor Suhu
#include <OneWire.h>
#include <DallasTemperature.h>

//Konfigurasi Ethernet Address
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 102); //Ethernet Shield
IPAddress server(192, 168, 7, 244); //Server

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

//Variabel Sensor Suhu
int pinSuhu = 30;
float suhu;

//Variabel Sensor Ultrasonik
int pinTrig = 5;
int pinEcho = 6;
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
long durasi;
float jarakCm, jarakInch, tinggiAir;

//Variabel Sensor Analog Kekeruhan dan Konduktivitas
float turbid, conduct;
float koefTurbid = 0.4597; //0.5937
float konstTurbid = -3.8775; //1.3775
//float koefConduct = 1.3169;
float koefConduct = 0.7869; //1.3169
float konstConduct = -3.2897; //-2.1987

//Variabel Pin Relay
int pinRelay1 = 3; //Sensor Analog
int pinRelay2 = 2; //Buzzer Emergency
int pinRelay3 = 32; //Pilot Lamp Temp Indicator
int pinRelay4 = 9; //Pilot Lamp Distance Indicator
int pinRelay5 = 8; //Pilot Lamp Turbidity Indicator
int pinRelay6 = 7; //Pilot Lamp Conductivity Indicator

//Variabel Kondisi dan Status Sistem
int buzzer;
int lampSuhu;
int lampTinggi;
int lampTurbid;
int lampConduct;
String kondisiSuhu;
String kondisiTinggi;
String kondisiTurbid;
String kondisiConduct;
String kondisiAll;
char suhuOut[20];
char tinggiAirOut[20];
char turbidOut[20];
char conductOut[20];
char kondisiSuhuOut[20];
char kondisiTinggiAirOut[20];
char kondisiTurbidOut[20];
char kondisiConductOut[20];
char kondisiAllOut[20];

//Setup onewire to commpiunicate with any other onewire (Sensor Suhu)
OneWire oneWire(pinSuhu);

//Onewire to dallas temperature (Sensor Suhu)
DallasTemperature sensors(&oneWire);

//Variabel Millis
unsigned long currentTime;
unsigned long prevTime, prevTime2, prevTime3, prevTime4, prevTime5;
unsigned long sensorTime;
unsigned long lcdTime1, lcdTime2, lcdTime3, lcdTime4;
unsigned long relayTime;

int stateRelay;
int stateSensor;
int statusBaca;
int timer1, timer2, timer3, timer4;
int stateLCD1, stateLCD2, stateLCD3, stateLCD4;

//Fungsi untuk Sensor Suhu
void sensorSuhu(){
  sensors.requestTemperatures();
  suhu = sensors.getTempCByIndex(0);
  suhu = random(18.00,50.00);
  if(suhu < 0){
    suhu = random(29.00,33.00);
  }
}

//Fungsi untuk Sensor Ultrasonik
void sensorUltrasonik(){
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(50);
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(100);
  digitalWrite(pinTrig, HIGH);
  durasi = pulseIn(pinEcho, HIGH);
  //calculate the distance
  jarakCm = durasi * SOUND_SPEED / 2;
//  jarakInch = jarakCm * CM_TO_INCH;
  tinggiAir = 30 - jarakCm;
  if(tinggiAir < 0){
    tinggiAir = 0;
  }
  tinggiAir = random(0.00,30.00);
}

//Fungsi untuk Sensor Analog
void sensorAnalog(){
   //Baca Nilai ADC
  int adcRead = 0;
  int looping = 10;
  for(int i=0; i<looping; i++){
    adcRead += analogRead(A0);
    delay(10);
  }
  int avgADC = adcRead / looping;

  //Nilai Sensor
  turbid = (avgADC*koefTurbid)+konstTurbid;
  conduct = (avgADC*koefConduct)+konstConduct;
  if(turbid < 0){
    turbid = 0;
  }
  if(conduct < 0){
    conduct = 0;
  }

  turbid = random(10.00,100.00);
  conduct = random(25.00,100.00);
}

//Fungsi untuk Kondisi Buzzer Emergency dan Pilot Lamp Indicator beserta Status Sistem
void kondisiAir(){
  //Rule 1 (000)
  if(suhu < 38 && tinggiAir > 15 && turbid < 50 && conduct < 50){
    Serial.println("Buzzer Emergency Off");
    Serial.println("All Pilot Lamp Indicator Off");
    digitalWrite(pinRelay2, HIGH);
    digitalWrite(pinRelay3, HIGH);
    digitalWrite(pinRelay4, HIGH);
    digitalWrite(pinRelay5, HIGH);
    digitalWrite(pinRelay6, HIGH);
    kondisiAll = "Normal";
    kondisiSuhu = "Low";
    kondisiTinggi = "High";
    kondisiTurbid = "Low";
    kondisiConduct = "Low";
  }
  //Rule 2 (0001)
  else if(suhu < 38 && tinggiAir > 15 && turbid < 50 && conduct > 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Conduct Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, HIGH);
    digitalWrite(pinRelay4, HIGH);
    digitalWrite(pinRelay5, HIGH);
    digitalWrite(pinRelay6, LOW);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "Low";
    kondisiTinggi = "High";
    kondisiTurbid = "Low";
    kondisiConduct = "High";
  }
  //Rule 3 (0010)
  else if(suhu < 38 && tinggiAir > 15 && turbid > 50 && conduct < 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Turbid Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, HIGH);
    digitalWrite(pinRelay4, HIGH);
    digitalWrite(pinRelay5, LOW);
    digitalWrite(pinRelay6, HIGH);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "Low";
    kondisiTinggi = "High";
    kondisiTurbid = "High";
    kondisiConduct = "Low";
  }
  //Rule 4 (0011)
  else if(suhu < 38 && tinggiAir > 15 && turbid > 50 && conduct > 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Turbid-Conduct Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, HIGH);
    digitalWrite(pinRelay4, HIGH);
    digitalWrite(pinRelay5, LOW);
    digitalWrite(pinRelay6, LOW);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "Low";
    kondisiTinggi = "High";
    kondisiTurbid = "High";
    kondisiConduct = "High";
  }
  //Rule 5 (0100)
  else if(suhu < 38 && tinggiAir < 15 && turbid < 50 && conduct < 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Height Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, HIGH);
    digitalWrite(pinRelay4, LOW);
    digitalWrite(pinRelay5, HIGH);
    digitalWrite(pinRelay6, HIGH);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "Low";
    kondisiTinggi = "Low";
    kondisiTurbid = "Low";
    kondisiConduct = "Low";
  }
  //Rule 6 (0101)
  else if(suhu < 38 && tinggiAir < 15 && turbid < 50 && conduct > 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Height-Conduct Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, HIGH);
    digitalWrite(pinRelay4, LOW);
    digitalWrite(pinRelay5, HIGH);
    digitalWrite(pinRelay6, LOW);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "Low";
    kondisiTinggi = "Low";
    kondisiTurbid = "Low";
    kondisiConduct = "High";
  }
   //Rule 7 (0110)
  else if(suhu < 38 && tinggiAir < 15 && turbid > 50 && conduct < 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Height-Turbid Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, HIGH);
    digitalWrite(pinRelay4, LOW);
    digitalWrite(pinRelay5, LOW);
    digitalWrite(pinRelay6, HIGH);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "Low";
    kondisiTinggi = "Low";
    kondisiTurbid = "High";
    kondisiConduct = "Low";
  }
  //Rule 8 (0111)
  else if(suhu < 38 && tinggiAir < 15 && turbid > 50 && conduct > 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Height-Turbid-Conduct Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, HIGH);
    digitalWrite(pinRelay4, LOW);
    digitalWrite(pinRelay5, LOW);
    digitalWrite(pinRelay6, LOW);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "Low";
    kondisiTinggi = "Low";
    kondisiTurbid = "High";
    kondisiConduct = "High";
  }
  //Rule 9 (1000)
  else if(suhu > 38 && tinggiAir > 15 && turbid < 50 && conduct < 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Temp Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    digitalWrite(pinRelay4, HIGH);
    digitalWrite(pinRelay5, HIGH);
    digitalWrite(pinRelay6, HIGH);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "High";
    kondisiTinggi = "High";
    kondisiTurbid = "Low";
    kondisiConduct = "Low";
  }
  //Rule 10 (1001)
  else if(suhu > 38 && tinggiAir > 15 && turbid < 50 && conduct > 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Temp-Conduct Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    digitalWrite(pinRelay4, HIGH);
    digitalWrite(pinRelay5, HIGH);
    digitalWrite(pinRelay6, LOW);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "High";
    kondisiTinggi = "High";
    kondisiTurbid = "Low";
    kondisiConduct = "High";
  }
  //Rule 11 (1010)
  else if(suhu > 38 && tinggiAir > 15 && turbid > 50 && conduct < 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Temp-Turbid Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    digitalWrite(pinRelay4, HIGH);
    digitalWrite(pinRelay5, LOW);
    digitalWrite(pinRelay6, HIGH);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "High";
    kondisiTinggi = "High";
    kondisiTurbid = "High";
    kondisiConduct = "Low";
  }
  //Rule 12 (1011)
  else if(suhu > 38 && tinggiAir > 15 && turbid > 50 && conduct > 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Temp-Turbid-Conduct Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    digitalWrite(pinRelay4, HIGH);
    digitalWrite(pinRelay5, LOW);
    digitalWrite(pinRelay6, LOW);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "High";
    kondisiTinggi = "High";
    kondisiTurbid = "High";
    kondisiConduct = "High";
  }
  //Rule 13 (1100)
  else if(suhu > 38 && tinggiAir < 15 && turbid < 50 && conduct < 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Temp-Height Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    digitalWrite(pinRelay4, LOW);
    digitalWrite(pinRelay5, HIGH);
    digitalWrite(pinRelay6, HIGH);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "High";
    kondisiTinggi = "Low";
    kondisiTurbid = "Low";
    kondisiConduct = "Low";
  }
  //Rule 14 (1101)
  else if(suhu > 38 && tinggiAir < 15 && turbid < 50 && conduct > 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Temp-Height-Conduct Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    digitalWrite(pinRelay4, LOW);
    digitalWrite(pinRelay5, HIGH);
    digitalWrite(pinRelay6, LOW);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "High";
    kondisiTinggi = "Low";
    kondisiTurbid = "Low";
    kondisiConduct = "High";
  }
  //Rule 15 (1110)
  else if(suhu > 38 && tinggiAir > 15 && turbid > 50 && conduct < 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("Pilot Lamp Temp-Height-Turbid Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    digitalWrite(pinRelay4, LOW);
    digitalWrite(pinRelay5, LOW);
    digitalWrite(pinRelay6, HIGH);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "High";
    kondisiTinggi = "Low";
    kondisiTurbid = "High";
    kondisiConduct = "Low";
  }
  //Rule 16 (1111)
  else if(suhu > 38 && tinggiAir > 15 && turbid > 50 && conduct > 50){
    Serial.println("Buzzer Emergency On"); 
    Serial.println("All Pilot Lamp Indicator On");
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    digitalWrite(pinRelay4, LOW);
    digitalWrite(pinRelay5, LOW);
    digitalWrite(pinRelay6, LOW);
    kondisiAll = "Not_Normal";
    kondisiSuhu = "High";
    kondisiTinggi = "Low";
    kondisiTurbid = "High";
    kondisiConduct = "High";
  }
}

//Fungsi untuk Display Sensor Suhu ke LCD
void displaySuhu(){
  //Printing the values on the serial monitor
  lcd.clear();
  Serial.print("Suhu = ");
  Serial.println(suhu);
  //Display ke LCD
  lcd.setCursor(6,0);
  lcd.print("Suhu Air");
  lcd.setCursor(0,1);
  lcd.print("Suhu    : ");
  lcd.setCursor(10,1);
  lcd.print(suhu);
  lcd.setCursor(0,2);
  lcd.print("Kondisi : ");
  lcd.setCursor(10,2);
  lcd.print(kondisiSuhu);
  lcd.setCursor(0,3);
  lcd.print("Sistem  : ");
  lcd.setCursor(10,3);
  lcd.print(kondisiAll);
//  lcd.clear();
}

//Fungsi untuk Display Sensor Ultrasonik ke LCD
void displayTinggi(){
  //Printing the values on the serial monitor
  Serial.print("Tinggi Air = ");
  Serial.println(tinggiAir);
  //Display ke LCD
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Ketinggian Air");
  lcd.setCursor(0,1);
  lcd.print("Tinggi  : ");
  lcd.setCursor(10,1);
  lcd.print(tinggiAir);
  lcd.setCursor(0,2);
  lcd.print("Kondisi : ");
  lcd.setCursor(10,2);
  lcd.print(kondisiTinggi);
  lcd.setCursor(0,3);
  lcd.print("Sistem  : ");
  lcd.setCursor(10,3);
  lcd.print(kondisiAll);
//  lcd.clear();
}

//Fungsi untuk Display Sensor Kekeruhan ke LCD
void displayTurbid(){
  lcd.clear();
  //Printing the values on the serial monitor
  Serial.print("Kekeruhan = ");
  Serial.println(turbid);
  //Display ke LCD
  lcd.setCursor(4,0);
  lcd.print("Kekeruhan Air");
  lcd.setCursor(0,1);
  lcd.print("Kekeruhan : ");
  lcd.setCursor(12,1);
  lcd.print(turbid);
  lcd.setCursor(0,2);
  lcd.print("Kondisi : ");
  lcd.setCursor(10,2);
  lcd.print(kondisiTurbid);
  lcd.setCursor(0,3);
  lcd.print("Sistem  : ");
  lcd.setCursor(10,3);
  lcd.print(kondisiAll);
//  lcd.clear();
}

//Fungsi untuk Display Sensor Konduktivitas ke LCD
void displayConduct(){
  lcd.clear();
  //Printing the values on the serial monitor
  Serial.print("Konduktivitas = ");
  Serial.println(conduct);
  //Display ke LCD
  lcd.setCursor(2,0);
  lcd.print("Konduktivitas Air");
  lcd.setCursor(0,1);
  lcd.print("Konduktif : ");
  lcd.setCursor(12,1);
  lcd.print(conduct);
  lcd.setCursor(0,2);
  lcd.print("Kondisi : ");
  lcd.setCursor(10,2);
  lcd.print(kondisiConduct);
  lcd.setCursor(0,3);
  lcd.print("Sistem  : ");
  lcd.setCursor(10,3);
  lcd.print(kondisiAll);
//  lcd.clear();
}

//Fungsi untuk Kirim Data
void sendData(){
  //Konversi Float ke Char
  dtostrf(suhu,2,2,suhuOut); 
  dtostrf(tinggiAir,2,2,tinggiAirOut);
  dtostrf(turbid,2,2,turbidOut);
  dtostrf(conduct,2,2,conductOut);

  //Konversi String ke Char
//  int str_len1 = kondisiSuhu.length() + 1;
//  kondisiSuhu.toCharArray(kondisiSuhuOut, str_len1);
//  int str_len2 = kondisiTinggi.length() + 1;
//  kondisiTinggi.toCharArray(kondisiTinggiAirOut, str_len2);
//  int str_len3 = kondisiTurbid.length() + 1;
//  kondisiTurbid.toCharArray(kondisiTurbidOut, str_len3);
//  int str_len4 = kondisiConduct.length() + 1;
//  kondisiConduct.toCharArray(kondisiConductOut, str_len4);
//  int str_len5 = kondisiAll.length() + 1;
//  kondisiAll.toCharArray(kondisiAllOut, str_len5);
  
  //Kirim ke MQTT Explorer
  if (client.connect("MillC1", "faizi", "mqtt-faizi")){
    Serial.println("Data Terkirim!");
    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/SUHU", suhuOut);
//    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/KONDISI_SUHU", kondisiSuhuOut);
    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/TINGGI_AIR", tinggiAirOut);
//    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/KONDISI_TINGGI_AIR", kondisiTinggiAirOut);
    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/TURBID", turbidOut);
//    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/KONDISI_TURBID", kondisiTurbidOut);
    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/CONDUCT", conductOut);
//    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/KONDISI_CONDUCT", kondisiConductOut);
//    client.publish("SPINDO/UNIT6/PRODUKSI/MILLC1/AIR_DEMIN/STATUS_SISTEM", kondisiAllOut);
  }
  else {
    Serial.println("Data Tidak Terkirim!");
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());

  //Inisialisasi Sensor Suhu
  sensors.begin(); //Start Sensor DS18B20
  
  //Inisialisasi LCD Crystal 20x4 I2C
  lcd.init(); //Initialize LCD
  lcd.backlight();
  lcd.setCursor(2,0); //(0 menunjukan kolom ke, 1 menunjukan baris ke)
  lcd.print("PT SPINDO UNIT VI");
  lcd.setCursor(0,1);
  lcd.print("SIDOARJO, JAWA TIMUR");
  lcd.setCursor(1,2);
  lcd.print("MUHAMMAD NUR FAIZI");
  lcd.setCursor(2,3);
  lcd.print("DEPARTMEN TEKNIK");

  //Inisialisasi Pin Mode
  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);
  pinMode(pinRelay1, OUTPUT);
  pinMode(pinRelay2, OUTPUT);
  pinMode(pinRelay3, OUTPUT);
  pinMode(pinRelay4, OUTPUT);
  pinMode(pinRelay5, OUTPUT);
  pinMode(pinRelay6, OUTPUT);
  digitalWrite(pinRelay1, HIGH);
  digitalWrite(pinRelay2, HIGH);
  digitalWrite(pinRelay3, HIGH);
  digitalWrite(pinRelay4, HIGH);
  digitalWrite(pinRelay5, HIGH);
  digitalWrite(pinRelay6, HIGH);
  delay(1000);
  lcd.clear();
}

void loop() {
  // put your main code here, to run repeatedly:
  currentTime = millis();
  timer1 = 3000;
  timer2 = 5000;
  timer3 = 10000;
  timer4 = 1000;

  //Tampil ke LCD
  if(currentTime - lcdTime1 >= timer1 && stateLCD1 == 0){
    prevTime2 = currentTime;
    Serial.println("Display LCD 1 On");
    displaySuhu();
    stateLCD1 = 1;
  }

  if(stateLCD1 == 1){
    lcdTime2 = currentTime;
    if(lcdTime2 - prevTime2 >= timer1 && stateLCD2 == 0){
      prevTime3 = currentTime;
      Serial.println("Display LCD 2 On");
      displayTinggi();
      stateLCD2 = 1;
    }
  }

  if(stateLCD2 == 1){
    lcdTime3 = currentTime;
    if(lcdTime3 - prevTime3 >= timer1 && stateLCD3 == 0){
      prevTime4 = currentTime;
      Serial.println("Display LCD 3 On");
      displayTurbid();
      stateLCD3 = 1; 
    }
  }

  if(stateLCD3 == 1){
    lcdTime4 = currentTime;
    if(lcdTime4 - prevTime4 >= timer1 && stateLCD4 == 0){
      Serial.println("Display LCD 4 On");
      displayConduct();
      stateLCD1 = 0;
      stateLCD2 = 0;
      stateLCD3 = 0;
      stateLCD4 = 0;
      lcdTime1 = currentTime;
    }
  }

  //Kirim ke MQTT tiap 30 detik
  if(currentTime - prevTime5 >= timer4){
    Serial.println("Pengiriman Data");
    prevTime5 = currentTime;
    Serial.println("Kirim ke MQTT Explorer");
    sendData();
  }

  //Baca Sensor tiap 5 detik u/ suhu & ultrasonik dan 15 detik u/ analog
  if(currentTime - prevTime >= timer2 && stateSensor == 0){
    relayTime = currentTime;
    Serial.println("Sensor On");
    digitalWrite(pinRelay1, LOW);
    sensorSuhu();
    sensorUltrasonik();
    stateSensor = 1;
  }

  if(stateSensor == 1){
    sensorTime = currentTime;
    if(sensorTime - relayTime >= timer3 && statusBaca == 0){
      Serial.println("Get Data Analog Sensor");
      sensorAnalog();
      kondisiAir();
      statusBaca = 1;
    }

    if(statusBaca == 1){
      Serial.println("Analog Sensor Off");
      digitalWrite(pinRelay1, HIGH);
      stateSensor = 0;
      statusBaca = 0;
      prevTime = currentTime;
    }
  }
} 
