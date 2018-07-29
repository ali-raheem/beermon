#include <SoftwareSerial.h>
#define LOW_LIMIT 15
#define HIGH_LIMIT 25
#define LED 13
#define BT_RX 11
#define BT_TX 12
#define NUM_SENSORS 5

SoftwareSerial BT(BT_RX, BT_TX);
int inData;

struct tempSens{
  char * name = strdup("XXX: ");
  double maxTemp;
  double minTemp;
  double curTemp;
  double avgTemp;
  int lowLimit = LOW_LIMIT;
  int highLimit = HIGH_LIMIT;
  int monitor = 0;
  };
tempSens probe[5];

double vcc;

void setup(){
  BT.begin(9600);
//Setup probes
  probe[0].name = strdup("ALE: ");
  probe[0].monitor = 1;

  clear();
}
void clear(){
  int i;
  getTemp();
  for( i = 0; i < NUM_SENSORS; i++){
    probe[i].avgTemp = probe[i].curTemp;
    probe[i].maxTemp = -128;
    probe[i].minTemp = 128;
  }
  digitalWrite(LED, LOW);
}

void help(){
  BT.println("\n\n############################");
  BT.println("BeerMon v0.2 Ali Raheem 2014");
  BT.println("############################");
  BT.println("\n\n?: Print this message.");
  BT.println("p: Print current temp.");
  BT.println("a: Print average temp.");
  BT.println("M: Print Max temp.");
  BT.println("m: Print Min temp.");
  BT.println("c: Clear data.");
}
void getTemp(){
  vcc = readVcc()/1000.0;
  int i;
  for( i = 0; i < NUM_SENSORS; i++ ){
    double temp = analogRead(i);
    
    //Calculate temp from ADC
    temp *= vcc;
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
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}

void loop(){
  static unsigned long takeReading = 0;
    
  if( millis() > takeReading ){
    //Run every 5 minutes
    getTemp();
    takeReading = millis() + 300000L;
  }
  
  if(BT.available()){
    inData = BT.read();
    switch(inData){
      case 'a': printAvgTemp();
                break;
      case 'p': printCurTemp();
                break;
      case 'M': printMaxTemp();
                break;
      case 'm': printMinTemp();
                break;
      case 'c': clear();
                break;
      case '?':
      default: BT.println("Not recognised.");
               help();
    }
  }
}
