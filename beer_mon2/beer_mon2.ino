/*********************************
*Title: BeerMon
*Version: 1
*Date: 30/10/2014
*Author: Ali Raheem
**********************************/
#include <SoftwareSerial.h>

#define LOW_LIMIT 15
#define HIGH_LIMIT 25
#define LED 13
#define BT_RX 3
#define BT_TX 2
#define NUM_SENSORS 5
#define DEFAULT_NAME "XXX: "

SoftwareSerial BT(BT_RX, BT_TX);

char flag = 1;
char * defaultName = strdup(DEFAULT_NAME);

struct tempSens{
  char * name = defaultName;
  double maxTemp;
  double minTemp;
  double curTemp;
  double avgTemp;
  int lowLimit = LOW_LIMIT;
  int highLimit = HIGH_LIMIT;
  int monitor = 0;
  };
  
tempSens probe[NUM_SENSORS];

void setup(){
  BT.begin(38400);
  //Use the internal 1.1v as a reference
  analogReference(INTERNAL);
  
//Setup probes
  probe[0].name = strdup("ALE: ");
  probe[0].monitor = 1;

  clearData();
}
void clearData(){
  int i;
  getTemp();
  for( i = 0; i < NUM_SENSORS; i++){
    probe[i].avgTemp = probe[i].curTemp;
    probe[i].maxTemp = -128;
    probe[i].minTemp = 128;
  }
  digitalWrite(LED, LOW);
}

void printHelp(){
  BT.println("\n\n############################");
  BT.println("BeerMon v1 Ali Raheem 2014");
  BT.println("############################");
  BT.println("\n\n?: Print this message.");
  BT.println("p: Print current temp.");
  BT.println("a: Print average temp.");
  BT.println("M: Print Max temp.");
  BT.println("m: Print Min temp.");
  BT.println("c: Clear data.");
}
void getTemp(){
  int i;
  for( i = 0; i < NUM_SENSORS; i++ ){
    double temp = analogRead(i);
    
    //Calculate temp from ADC
    temp *= 1.1;
    temp /= 1024;
    temp -= 0.424;
    temp /= 0.00625;
    probe[i].curTemp = temp;
    
    probe[i].avgTemp *= 10;
    probe[i].avgTemp += probe[i].curTemp;
    probe[i].avgTemp /= 11;
    
    if(probe[i].avgTemp > probe[i].maxTemp)
      probe[i].maxTemp = probe[i].avgTemp;
    if(probe[i].avgTemp < probe[i].minTemp)
      probe[i].minTemp = probe[i].avgTemp;
      
    if(probe[i].monitor){
      if(probe[i].highLimit <= probe[i].maxTemp)
        digitalWrite(LED, HIGH);
      if(probe[i].lowLimit >= probe[i].minTemp)
        digitalWrite(LED, HIGH);
    }
    
  }
}

void printAvgTemp(){
  int i;
  for( i = 0; i < NUM_SENSORS; i++){
    BT.print(probe[i].name);
    BT.print(probe[i].avgTemp);
    BT.println("°C");
  }
}
void printCurTemp(){
  int i;
  for( i = 0; i < NUM_SENSORS; i++){
    BT.print(probe[i].name);
    BT.print(probe[i].curTemp);
    BT.println("°C");
  }
}
void printMinTemp(){
  int i;
  for( i = 0; i < NUM_SENSORS; i++){
    BT.print(probe[i].name);
    BT.print(probe[i].minTemp);
    BT.println("°C");
  }
}

void printMaxTemp(){
  int i;
  for( i = 0; i < NUM_SENSORS; i++){
    BT.print(probe[i].name);
    BT.print(probe[i].maxTemp);
    BT.println("°C");
  }
}

void loop(){
  static unsigned long timer = 0;
    
  if( millis() > timer ){
    //Run every 5 minutes
    getTemp();
    timer = millis() + 300000L;
  }
  
  if(BT.available()){
    int inData = BT.read();
    Serial.print(inData);
    switch(inData){
      case 'a': printAvgTemp();
                break;
      case 'p': printCurTemp();
                break;
      case 'M': printMaxTemp();
                break;
      case 'm': printMinTemp();
                break;
      case 'c': clearData();
                break;
      case '?':
      default: BT.println("Not recognised.");
               printHelp();
    }
  }
}
