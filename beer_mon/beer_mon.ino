#include <SoftwareSerial.h>
#define lowLimit 15
#define highLimit 25
#define LED 13
#define BT_RX 11
#define BT_TX 12
#define NUMSENSORS 5

SoftwareSerial BT(BT_RX, BT_TX);
int inData;
double maxTemp[NUMSENSORS];
double minTemp[NUMSENSORS];
double curTemp[NUMSENSORS];
double avgTemp[NUMSENSORS];
double vcc;

void setup(){
  BT.begin(9600);
  delay(5);
  clear();
}
void clear(){
  vcc = readVcc()/1000.0;
  int i;
  for( i = 0; i < NUMSENSORS; i++){
    getTemp(i);
    avgTemp[i] = curTemp[i];
    maxTemp[i] = -128;
    minTemp[i] = 128;
  }
  digitalWrite(LED, LOW);
}

void help(){
  BT.println("\n\n############################");
  BT.println("BeerMon v0.1 Ali Raheem 2014");
  BT.println("############################");
  BT.println("\n\n?: Print this message.");
  BT.println("p: Print current temp.");
  BT.println("a: Print average temp.");
  BT.println("M: Print Max temp.");
  BT.println("m: Print Min temp.");
  BT.println("c: Clear data.");
}
void getTemp(int i){
  double temp = analogRead(i);
  temp *= vcc;
  temp /= 1024;
  temp -= 0.424;
  temp /= 0.00625;
  curTemp[i] = temp;
  avgTemp[i] *= 10;
  avgTemp[i] += curTemp[i];
  avgTemp[i] /= 11;
  if(avgTemp[i] > maxTemp[i])
    maxTemp[i] = avgTemp[i];
  if(avgTemp[i] < minTemp[i])
    minTemp[i] = avgTemp[i];
  if(highLimit <= maxTemp[i])
    digitalWrite(LED, HIGH);
  if(lowLimit >= minTemp[i])
    digitalWrite(LED, HIGH);
}

void printAvgTemp(){
  int i;
  for( i = 0; i < NUMSENSORS; i++)
    BT.println(avgTemp[i]);
}
void printCurTemp(){
  int i;
  for( i = 0; i < NUMSENSORS; i++)
    BT.println(curTemp[i]);
}
void printMinTemp(){
  int i;
  for( i = 0; i < NUMSENSORS; i++)
    BT.println(minTemp[i]);
}
void printMaxTemp(){
  int i;
  for( i = 0; i < NUMSENSORS; i++)
    BT.println(maxTemp[i]);
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
    vcc = readVcc()/1000.0;
    int i;
    for( i = 0; i < NUMSENSORS; i++)
      getTemp(i);
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
