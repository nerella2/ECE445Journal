#include <TM1637Display.h>



#include <Wire.h>
#include <RTClib.h>



#define dio_pin 13
#define clk_pin 12
u_int32_t seconds;
u_int32_t minutes;
u_int32_t hours;
u_int32_t alarmMinutes;
u_int32_t alarmHours;
bool timekeeping;
bool measuring;
bool changeAlarm;
bool changeTime;
bool alarmSet;
bool beep;
bool turn;
bool sound;
bool alarmAM;
bool realAM;
bool distWait;
unsigned long lastMillis;
unsigned long b1Millis;
unsigned long b2Millis;
unsigned long b3Millis;
unsigned long b4Millis;
unsigned long b5Millis;
unsigned long testMillis;
unsigned long motorMillis;
unsigned long curMillis;
unsigned long distMillis;
unsigned long distTime;
unsigned long turnMillis;
int displayTime;
TM1637Display display(clk_pin, dio_pin);

void setup() {
  Serial.begin(9600);
  b1Millis=millis();
  b2Millis=millis();
  b3Millis=millis();
  b4Millis=millis();
  b5Millis=millis();
  motorMillis=millis();
  distMillis=millis();
  distTime=millis();
  turnMillis=millis();
  testMillis=millis();
  timekeeping=true;
  sound=false;
  alarmAM=true;
  realAM=true;
  hours=1;
  minutes=0;
  alarmHours=1;
  alarmMinutes=1;
  seconds=50;
  alarmSet=true;
  distWait=false;
  measuring=false;
  turn=false;
  lastMillis=millis();
  pinMode(2,OUTPUT);  //Speaker
  pinMode(35,INPUT);  //Off Button
  pinMode(32,INPUT);  //Hours Button
  pinMode(21,INPUT);  //Minutes Button
  pinMode(14,INPUT);  //Alarm set button
  pinMode(17,OUTPUT); //LED
  pinMode(34,INPUT);  //Change Alarm
  pinMode(18,OUTPUT); //Ultrasonic Echo
  pinMode(19,INPUT);  //Ultrasonic Trigger
  pinMode(13,OUTPUT); //Display pins
  pinMode(12,OUTPUT); //Display pins
  pinMode(26,OUTPUT);
  pinMode(25,OUTPUT);
  pinMode(27,OUTPUT);
  pinMode(33,OUTPUT);

  beep=false;
  digitalWrite(17,HIGH);
  display.setBrightness(7);
  displayTime=hours*100+minutes;
  display.showNumberDecEx(displayTime,0b01000000);
  
}
void rightTurn(){
  digitalWrite(33,HIGH);
  digitalWrite(27,LOW);
  digitalWrite(26,LOW);
  digitalWrite(25,LOW);
}
void leftTurn(){
  digitalWrite(33,LOW);
  digitalWrite(27,LOW);
  digitalWrite(26,LOW);
  digitalWrite(25,HIGH);
}
void straight(){
  digitalWrite(33,HIGH);
  digitalWrite(27,LOW);
  digitalWrite(26,LOW);
  digitalWrite(25,HIGH);
}
void randomTurn(){
  turn=true;
  turnMillis=millis();
  if(random(0,10)>5){
    rightTurn();
  }
  else{
    leftTurn();
  }
}
void loop() {
  if(timekeeping){
    if(millis()-lastMillis>=1000){
      lastMillis=millis();
      if(!changeAlarm){
        Serial.printf("%d:%02d:%02d\n",hours,minutes,seconds);
      }
      seconds++;

      if(seconds>=60){
        minutes++;
        if(minutes>=60){
          hours++;
          if(hours>12){
            hours=1;
            realAM=!realAM;
          }
          minutes=0;
        }
        seconds=0;
        if(alarmSet && hours==alarmHours && minutes==alarmMinutes && alarmAM==realAM && seconds==0){
          beep=true;
          straight();
          motorMillis=millis();
        }
        displayTime=hours*100+minutes;
        display.showNumberDecEx(displayTime,0b01000000);

      }
    }
  }
  else if(changeTime){
    if(digitalRead(35)){

      seconds=0;
      Serial.printf("Time changed");
      changeTime=false;
      timekeeping=true;
    }
    if(digitalRead(21)){
      if(millis()-b4Millis>300){
        b4Millis=millis();
        minutes++;
        if(minutes>=60){
          minutes=0;
        }
        Serial.printf("New Time: %d:%02d\n", hours,minutes);
        displayTime=hours*100+minutes;
        display.showNumberDecEx(displayTime,0b01000000);
      }
    }
    if(digitalRead(32)){
      if(millis()-b5Millis>300){
        b5Millis=millis();
        hours++;
        if(hours>12){
          hours=1;
          realAM=!realAM;
        }
        Serial.printf("New Time: %d:%02d\n", hours,minutes);
        displayTime=hours*100+minutes;
        display.showNumberDecEx(displayTime,0b01000000);
      }
    }
  }
  if(changeAlarm){
    if(digitalRead(21)){
      if(millis()-b4Millis>300){
        b4Millis=millis();
        alarmMinutes++;
        if(alarmMinutes>=60){
          alarmMinutes=0;
        }
        Serial.printf("Alarm Time: %d:%02d\n", alarmHours,alarmMinutes);
        displayTime=hours*100+minutes;
        display.showNumberDecEx(alarmHours*100+alarmMinutes,0b01000000);
        Serial.printf("Time to change Minutes: %f ms \n",millis()-b4Millis);
      }
      if(digitalRead(35)){
        Serial.printf("Alarm changed");
        changeTime=false;
        changeAlarm=false;
        timekeeping=true;
        displayTime=hours*100+minutes;
        display.showNumberDecEx(displayTime,0b01000000);
      }
    }
    if(digitalRead(32)){
      if(millis()-b5Millis>300){
        b5Millis=millis();
        alarmHours++;
        if(alarmHours>12){
          alarmHours=1;
          alarmAM=!alarmAM;
        }
        Serial.printf("Alarm Time: %d:%02d\n", alarmHours,alarmMinutes);
        display.showNumberDecEx(alarmHours*100+alarmMinutes,0b01000000);
        Serial.printf("Time to change hours: %f ms \n",millis()-b5Millis);

      }
    }
  }
  if(beep){
    if(!sound){
      sound=true;
      // tone(2,1000);
      digitalWrite(2,HIGH);
    }
    //make motor go
    if(!distWait || millis()-distTime>3000){ //measure distance
      distWait=true;
      measuring=false;
      distTime=millis();
      digitalWrite(18,HIGH);
      delayMicroseconds(10);
      digitalWrite(18,LOW);
    }
    if(!measuring && digitalRead(19)){
      distMillis=micros();
      measuring=true;
    }
    if(turn && (millis()-turnMillis)>1200){
      turn=false;
      straight();
      motorMillis=millis();
    }
    if((millis()-motorMillis>3000 || !digitalRead(19) && measuring)&&!turn){ // or ultrasonic sensor detects something
      testMillis=millis();
      if((0.0344*(micros()-distMillis)/2)<30.0){
        Serial.printf("obstacle detected");
      }
      if((!digitalRead(19) && (0.0344*(micros()-distMillis)/2)<10.0) || millis()-motorMillis>3000)
      {
        motorMillis=millis();
        randomTurn();
      }
      Serial.printf("Time to start turning: %f ms \n",millis()-testMillis);
      distWait=false;
      measuring=false;
    }
    if(digitalRead(35)){
      b1Millis=millis();
      beep=false;
      // noTone(2);
      digitalWrite(2,LOW);
      digitalWrite(26,LOW);
      digitalWrite(25,LOW);
      digitalWrite(33,LOW);
      digitalWrite(27,LOW);
      sound=false;
      Serial.printf("Sound Turn Off: %f ms \n", millis()-b1Millis);
    }

  }

  if(digitalRead(14)){
    if(millis()-b2Millis>300){
      b2Millis=millis();
      alarmSet=!alarmSet;
      digitalWrite(17,alarmSet);
      Serial.printf("Alarm Status Button: %f ms \n", millis()-b2Millis);
    }
  }
  if(digitalRead(34)){
    if(millis()-b3Millis>300){
      b3Millis=millis();
    
      changeAlarm=!changeAlarm;
      if(changeAlarm){
        display.showNumberDecEx(alarmHours*100+alarmMinutes,0b01000000);
      }
      else{
        display.showNumberDecEx(hours*100+minutes,0b01000000);
      }
      changeTime=false;
      timekeeping=true;
      Serial.printf("Change alarm button: %f ms \n",millis()-b3Millis);
    }
  }
  if(digitalRead(21) && digitalRead(32)){
    if(millis()-b4Millis>300 && millis()-b5Millis>300){
      b4Millis=millis();
      b5Millis=millis();
      changeAlarm=false;
      changeTime=!changeTime;
      timekeeping=!timekeeping;
    }
  }
  
  
}
