#include <MotionQueue.h>

#include "SDPArduino.h"
#include <Wire.h>


#define DEBUG 1

//Globals
bool ON = false;

//Moving
#define MOTOR_N 3
bool queueChanged = false;
byte motorMapping[MOTOR_N] = {1, 0, 2};
int motorMultiplier[MOTOR_N] = {1, 1, 1};

//Kicking
#define KICK_DELAY_MOVING_UP 250
#define KICK_DELAY_UP 200
#define KICK_DELAY_MOVING_DOWN 220

#define KICK_STATE_IDLE 0
#define KICK_STATE_START 1
#define KICK_STATE_MOVING_UP 2
#define KICK_STATE_UP 3
#define KICK_STATE_MOVING_DOWN 4

#define KICK_MOTOR 4
#define KICK_MOTOR_DIR -1

long kickStartTime;
int kickState = KICK_STATE_IDLE;
int kickPower = 0;

//Catch
#define CATCH_MOTOR 3

#define CATCH_STATE_IDLE 0
#define CATCH_STATE_DISENGAGE 1
#define CATCH_STATE_ENGAGE 2
#define CATCH_STATE_OPERATING_ENGAGE 3
#define CATCH_STATE_OPERATING_DISENGAGE 4
//#define CATCH_STATE_ENGAGED 5
//#define CATCH_STATE_DISENGAGE 6

#define CATCH_DISENGAGE_DIR -1
#define CATCH_DISENGAGE_POWER 1
#define CATCH_DISENGAGE_DELAY 200

#define CATCH_ENGAGE_DIR 1
#define CATCH_ENGAGE_POWER 1
#define CATCH_ENGAGE_DELAY 200

long catchStartTime;
int catchState = CATCH_STATE_IDLE;

// Construct a queue to store motions
MotionQueue queue;


void setup() {
  
  SDPsetup(); 
  motorAllStop();
  debugPrint("started");// transmit started packet
}


//3 kick 2byte
//4 motor 4byte
//5 read sensor 2byte
void loop() {
  
  //Coms
  readComms();
  if(queue.update() == 1)
  {
    Serial.write('O');
    queueChanged = true;
  }
 
 
  //Control 
  if(ON)
  {
    doMotors();
    doKick();
    doCatcher();
  }
  else
  {    
    motorAllStop();
  } 
}


void moveMotor(int motor, int power){
  /*
  debugPrint("\n***MM ");
  debugPrint(motor);
  debugPrint(", ");
  debugPrint(power);
  debugPrint(" **\n");
  */
  
  
  if(power >= 0){
    motorForward(motor, power);
  }
  else if(power < 0){
    motorBackward(motor, -power);
  }
  else {
    motorStop(motor);
  }
}

void doCatcher(){
  if(catchState == CATCH_STATE_ENGAGE){
    catchStartTime = millis();

    catchState = CATCH_STATE_OPERATING_ENGAGE;

    moveMotor(CATCH_MOTOR, CATCH_ENGAGE_POWER * CATCH_ENGAGE_DIR);
  }
  else if(catchState == CATCH_STATE_OPERATING_ENGAGE){
     if(millis() - catchStartTime >= CATCH_ENGAGE_DELAY){
       catchStartTime = millis();

       catchState = CATCH_STATE_IDLE;

       motorStop(CATCH_MOTOR);
     }
  }
  else if(catchState == CATCH_STATE_DISENGAGE){
    catchStartTime = millis();

    catchState = CATCH_STATE_OPERATING_DISENGAGE;

    moveMotor(CATCH_MOTOR, CATCH_DISENGAGE_POWER * CATCH_DISENGAGE_DIR);
  }
  else if(catchState == CATCH_STATE_OPERATING_DISENGAGE){
      if(millis() - catchStartTime >= CATCH_DISENGAGE_DELAY){
       catchStartTime = millis();

       catchState = CATCH_STATE_IDLE;

       motorStop(CATCH_MOTOR);
     }
  }
}

void doKick(){  
  if(kickState == KICK_STATE_START){
    kickStartTime = millis();
    
    kickState = KICK_STATE_MOVING_UP;
    
    moveMotor(KICK_MOTOR, kickPower * KICK_MOTOR_DIR);
  }
  else if(kickState == KICK_STATE_MOVING_UP){
    if(millis() - kickStartTime > KICK_DELAY_MOVING_UP){
      kickStartTime = millis();
      
      kickState = KICK_STATE_UP;
    
      motorStop(KICK_MOTOR); 
    }
  }
  else if(kickState == KICK_STATE_UP){
    if(millis() - kickStartTime > KICK_DELAY_UP){
      kickStartTime = millis();
      
      kickState = KICK_STATE_MOVING_DOWN;
      
      moveMotor(KICK_MOTOR, -kickPower * KICK_MOTOR_DIR);
    }
  } 
  else if(kickState == KICK_STATE_MOVING_DOWN)
    if(millis() - kickStartTime > KICK_DELAY_MOVING_DOWN){
      kickState = KICK_STATE_IDLE;
      
      motorStop(KICK_MOTOR); 
  } 
}

void doMotors(){
  if(queueChanged){
    queueChanged = false;
    
    int i = 0;
    if(queue.count() > 0)
    {
      queued_motion_t* current = queue.current();
      for( ; i < MOTOR_N; i++)
      {
         if(motorMapping[i] > -1){
           debugPrint(current->power[i]);
           moveMotor(motorMapping[i], current->power[i] * current->direction[i] * motorMultiplier[i]);
         }
      }
    } else {
      for( ; i < MOTOR_N; i++)
      {
         if(motorMapping[i] > -1){
           debugPrint("disabled");
           moveMotor(motorMapping[i], 0);
         }
      }
    }
  } 
}


void readComms(){
  if (Serial.available()) // have we got enough characters for a message?
  {
    byte incoming = Serial.read();
    if (incoming == 'D') //Deactivate
    {
      Serial.print('C');
      debugPrint("deactivated");
      ON = false;
    }
    else if (incoming == 'A') // Activate
    {
      Serial.print('C');
      debugPrint("activated");
      ON = true;
      
      queueChanged = false;
      kickState = KICK_STATE_IDLE;
      
    }
    else if (incoming == 'K') // Kick
    {
      Serial.print('C');
      debugPrint("kick");
      
      int nextByte = waitForByte();   
      
      if(kickState == KICK_STATE_IDLE){
        kickState = KICK_STATE_START;
        kickPower = nextByte;
      }      
      
      debugPrint(" ");
      debugPrint(kickPower);      
    }
    else if (incoming == 'R') // Sensor read
    {
      debugPrint("read");
      int nextByte = waitForByte(); 
      //Read sensor
      //Reply
      debugPrint(" ");
      debugPrint(nextByte);
      
      
      Serial.print('S');
      Serial.print("1234");
    }
    else if (incoming == 'Q') // Enqueue
    {
      byte m[] = { 
        getPower(), getDirection(),
        getPower(), getDirection(),
        getPower(), getDirection()
      };
      
      unsigned short ms = getMillis();
      
      // Enqueue
      int result = queue.enqueue(
         m[0],
         m[1],
         m[2],
         m[3],
         m[4],
         m[5],
         ms);
  
      if(result == 0)
      {
        Serial.print('C');
        //debugPrint("QUEUED");
        debugPrint(ms);
        queueChanged = true;
      }
    }
    else if (incoming == 'L') // Clear queue
    {
      // Clear queue
      queue.clear();
      Serial.print('C');
      debugPrint("clear");
    }
    else if (incoming == 'P') // Pop from queue
    {
      // Queue pop
      queue.pop();
      Serial.print('C');
      debugPrint("pop");
    }
    else if (incoming == 'N') // Engage catcher
    {
      Serial.print('G');
      debugPrint("catch");

      if(catchState == CATCH_STATE_IDLE){
        catchState = CATCH_STATE_ENGAGE;
      }
    }
    else if (incoming == 'I') // Disengage catcher
    {
      Serial.print('G');
      debugPrint("uncatch");

      if(catchState == CATCH_STATE_IDLE){
        catchState = CATCH_STATE_DISENGAGE;
      }
    }
    else{
      Serial.print('E');
      
      debugPrint("get off my lawn");      
    }
  }   
}

byte getPower()
{
  int nextByte = waitForByte();
  return nextByte;
}

byte getDirection()
{
  int nextByte = waitForByte();
  if(nextByte == 0)
    return 1;
  else
    return -1;
}

unsigned short getMillis()
{
  byte top = waitForByte();
  byte bottom = waitForByte();

  return (top << 8) | bottom;
}

byte waitForByte()
{
  while(!Serial.available());
  return Serial.read();
}


void debugPrint(char msg[]){
  if(DEBUG){
    Serial.print(msg);
  }
}

void debugPrint(int msg){
  if(DEBUG){
    Serial.print(msg, DEC);
  }
}

void debugPrint(unsigned short msg){
  if(DEBUG){
    Serial.print(msg, HEX);
  }
}
