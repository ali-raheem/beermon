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
#define BT_RX 2
#define BT_TX 3
//Uncomment GSM_ON to turn on GSM features.
//#define GSM_ON
#define GSM_RX 7
#define GSM_TX 8
#define NUM_SENSORS 1
#define DEFAULT_NAME "XXX: "
#define BT_NAME "BeerMon 1.0"
#define BT_PIN "9999"
#define MOB_NUM "+447939673619"

SoftwareSerial BT(BT_RX, BT_TX);

#ifdef GSM_ON
SoftwareSerial GSM(GSM_RX, GSM_TX);
#endif

int warned = 0;

struct tempSens{
  char * name ;
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
  //Use the internal 1.1v as a reference
  analogReference(INTERNAL);

  probe[0].name = strdup("ALE: ");
  probe[0].monitor = 1;
  clearData();

  BT.begin(9600);
  BT.print("AT+NAME");
  BT.print(BT_NAME);
  BT.print("\r\n");
  BT.print("AT+PIN");
  BT.print(BT_PIN);
  BT.print("\r\n");
  
  Serial.begin(19200);
  
#ifdef GSM_ON
  GSM.begin(19200);
  Serial.println("Powering on GSM module...");
  digitalWrite(9, HIGH);
  delay(1000);
  digitalWrite(9, LOW);
  delay(30000);
  GSM.print("AT+CMGF=1\r");
  delay(100);
  GSM.print("AT+CNMI=2,2,0,0,0\r"); 
  delay(100);
  Serial.println("Start up complete.");
  //sendSMS(MOB_NUM, "Powering on.");
#endif
}
 
void loop(){
  char inData;
  static unsigned long timer = 0;
    
  if( millis() > timer ){
    //Run every 5 minutes
    getTemp();
    timer = millis() + 300000L;
  }
  
  if(BT.available()){
    inData = BT.read();
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

#ifdef GSM_ON
  if(GSM.available()){
    inData = GSM.read();
    Serial.print(inData);
  }
#endif
}

#ifdef GSM_ON
void sendSMS(char *number, char *message){
  Serial.print("Sending \"");
  Serial.print(message);
  Serial.print("\" to number: ");
  Serial.println(number);

  GSM.print("AT+CMGF=1\r");
  delay(100);
  GSM.print("AT + CMGS = \"");
  GSM.print(number);
  GSM.println("\"");
  delay(100);
  GSM.println(message);
  delay(100);
  GSM.println((char)26);
  delay(100); 
  GSM.println();
  delay(5000);
}
#endif

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
  BT.println("#BeerMon v1 Ali Raheem 2014             #");
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
      
    if(probe[i].monitor & !warned){
      if(probe[i].highLimit <= probe[i].maxTemp){
        warned = 1;
        digitalWrite(LED, HIGH);
#ifdef GSM_ON
        sendSMS(MOB_NUM, "WARNING HIGH TEMP");
#endif
      }
      if(probe[i].lowLimit >= probe[i].minTemp){
        digitalWrite(LED, HIGH);
      }
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
