#include <X113647Stepper.h>
#include <PID_v1.h>
#include <Event.h>
#include <Timer.h>

//timers
Timer timerSerial;
Timer timerDrive;
Timer timerData;

//pins
// solinoids
#define sol4inf 2
#define sol4def 3
#define sol3inf 4
#define sol3def 5
#define sol2inf 6
#define sol2def 7
#define sol1inf 8
#define sol1def 9
// stepper flow valves
#define step1A1 10
#define step1B2 11
#define step1C3 12
#define step1D4 13
#define step2A1 22
#define step2B2 23
#define step2C3 24
#define step2D4 25
#define step3A1 26
#define step3B2 27
#define step3C3 28
#define step3D4 29
#define step4A1 30
#define step4B2 31
#define step4C3 32
#define step4D4 33

//buttons
#define butt1 34
#define butt2 35
//button variables
bool butt1State = false;
bool butt2State = false;

//control variables
volatile double target = 0;
volatile double current = 0;
volatile double current1 = 0;
volatile double current2 = 0;
volatile double currentx = 0;
volatile double currentx1 = 0;
volatile double currentx2 = 0;
double output = 0;
double Kp = 5;
double Ki = 0;
double Kd = 0;

//serial variables
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

//drive variables
volatile bool inf1 = false;
volatile bool def1 = false;
volatile bool inf2 = false;
volatile bool def2 = false;
volatile bool inf3 = false;
volatile bool def3 = false;
volatile bool inf4 = false;
volatile bool def4 = false;

// steppers
#define Resolution 10 //22 for fully open to fully closed 1-100
int motorSpeed = 6;
int stepsPerRevolution =  64 * 32;
X113647Stepper flowStep1(stepsPerRevolution, step1A1, step1B2, step1C3, step1D4);
X113647Stepper flowStep2(stepsPerRevolution, step2A1, step2B2, step2C3, step2D4);
X113647Stepper flowStep3(stepsPerRevolution, step3A1, step3B2, step3C3, step3D4);
X113647Stepper flowStep4(stepsPerRevolution, step4A1, step4B2, step4C3, step4D4);
int flowStep1Curent = 0;
int flowStep1Target = 0;
int flowStep2Curent = 0;
int flowStep2Target = 0;
int flowStep3Curent = 0;
int flowStep3Target = 0;
int flowStep4Curent = 0;
int flowStep4Target = 0;

void setup() {
  // put your setup code here, to run once:
  //setup pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode (sol1inf,OUTPUT);
  pinMode (sol1def,OUTPUT);
  pinMode (sol2inf,OUTPUT);
  pinMode (sol2def,OUTPUT);
  pinMode (sol3inf,OUTPUT);
  pinMode (sol3def,OUTPUT);
  pinMode (sol4inf,OUTPUT);
  pinMode (sol4def,OUTPUT);
  pinMode (butt1,INPUT_PULLUP);
  pinMode (butt2,INPUT_PULLUP);

  //set stepper speed
  flowStep1.setSpeed(motorSpeed);
  flowStep2.setSpeed(motorSpeed);
  flowStep3.setSpeed(motorSpeed);
  flowStep4.setSpeed(motorSpeed);

  //setup timers
  timerSerial.every(10, serial);
  timerDrive.every(50, drive);
  timerData.every(40, data);

    //setup serial
  Serial.begin(38400);
  Serial1.begin(9600);

  //button reading setup
  butt1State = digitalRead(butt1);
  butt2State = digitalRead(butt2);

  flowStep1Target = 20;
  flowStep2Target = 20;
  flowStep3Target = 20;
  flowStep4Target = 20;
  }

void loop() {
  // put your main code here, to run repeatedly:
  timerSerial.update();
  timerDrive.update();
  timerData.update();

  if (stringComplete) {
      
      inputString.remove(inputString.indexOf('!'));
      
      currentx = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);
      
      currentx1 = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);
      
      currentx2 = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);

      current = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);
      
      current1 = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);
      
      current2 = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);
      
      target = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);
      
      int tempKp = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);
      
      int tempKi = inputString.substring(0,inputString.indexOf(',')).toInt();
      inputString.remove(0,inputString.indexOf(',')+1);
      
      int tempKd = inputString.toInt();
      
      Kp = (double)tempKp/10;
      Ki = (double)tempKi/10;
      Kd = (double)tempKd/10;
      
      // clear the string:
      inputString = "";
      stringComplete = false;
    }

    //get steppers to inital positions
    butt1State = digitalRead(butt1);
    if(!butt1State){
      inf1 = true;
      inf2 = true;
      inf3 = true;
      inf4 = true;
    }
    else{
      inf1 = false;
      inf2 = false;
      inf3 = false;
      inf4 = false;
    }
    butt2State = digitalRead(butt2);
    if(!butt2State){
      def1 = true;
      def2 = true;
      def3 = true;
      def4 = true;
    }
    else{
      def1 = false;
      def2 = false;
      def3 = false;
      def4 = false;
    }
  }

//set output pins to current values
void drive(){
  //Serial.println("drive");
  //drive solinoids
  digitalWrite(sol1inf, inf1);
  digitalWrite(sol1def, def1);
  digitalWrite(sol2inf, inf2);
  digitalWrite(sol2def, def2);
  digitalWrite(sol3inf, inf3);
  digitalWrite(sol3def, def3);
  digitalWrite(sol4inf, inf4);
  digitalWrite(sol4def, def4);
  //drive steppers
  if(flowStep1Curent > flowStep1Target){
    flowStep1.step(-Resolution);
    flowStep1Curent-=1;
  }
  else if(flowStep1Curent < flowStep1Target){
    flowStep1.step(Resolution);
    flowStep1Curent+=1;
  }
  
  if(flowStep2Curent > flowStep2Target){
    flowStep2.step(-Resolution);
    flowStep2Curent-=1;
  }
  else if(flowStep2Curent < flowStep2Target){
    flowStep2.step(Resolution);
    flowStep2Curent+=1;
  }
  
  if(flowStep3Curent > flowStep3Target){
    flowStep3.step(-Resolution);
    flowStep3Curent-=1;
  }
  else if(flowStep3Curent < flowStep3Target){
    flowStep3.step(Resolution);
    flowStep3Curent+=1;
  }
  
  if(flowStep4Curent > flowStep4Target){
    flowStep4.step(-Resolution);
    flowStep4Curent-=1;
  }
  else if(flowStep4Curent < flowStep4Target){
    flowStep4.step(Resolution);
    flowStep4Curent+=1;
  }
  
}

//get serial data for target and current position
void serial(){
  
  Serial.println("A");

}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {

    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
        if (inChar == '!') {
      stringComplete = true;
        }
  }
}

void data(){
         Serial1.println(current);
            Serial1.println(',');

         Serial1.println(current1);
            Serial1.println(',');

         Serial1.println(current2);
            Serial1.println(',');
                  
         Serial1.println(currentx);
            Serial1.println(',');

         Serial1.println(currentx1);
            Serial1.println(',');
            
         Serial1.println(currentx2);
            Serial1.println(',');
  
         Serial1.println('!');
}
