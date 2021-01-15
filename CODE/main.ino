/* 
Triangle of horizontal/vertical and sinelon, alternate
*/



#include <FastLED.h>
#include <Servo.h>
#include <NewPing.h>
#include <Adafruit_PWMServoDriver.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
/*

    SERVOS ---------------------------------------------------------------

*/

// dances
// game of life
// sonar sensors
// reset
// turn off

#define NUM_SERVOS  32

int servoPos[NUM_SERVOS];
int targetPos[NUM_SERVOS];


Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver();
#define SERVOMIN  125 // 125
#define SERVOMAX  575 // 575


const int row1[7] = {12, 15, 17, 31, 21, 30, 100};
const int row2[7] = {11, 14, 0, 18, 16, 24, 28};
const int row3[7] = {9, 13, 10, 19, 22, 27, 100};
const int row4[7] = {5, 6, 8, 7, 20, 25, 26};
const int row5[7] = {4, 1, 2, 3, 23, 29, 100};

const int mat[5][7] = {
  {12, 15, 17, 31, 21, 30, 100},
  {11, 14, 0, 18, 16, 24, 28},
  {9, 13, 10, 19, 22, 27, 100},
  {5, 6, 8, 7, 20, 25, 26},
  {4, 1, 2, 3, 23, 29, 100}
};

const int servoOffsets[NUM_SERVOS] = {9, 9, 0, -3, 0,
                                      6, 9, 3, 6, 0, //9
                                      9, 0, 0, 0, 6, //14
                                      3, 6, 6, 3, 0, //19
                                      0, 6, 9, 0, 0, //24
                                      3, 9, 3, 6, 6, 0
                                     }; //31

//acquired through calbration

const int offsets[NUM_SERVOS*4] = { 36, 86, 116, 149, 
30, 74, 112, 147,
39, 83, 119, 152,
42, 92, 127, 165,
30, 77, 112, 147,
27, 77, 109, 147,
30, 80, 115, 150,
33, 80, 115, 153, 
39, 87, 123, 153,
33, 86, 114, 142,
30, 86, 114, 142, //10
33, 77, 108, 139,
36, 77, 114, 145,
33, 87, 115, 147,
30, 81, 117, 155,
30, 81, 117, 155,
39, 90, 124, 159,
36, 81, 117, 155,
39, 84, 117, 152,
33, 81, 120, 155,
33, 81, 111, 144, //20
36, 81, 114, 152,
33, 84, 117, 141,
33, 81, 111, 147,
36, 84, 120, 155,
36, 77, 114, 152,
36, 87, 117, 155,
33, 87, 117, 147,
36, 96, 127, 159,
27, 77, 114 ,147,
36, 84, 120, 165,
48, 95, 135, 165
};

// padded with 100 to keep array dim consistent

/*

   LEDs ---------------------------------------------------------------

*/

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    7
#define BRIGHTNESS          64 //96
#define FRAMES_PER_SECOND  120

CRGB leds[NUM_LEDS*NUM_SERVOS];
//CRGB leds2[NUM_LEDS];
//CRGB * allLEDs[NUM_SERVOS]; // ASSERT: num led strips = num servos

int gHue = 0;


const int lrow1[7] = {17, 9, 13, 18, 27, 8, 100};
const int lrow2[7] = {5, 11, 22, 23, 15, 16, 30};
const int lrow3[7] = {2, 14, 20, 26, 21, 29, 100};
const int lrow4[7] = {3, 6, 10, 25, 28, 0, 31};
const int lrow5[7] = {1, 4, 12, 7, 19, 24};

const int lmat[5][7] = {
  {17, 9, 13, 18, 27, 8, 100},
  {5, 11, 22, 23, 15, 16, 30},
  {2, 14, 20, 26, 21, 29, 100},
  {3, 6, 10, 25, 28, 0, 31},
  {1, 4, 12, 7, 19, 24}
};


int curHue[NUM_LEDS*NUM_SERVOS];
int tarHue[NUM_LEDS*NUM_SERVOS]; 
bool allstripsoff = false;

/*

   SONAR ---------------------------------------------------------------

*/


#define SONAR_NUM 4      // Number of sensors.
#define MAX_DISTANCE 100 // Maximum distance (in cm) to ping.

NewPing sonar[SONAR_NUM] = {   // Sensor object array.
  NewPing(13, 12, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(11, 10, MAX_DISTANCE),
  NewPing(9, 8, MAX_DISTANCE),
  NewPing(7, 6, MAX_DISTANCE)
};

/*

   MICROPHONE

*/

#define MIC_PIN 5

/*

   CLOCK

*/
ThreeWire myWire(A1, A2, A0); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);



/* MISC */

int state = 4; // starts on sonar
bool select = false; //if on menu and selected, latch

void setup() {

  Serial.begin(9600);
  delay(3000); // recovery for LED strip
  pinMode(MIC_PIN, INPUT);
  setupServos();
  setupLEDs();
  checkState();
  randomSeed(analogRead(8));
  resetServos();
//  setupButtons();
  setupclock();
  
  Serial.println("All setup");
}

void loop() {
//  int num = analogRead(A3);
switch(state){
  case 1:
    dance();
  break;
  case 2:
//    gameOfLife();
  dominoes();
  break;
  case 3:
    clockRoutine();
  break;
  case 4:
    moveToHandSimple();
  break;
}
//  // dance, game of life, clock, sonar
//  if(num < 256){
//    Serial.println("danceee");
////    delay(2000);
//    dance();
//  }
//  else if(num < 512){
//    Serial.println("GoL");
////    delay(2000);
//   gameOfLife(); 
//  }
//  else if(num < 768){
//    Serial.println("clock");
////    delay(2000);
//    clockRoutine();
//  }
//  else{
//    Serial.println("sonar");
////    delay(2000);
//    moveToHandSimple();
//  }
  
//  gameOfLife();
//  moveToHandSimple();
//  setAllStripsOff();
//clockRoutine();
//dance();
//calibrateServo();
}


void setupServos() {
  Serial.println("Setting up servos");
  pwm.begin();
  pwm2.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  pwm2.setPWMFreq(60);
}

void setupLEDs() {
  Serial.println("Setting up LEDs");
  //  FastLED.addLeds<LED_TYPE, 3, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //  FastLED.addLeds<LED_TYPE, 4, COLOR_ORDER>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.clear();
  //  allLEDs[0] = leds;

//  //in an ideal world this would have saved the monstrosity below.
//  for (int i = 0; i < NUM_SERVOS; i++) {
//    allLEDs[i] = new CRGB[NUM_LEDS];
//    //    const int index = 22 + i;
//    //    FastLED.addLeds<LED_TYPE, index, COLOR_ORDER>(allLEDs[i], NUM_LEDS).setCorrection(TypicalLEDStrip);
//  }

  FastLED.addLeds<LED_TYPE, 22, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 23, COLOR_ORDER>(leds, NUM_LEDS, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 24, COLOR_ORDER>(leds, NUM_LEDS * 2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 25, COLOR_ORDER>(leds, NUM_LEDS * 3, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 26, COLOR_ORDER>(leds, NUM_LEDS * 4, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 27, COLOR_ORDER>(leds, NUM_LEDS * 5, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 28, COLOR_ORDER>(leds, NUM_LEDS * 6, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 29, COLOR_ORDER>(leds, NUM_LEDS * 7, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 30, COLOR_ORDER>(leds, NUM_LEDS * 8, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 31, COLOR_ORDER>(leds, NUM_LEDS * 9, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 32, COLOR_ORDER>(leds, NUM_LEDS * 10, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 33, COLOR_ORDER>(leds, NUM_LEDS * 11, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 34, COLOR_ORDER>(leds, NUM_LEDS * 12, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 35, COLOR_ORDER>(leds, NUM_LEDS * 13, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 36, COLOR_ORDER>(leds, NUM_LEDS * 14, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 37, COLOR_ORDER>(leds, NUM_LEDS * 15, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 38, COLOR_ORDER>(leds, NUM_LEDS * 16, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 39, COLOR_ORDER>(leds, NUM_LEDS * 17, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 40, COLOR_ORDER>(leds, NUM_LEDS * 18, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 41, COLOR_ORDER>(leds, NUM_LEDS * 19, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 42, COLOR_ORDER>(leds, NUM_LEDS * 20, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 43, COLOR_ORDER>(leds, NUM_LEDS * 21, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 44, COLOR_ORDER>(leds, NUM_LEDS * 22, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 45, COLOR_ORDER>(leds, NUM_LEDS * 23, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 46, COLOR_ORDER>(leds, NUM_LEDS * 24, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 47, COLOR_ORDER>(leds, NUM_LEDS * 25, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 48, COLOR_ORDER>(leds, NUM_LEDS * 26, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 49, COLOR_ORDER>(leds, NUM_LEDS * 27, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 50, COLOR_ORDER>(leds, NUM_LEDS * 28, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 51, COLOR_ORDER>(leds, NUM_LEDS * 29, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 52, COLOR_ORDER>(leds, NUM_LEDS * 30, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 53, COLOR_ORDER>(leds, NUM_LEDS * 31, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  sinelon();
  refreshLED();
  //  allLEDs[1] = leds2;
}

void refreshServos() {
  for (int i = 0; i < NUM_SERVOS; i++) {
    //    myServos[i].write(servoPos[i]);
  }
  delay(15);
}

void refreshServo(int i) {
  //    myServos[i].write(servoPos[i]);
  delay(15);
}


void refreshLED() {
  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void LEDTravel(CRGB led) {
  //FastLED.clear();
  for (int i = 0; i < NUM_LEDS; i++) {
    led[i] = CRGB::White;
    refreshLED();
    delay(100);
  }
}

//moves servo to a target
void moveTo(int idx, int to) {
  Serial.println("moving");
  int pos = servoPos[idx];
  int coeff = 1;

  if (to < pos) {
    coeff = -1;
  }
  for (int i = pos; coeff * i < coeff * to; i += coeff) {
    servoPos[idx] = i;
    refreshServos();
  }
}

// this is the primary routine for moving the servos, movint them from there current position toward their target postions
// credit to Doug Domke
void moveMultiple(int reps, int mydelay) {
  //  bool break = false;
  for (int count = 0; count < 5 + 5 * reps; count++) {
    for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
      int tar = targetPos[myservo] + servoOffsets[myservo];
      if (servoPos[myservo] > tar) servoPos[myservo]--;
      if (servoPos[myservo] < tar) servoPos[myservo]++;
      if (servoPos[myservo] != tar) {
        if (myservo < 16)  pwm.setPWM(myservo, 0, 3 * servoPos[myservo]);
        if (myservo > 15) pwm2.setPWM(myservo - 16, 0, 3 * servoPos[myservo]);
        //          myServos[myservo].write(servoPos[myservo]);
      }


    }
        gHue +=1;
    
    refreshLED();
    delay(mydelay);
  }
}

void moveMultipleFast() {
  //  bool break = false;
    for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
      int tar = targetPos[myservo] + servoOffsets[myservo];
      servoPos[myservo] = tar;
      if (myservo < 16)  pwm.setPWM(myservo, 0, 3 * servoPos[myservo]);
      if (myservo > 15) pwm2.setPWM(myservo - 16, 0, 3 * servoPos[myservo]);

    }
}

//as I cba rewriting and C very much does not do overloading functions
void moveMultipleWithLED(int reps, int mydelay, int led) { //added extra arg for LED function
   //  bool break = false;
  for (int count = 0; count < 5 + 5 * reps; count++) {
    for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
      
      int tar = targetPos[myservo] + servoOffsets[myservo];
      if (servoPos[myservo] > tar) servoPos[myservo]--;
      if (servoPos[myservo] < tar) servoPos[myservo]++;
      if (servoPos[myservo] != tar) {
        if (myservo < 16)  pwm.setPWM(myservo, 0, 3 * servoPos[myservo]);
        if (myservo > 15) pwm2.setPWM(myservo - 16, 0, 3 * servoPos[myservo]);
        //          myServos[myservo].write(servoPos[myservo]);
      }


    }
    if(led == 1){
        sinelon();
    }
    else if(led ==2){
      moveLED();
    }
        gHue +=1;
    
    refreshLED();
    delay(mydelay);
  }
}

void resetServos() {
  Serial.println("resetting servos");
  for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
    targetPos[myservo] = offsets[myservo*4];
  }
  moveMultiple(40, 15);

}

// 30 is rest

//used to increment all by an amount to find true values
void calibrateServos() {
  Serial.println("calibrating");

  for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
    bool next = false;
    Serial.println();
    Serial.println(myservo);
    Serial.println();
    while (! next){
      Serial.println("enter amount");
      while (!Serial.available()) {}
      String input2 = Serial.readStringUntil('\n');
      if(input2 ==  "next"){
        next = true;
      }
      else{
         
        int num = input2.toInt();
        targetPos[myservo] = num;
        moveMultiple(40, 12); 
      }
    }
  }
  

}

//to test calibration of the mat variable
void matTest() {
  Serial.println(mat[0][1]);
  targetPos[mat[3][4]] = 180;
  moveMultiple(40, 12);
  targetPos[mat[3][4]] = 0;
  moveMultiple(40, 12);

  targetPos[mat[0][1]] = 180;
  moveMultiple(40, 12);
  targetPos[mat[0][1]] = 0;
  moveMultiple(40, 12);
}

//used to see if each servo matches led strip
void lmatTest(){
//  r++;
//  for (int i  = 0; i< NUM_SERVOS; i++) {
//    FastLED.clear();
//    setStripColour(i, r, 0, 100);
//    Serial.println(i);
//    delay(5000);
//  }
  //checks for matches
  for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 5; j++) {
        int index = lmat[j][i];
        targetPos[mat[j][i]] = 110;
        setStripColour(index, 0, 0, 200);
        moveMultiple(40, 100);
        delay(1000);
      }
  }
}

void basicDance() {
  Serial.println("dance");
  rotateRow(row1, 6, 1);
  rotateRow(row2, 7, 3);
  rotateRow(row3, 6, 1);
  rotateRow(row4, 7, 3);
  rotateRow(row5, 6, 1);
  //  delay(1000);
  moveMultipleWithLED(15, 10,1);
  rotateRow(row1, 6, 3);
  rotateRow(row2, 7, 1);
  rotateRow(row3, 6, 3);
  rotateRow(row4, 7, 1);
  rotateRow(row5, 6, 3);
  //  delay(1000);
  moveMultipleWithLED(15, 10,1);
}

void rotateRow(int* row, int count, int to) {
  for (int i = 0; i < count; i++) {
    targetPos[row[i]] = offsets[row[i]*4 + to];
  }
}

void dominoes() {
  int hue = random(0, 255);
  setAllStrips(hue);
  for (int i = 0; i < NUM_SERVOS; i++) {
      targetPos[i] = offsets[i * 4 + 2];
  }
  moveMultiple(5, 12);

  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 7; i++) {
     if (i == 6 && (j == 0 || j == 2 || j == 4)) {
        continue;
      }
      targetPos[mat[j][i]] = offsets[mat[j][i] * 4];
//      gradient();
//      refreshLED();
      hue ++;
      setStripHSV(lmat[j][i], hue);
      moveMultiple(5, 5);
     
    }
  }
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 7; i++) {
      if (i == 6 && (j == 0 || j == 2 || j == 4)) {
        continue;
      }
      targetPos[mat[j][i]] = offsets[1 + mat[j][i] * 4];
      hue ++;
      setStripHSV(lmat[j][i], hue);
      refreshLED();
      moveMultiple(5, 5);
    
    }

  }
   for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 7; i++) {
      if (i == 6 && (j == 0 || j == 2 || j == 4)) {
        continue;
      }
      targetPos[mat[j][i]] = offsets[2 + mat[j][i] * 4];
      hue ++;
      setStripHSV(lmat[j][i], hue);
      refreshLED();
      moveMultiple(5, 5);
    
    }

  } for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 7; i++) {
      if (i == 6 && (j == 0 || j == 2 || j == 4)) {
        continue;
      }
      targetPos[mat[j][i]] = offsets[3 + mat[j][i] * 4];
      hue ++;
      setStripHSV(lmat[j][i], hue);
      refreshLED();
      moveMultiple(5, 5);
      
    }

  }
  
}

void dance2() {
  resetServos();

  setAllStrips(gHue);
  
  targetPos[mat[0][1]] = offsets[mat[0][1]*4+1];
  targetPos[mat[1][2]] = offsets[mat[1][2]*4+1];
  targetPos[mat[2][2]] = offsets[mat[2][2]*4+1];
  targetPos[mat[3][3]] = offsets[mat[3][3]*4+1];
  targetPos[mat[2][3]] = offsets[mat[2][3]*4+1];
  targetPos[mat[1][4]] = offsets[mat[1][4]*4+1];
  targetPos[mat[0][4]] = offsets[mat[0][4]*4+1];
  targetPos[mat[1][1]] = offsets[mat[1][1]*4+1];
  targetPos[mat[2][0]] = offsets[mat[2][0]*4+1];
  targetPos[mat[3][0]] = offsets[mat[3][0]*4+1];
  targetPos[mat[1][5]] = offsets[mat[1][5]*4+1];
  targetPos[mat[2][5]] = offsets[mat[2][5]*4+1];
  targetPos[mat[3][6]] = offsets[mat[3][6]*4+1];

  setStripHSV(lmat[0][1], gHue + 126);
  setStripHSV(lmat[1][2], gHue + 126);
  setStripHSV(lmat[2][2], gHue + 126);
  setStripHSV(lmat[3][3], gHue + 126);
  setStripHSV(lmat[2][3], gHue + 126);
  setStripHSV(lmat[1][4], gHue + 126);
  setStripHSV(lmat[0][4], gHue + 126);
  setStripHSV(lmat[1][1], gHue + 126);
  setStripHSV(lmat[2][0], gHue + 126);
  setStripHSV(lmat[3][0], gHue + 126);
  setStripHSV(lmat[1][5], gHue + 126);
  setStripHSV(lmat[2][5], gHue + 126);
  setStripHSV(lmat[3][6], gHue + 126);
  moveMultiple(50, 12);
  for (int i = 0; i < NUM_SERVOS; i++) {
    targetPos[i] = offsets[i*4+1];
  }
  targetPos[mat[0][1]] = offsets[mat[0][1]*4];
  targetPos[mat[1][2]] = offsets[mat[1][2]*4];
  targetPos[mat[2][2]] = offsets[mat[2][2]*4];
  targetPos[mat[3][3]] = offsets[mat[3][3]*4];
  targetPos[mat[2][3]] = offsets[mat[2][3]*4];
  targetPos[mat[1][4]] = offsets[mat[1][4]*4];
  targetPos[mat[0][4]] = offsets[mat[0][4]*4];
  targetPos[mat[1][1]] = offsets[mat[1][1]*4];
  targetPos[mat[2][0]] = offsets[mat[2][0]*4];
  targetPos[mat[3][0]] = offsets[mat[3][0]*4];
  targetPos[mat[1][5]] = offsets[mat[1][5]*4];
  targetPos[mat[2][5]] = offsets[mat[2][5]*4];
  targetPos[mat[3][6]] = offsets[mat[3][6]*4];
  setAllStrips(gHue + 126);
  setStripHSV(lmat[0][1], gHue);
  setStripHSV(lmat[1][2], gHue);
  setStripHSV(lmat[2][2], gHue);
  setStripHSV(lmat[3][3], gHue);
  setStripHSV(lmat[2][3], gHue);
  setStripHSV(lmat[1][4], gHue);
  setStripHSV(lmat[1][1], gHue);
  setStripHSV(lmat[0][4], gHue);
  setStripHSV(lmat[2][0], gHue);
  setStripHSV(lmat[3][0], gHue);
  setStripHSV(lmat[1][5], gHue);
  setStripHSV(lmat[2][5], gHue);
  setStripHSV(lmat[3][6], gHue);
  moveMultiple(50, 12);

}

void gameOfLife() {
  resetServos();

  //init grid
  int grid[5][7];
  /*
     Any live cell with two or three live neighbours survives.
     Any dead cell with three live neighbours becomes a live cell.
     All other live cells die in the next generation. Similarly, all other dead cells stay dead.


     Slightly adapted for hex coordinates.
  */
  FastLED.clear();
  Serial.println("game of life");
  
  //initialised 
  delay(1000);
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 5; j++) {
      int randNumber = random(0, 2);
      int index = lmat[j][i];
      if (randNumber == 0) {
        setStripColour(index, 255, 0, 0);
        targetPos[mat[j][i]] = offsets[mat[j][i]*4];;
        grid[j][i] = 0; // dead
//        tarHue[index] = 0;
      }
      else {
        setStripColour(index, 0, 255, 0);
        targetPos[mat[j][i]] = offsets[mat[j][i]*4+2];;
        grid[j][i] = 1; // alive
//        tarHue[index] = 96;
      }
      
//      refreshLED();
      delay(100);
    }
  }
//  moveMultipleWithLED(40, 100, -1);
//  Not me hot fixing a stupid bug at led index 0
  if(grid[3][5] == 0){
    setStripColour(0, 255, 0,0);
  }else{
    setStripColour(0, 0, 255, 0);
  }
  moveMultiple(40,100);

  while(countGrid(grid) != 0 &&  countGrid(grid) != NUM_SERVOS){
//    if(analogRead(A3) > 512 || analogRead(A3)< 256){
//            delay(1000);
//            return;
//          }
      for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            int num_alive = 0; //amount of neighbiurs alive
    
           if( i != 6){
              num_alive += grid[j][i+1];
           }
           if ( i != 0){
              num_alive += grid[j][i-1];
           }
           if( j != 4){
              num_alive += grid[j+1][i];
               if( j == 1 || j == 3){
                  if ( i > 0) num_alive += grid[j+1][i -1];
               }
               else{
                if(i < 6) num_alive += grid[j+1][i +1];
               }
              
           }
           if ( j != 0){
              num_alive += grid[j-1][i];
               if( j == 1 || j == 3){
                  if ( i > 0) num_alive += grid[j-1][i -1];
               }
               else{
                if(i < 6) num_alive += grid[j-1][i +1];
               }
           }
    
           if(num_alive > 3 || num_alive < 2){ //overpopulation or underpopulation
            setStripColour(lmat[j][i], 255, 0, 0);
            
            targetPos[mat[j][i]] = offsets[mat[j][i]*4];;
            grid[j][i] = 0; // dead
           }
           else{
            setStripColour(lmat[j][i], 0, 255, 0);
            targetPos[mat[j][i]] = offsets[mat[j][i]*4 + 2];;
            grid[j][i] = 1; // alive
           }
//           Serial.println (i);
//           Serial.println (j);
//           Serial.println(num_alive);
//           Serial.println();
           delay(100);
        }
      }
//      And again
  if(grid[3][5] == 1){
    setStripColour(0, 0, 255,0);
  }else{
    setStripColour(0, 255, 0, 0);
  }
      moveMultiple(40,100);     
//    checkState();
//    if(state != 2){
//      resetServos();
//      Serial.println(analogRead(A3));
//      return;
//    }   
  }
  
}

//helper function to count array
int countGrid(int grid[5][7]){
  int sum = 0;
  for(int i =0; i < 7; i++){
    for(int j = 0; j < 5; j++){
      sum += grid[j][i]; 
    }
  }
  return sum;
}



void dance(){
  Serial.println("dance");
  for(int i = 0; i < 5; i ++){
    flashyLines();
//    checkState();
//    if(state != 1){
//      resetServos();
//      allstripsoff = false;
//      Serial.println(analogRead(A3));
//      return;
//    }
  }
  for(int i = 0; i < 3; i ++){
    basicDance();
//    checkState();
//    if(state != 1){
//      resetServos();
//      allstripsoff = false;
//      Serial.println(analogRead(A3));
//      return;
//    }
  }
  resetServos();
  for(int i = 0; i < 1; i ++){
    dance2();
//    checkState();
//    if(state != 1){
//      resetServos();
//      allstripsoff = false;
//      Serial.println(analogRead(A3));
//      return;
//    }
  }
  resetServos();
  floatingParticle();
//    checkState();
//    if(state != 1){
//      resetServos();
//      allstripsoff = false;
//      Serial.println(analogRead(A3));
//      return;
//    }
  resetServos();
  dominoes();
    checkState();
//    if(state != 1){
//      resetServos();
//      allstripsoff = false;
//      Serial.println(analogRead(A3));
//      return;
//    }
  resetServos();
}

void floatingParticle(){
  resetServos();
  FastLED.clear();
  for(int i =0; i < NUM_SERVOS; i++){
    setStripColour(i, 0,0,0);
  }
  refreshLED();

  Serial.println(lmat[3][0] + 1);
  

  for(int i = 0; i < NUM_LEDS-1; i++){
    
    setStripColour(lmat[3][0], 0,0,0);
    leds[lmat[3][0]*NUM_LEDS+1+i] = CRGB(0,155,155);
    refreshLED();
    delay(200);
  }

  setStripColour(lmat[3][0], 0,0,0);
  delay(1000);
  
  for(int i = 0; i < NUM_LEDS-1; i++){
    setStripColour(lmat[3][1], 0,0,0);
    
    leds[lmat[3][1]*NUM_LEDS+i] = CRGB(0,155,155);
    refreshLED();
    delay(200);
  }
    targetPos[mat[3][1]]= offsets[mat[3][0]*4 + 2];
    moveMultiple(40,12);
    
  delay(200);
  setStripColour(lmat[3][1], 0,0,0);
   
  leds[lmat[2][0]*NUM_LEDS+1+5] = CRGB(0,155,155);
  refreshLED();
  
  targetPos[mat[2][0]]= offsets[mat[2][0]*4 + 1];
  targetPos[mat[1][1]]= offsets[mat[1][1]*4 + 1];
  
  moveMultiple(20,12);

  setStripColour(lmat[2][0], 0,0,0);
   for(int i = 0; i < NUM_LEDS-1; i++){
    setStripColour(lmat[1][1], 0,0,0);
    
    leds[lmat[1][1]*NUM_LEDS+i+1] = CRGB(0,155,155);
    refreshLED();
    delay(200);
  }

  targetPos[mat[1][1]]= offsets[mat[1][1]*4];
  moveMultiple(20,12);
  for(int j =2; j < 6; j++){
     for(int i = 0; i < NUM_LEDS-1; i++){
      setStripColour(lmat[1][j-1], 0,0,0);
      setStripColour(lmat[1][j], 0,0,0);
      leds[lmat[1][j]*NUM_LEDS+i+1] = CRGB(0,155,155);
      refreshLED();
      delay(200);
    }
    setStripColour(lmat[1][j-1], 0,0,0);
    if(j!= 5) setStripColour(lmat[1][j], 0,0,0);
    refreshLED();
    delay(1000);
  }
  
  targetPos[mat[1][5]]= offsets[mat[1][4]*4 + 3];
  targetPos[mat[2][5]]= offsets[mat[2][4]*4 + 3];
  targetPos[mat[3][6]]= offsets[mat[3][4]*4 + 3];
  moveMultiple(20,12);
  
  for(int i = NUM_LEDS-2; i >= 0; i--){
    setStripColour(lmat[1][5], 0,0,0);
    leds[lmat[1][5]*NUM_LEDS+i+1] = CRGB(0,155,155);
    refreshLED();
    delay(200);
  }
  
    setStripColour(lmat[1][5], 0,0,0);

  
  for(int i = NUM_LEDS-2; i >= 0; i--){
    setStripColour(lmat[2][5], 0,0,0);
    leds[lmat[2][5]*NUM_LEDS+i+1] = CRGB(0,155,155);
    refreshLED();
    delay(200);
  }
  
    setStripColour(lmat[2][5], 0,0,0);
  
  for(int i = NUM_LEDS-2; i >= 0; i--){
    setStripColour(lmat[3][6], 0,0,0);
    leds[lmat[3][6]*NUM_LEDS+i+1] = CRGB(0,155,155);
    refreshLED();
    delay(200);
  }
  
  setStripColour(lmat[3][6], 0,0,0);

  resetServos();
  
  
}

void flashyLines(){
  allstripsoff = false;
  setAllStripsOff();
  refreshLED();
  for(int i =0; i < NUM_SERVOS; i++){
    int num = random(0,4);
    targetPos[i] = offsets[i*4 + num*2];
    int col = random(0,255);  
    for(int j = 1; j < NUM_LEDS; j++){
      leds[i*NUM_LEDS+j] = CHSV( col, 255, 192);
  }
  }
  moveMultipleFast();
  delay(500);
  refreshLED();
  delay(2000);
  
}

void clockRoutine(){ 

  RtcDateTime now = Rtc.GetDateTime();
  int minute = now.Minute();
  int hour = now.Hour() % 12;
  int second = now.Second();
  Serial.println(hour);

  targetPos[mat[1][0]] = offsets[mat[1][0]*4 + 2];
  targetPos[mat[3][0]] = offsets[mat[3][0]*4 + 2];

  if(hour > 6){
    setStripHSV(lmat[1][0], 128);
    for(int i = 0; i < hour-6; i++){
      leds[lmat[1][0]*NUM_LEDS+1+i] = CHSV(128,255,192);
    }  
    for(int i = 6; i < hour; i++){
      leds[lmat[3][0]*NUM_LEDS+1+i-6] = CHSV(128,255,192);
    } 
  }
  else{
    for(int i = 0; i < hour; i++){
      leds[lmat[3][0]*NUM_LEDS+1+i] = CHSV(128,255,192);
    }  
  }

  int count = 0;
  for(int j = 0; j < 5; j++){
    for(int i =0; i < 7; i++){
      if(i == 0 && j == 1){
        continue;
      }
      if(i == 0 && j == 3){
        continue;
      }

      if(count-1 < minute /2){
        //if 2 minutes has passed
        targetPos[mat[j][i]] = offsets[mat[j][i]*4 + 1];
          int numleds = second / 20;
          if(minute % 2 != 0){
                numleds +=3;
              }
           for(int w = 0; w < numleds+1; w++){
//              int hueclock;
//              if(minute % 2 == 0){
//                hueclock = 72;
//              }
//              else{
//                hueclock = 0;
//              }
              leds[lmat[j][i]*NUM_LEDS+w+1] = CHSV( 72, 255, 192);
            }
       
      }

//      if(count -1 >= minute/2){
//         targetPos[mat[j][i]] = offsets[mat[j][i]*4 + 1];
//          int numleds = second / 20;
//           for(int w = 0; w < numleds+1; w++){
////              int hueclock;
////              if(minute % 2 == 0){
////                hueclock = 72;
////              }
////              else{
////                hueclock = 0;
////              }
//              leds[lmat[j][i]*NUM_LEDS+w+1] = CHSV( 72, 255, 192);
//            }
//      }
      else{
        targetPos[mat[j][i]] = offsets[mat[j][i]*4];
      }
      
      count++;
    }
  }
  refreshLED();
  moveMultiple(40,12);

//  delay(4000);
//    checkState();
//  if(state != 3){
//    resetServos();
//    allstripsoff = false;
//    Serial.println(analogRead(A3));
//    return;
//  }

}


void fullRotate() {
  Serial.println("moving multiple");
  for (int i = 0; i < NUM_SERVOS; i++) {
    targetPos[i] = 0;
  }

  moveMultiple(40, 12); //80 40
  for (int i = 0; i < NUM_SERVOS; i++) {
    targetPos[i] = 180;
  }
  moveMultiple(40, 12);

}

void checkSonar() {
  for (int i = SONAR_NUM - 1; i > -1; i--) { // Backwards through loop as reflective of arrangement irl
    delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    if (sonar[i].ping_cm() != 0 ) {
      Serial.print(i);
      Serial.print("=");
      Serial.print(sonar[i].ping_cm());
      Serial.print("cm ");
    }
  }
  Serial.println();

}

/***

   WARNING: very unreliable

*/

//returns coords
float estimateSonarCoords() {
  const int dist = 17; //servos about 17cm from each other
  float distances[4];
  int num_measurements = 0;
  int num_comparisons = 0;
  double xs[6];
  double ys[6];

  for (int i = SONAR_NUM - 1; i > -1; i--) { // Backwards through loop
    delay(50);
    distances[i] = sonar[i].ping_cm();
    if (distances[i] > 1. ) num_measurements++;
    Serial.print(i);
    Serial.print("=");
    Serial.print(distances[i]);
    Serial.print("cm ");
  }
  Serial.println();
  for (int i = 0; i < SONAR_NUM; i++) { //measurements between each
    for (int j = i + 1; j < SONAR_NUM; j++) { //measurements between each

      float d1 = distances[i];
      float d2 = distances[j];

      if (d1 == 0. || d2 == 0.) continue; // skip if one value is 0

      float distt = dist * (j - i);
      float x = (distt * distt + (d2 * d2) - (d1 * d1)) / (2 * distt);
      float y = sqrt((d1 * d1) - (x * x));
      xs[num_comparisons] = x;
      ys[num_comparisons] = y;
      num_comparisons ++;

    }
  }

  if (num_comparisons == 0) {
    Serial.println("No comparisons!");
    delay(1000);
    return -1.; // no sensor data from two sensors
  }

  float x_avg = 0;
  float y_avg = 0;
  for (int i = 0; i < num_comparisons; i ++) {
    x_avg += xs[i];
    y_avg += ys[i];
  }

  x_avg /= num_comparisons;
  y_avg /= num_comparisons;

  Serial.println(x_avg);
  Serial.println(y_avg);
  Serial.println();
  delay (2000);
  return x_avg;

}

//simply moves in direction when hand is over certain sensors
//TODO light leds as much as strength.


int curdir = -1; //current direction
void moveToHandSimple() {
  FastLED.clear();
  Serial.println("sonar");
  float distances[4];
  int dir = -1; //could be between 0 and 7 0 is left and 7 is right
  int maxdistance = 0; // for led
  int mindistance = 10000;
  for (int i = 4; i > -1; i--) { // Backwards through loop
    delay(50);
    distances[i] = sonar[i].ping_cm();
    Serial.print(i);
    Serial.print("=");
    Serial.print(distances[i]);
    Serial.print("cm ");
    if(distances[i] > maxdistance) maxdistance = distances[i];
    if(distances[i] < mindistance && distances[i]> 1) mindistance = distances[i];
  }

  //hacky way instead of checking cables. WIll be fixed when I do massive code cleanup. 
  if(dir == 1) dir = 0;
  else if(dir == 0) dir = 1; 
  
    Serial.print(" max: ");
    Serial.print(maxdistance);
    Serial.print("cm ");
    Serial.print(" min: ");
    Serial.print(mindistance);
    Serial.print("cm ");
//
  int numactive = 0;
  for (int i = 0; i < 4; i++) {
    if (distances[i] > 0) {
      numactive++;
    }
//    if(distances[i] == mindistance ) dir = i;
  }

  if(numactive > 2) dir = 2;

  else if(distances[0] > 0) dir = 3;
  else if(distances[3] > 0) dir = 1;
  
  Serial.println(dir);


  if (dir > -1) {
    for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
      targetPos[myservo] = offsets[myservo*4 + dir];
      for(int i = 1; i < NUM_LEDS; i++){
         leds[myservo*NUM_LEDS+i] = CHSV(maxdistance*5, 255, 192);
      }
    }
    if(curdir != dir){
      curdir = dir;
      moveMultipleFast();
    }
    refreshLED();
  }

//  checkState();
//  if(state != 4){
//    delay(1000);
//    resetServos();
//    return;
//  }

}



void checkMic() {

  int sensorValue = digitalRead(MIC_PIN);
  Serial.println (sensorValue, DEC);
}

// old func to check if i had enough power
void checkPowerLights() {

  refreshLED();
  fill_rainbow( leds, NUM_LEDS, 0, 7);
}

void setupclock() {
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
}

void checkTime() {
  RtcDateTime now = Rtc.GetDateTime();
  int minute = now.Minute();
  int hour = now.Hour();
  int second = now.Second();
  
}

//TODO: DELETE
void reset11() {
  targetPos[10] = 30;
  moveMultiple(40, 12);
  targetPos[10] = 180;
  moveMultiple(40, 12);
}
void gradient()
{
  
  fill_solid( leds, NUM_LEDS*NUM_SERVOS, CRGB::Black);
  for (int j = 0; j < NUM_SERVOS; j++) {
      for(int i = 0; i < NUM_LEDS; i++){
        leds[i + j *NUM_SERVOS] += CHSV( j*5+gHue, 255, 192);
        
      }
  }
}
void sinelon()
{
  
  fill_solid( leds, NUM_LEDS*NUM_SERVOS, CRGB::Black);
  for (int j = 0; j < NUM_SERVOS; j++) {
        int pos = beatsin16( 13, 0, NUM_LEDS-2 ); // -2 vs -1 as first led is covered
//        allLEDs[j][pos+1] += CHSV( gHue, 255, 192); // same with pos +1 vs pos
        leds[pos+1 + j *NUM_LEDS] += CHSV( gHue, 255, 192);
  }
}
int poss = 0;
//moves LED across strip
void moveLED() {
  fill_solid( leds, NUM_LEDS*NUM_SERVOS, CRGB::Black);
  for (int j = 0; j < NUM_SERVOS; j++) {
    if (poss == NUM_LEDS) poss = 1;
    leds[poss+1 + j *NUM_LEDS] += CHSV( gHue, 255, 192);
    EVERY_N_MILLISECONDS(500) {
      poss ++;
    }

  }
}


//used to work out which LED is which
void calibrateLED() {
  //   for(int j =0; j < 5; j++){
  //    allLEDs[j][poss] =  CRGB(j,0,0)
  //  }
  //   for(int j =5; j < 10; j++){
  //    allLEDs[j][poss] =  CRGB(0,j,0)
  //  }
  //   for(int j =10; j < 15; j++){
  //    allLEDs[j][poss] =  CRGB(0,0,j)
  //  }
  int num;
  for (int j = 0; j < NUM_SERVOS; j++) {
//    fill_solid( allLEDs[j], NUM_LEDS, CRGB::Black);
  }
  refreshLED();
  while (!Serial.available()) {}
  String input = Serial.readStringUntil('\n');
  num = input.toInt();
  for (int i = 1; i < NUM_LEDS; i++) {
//    allLEDs[num][i] = CRGB::Green;
      leds[i + num *NUM_LEDS] = CRGB::Green;
  }
  refreshLED();
  delay(1000);
}

// finds out where each servo is + its max and min rotation
void calibrateServo() {
  int num;
  Serial.println("enter index");
  while (!Serial.available()) {}
  String input = Serial.readStringUntil('\n');

  Serial.println("enter amount");
  while (!Serial.available()) {}
  String input2 = Serial.readStringUntil('\n');
  num = input2.toInt();
  int index = input.toInt();
  targetPos[index] = num;
  moveMultiple(40, 12);
  targetPos[index] = num;
  moveMultiple(40, 12);
}

//checks offsets calibrated correctly (I do love a calibration)
void checkOffset(){
  FastLED.clear();
  sinelon();
//  refreshLED();
  Serial.println("checking offset");
  int num = analogRead(A3); //max 1023
//  for(int i = 0; i < NUM_SERVOS; i++){
//    Serial.println();
//    Serial.println(i);
////    Serial.println(getRotation(i, 0));
//    Serial.println(offsets[i*4]);
//    Serial.println();
//  }
  
  if(num < 256){
     for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
      targetPos[myservo] = offsets[myservo*4];
      }
      moveMultipleWithLED(40, 15,1);
  }
  else if(num < 512){
    for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
      targetPos[myservo] = offsets[myservo*4 + 1];
      }
      moveMultiple(40, 15);
      moveMultipleWithLED(40, 15,1);
  }
  else if(num < 768){
    for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
      targetPos[myservo] = offsets[myservo*4+2];
      }
      moveMultiple(40, 15);
      moveMultipleWithLED(40, 15,1);
  }
  else{
    for (int myservo  = 0; myservo < NUM_SERVOS; myservo++) {
      targetPos[myservo] = offsets[myservo*4+3];
      }
      moveMultiple(40, 15);
      moveMultipleWithLED(40, 15,1);
  }
  
}


void setStripColour(int idx, int r, int g, int b){
  for(int i = 1; i < NUM_LEDS; i++){
    if(idx == 6 && i == NUM_LEDS-1){
       //edge case 
      leds[idx*NUM_LEDS] = CRGB( r, g, b);
    }
    else{
      leds[idx*NUM_LEDS+i] = CRGB( r, g, b);
    }
  }
  refreshLED();
//    delay(5);
}
void setStripHSV(int idx, int hue){
  for(int i = 1; i < NUM_LEDS; i++){
    if(idx == 6 && i == NUM_LEDS-1){
       //edge case 
      leds[idx*NUM_LEDS] = CHSV( hue, 255, 192);
    }
    else{
      leds[idx*NUM_LEDS+i] = CHSV( hue, 255, 192);
    }
  }
  refreshLED();
    delay(15);
}

void setAllStrips(int hue){
  for(int idx =0; idx < NUM_SERVOS; idx++){
     for(int i = 1; i < NUM_LEDS; i++){
      if(idx == 6 && i == NUM_LEDS-1){
         //edge case 
        leds[idx*NUM_LEDS] = CHSV( hue, 255, 192);
      }
      else{
        leds[idx*NUM_LEDS+i] = CHSV( hue, 255, 192);
      }
    }
  }
}
void setAllStripsOff(){
  if(!allstripsoff){
    allstripsoff = true;
    for(int idx =0; idx < NUM_SERVOS; idx++){
     for(int i = 1; i < NUM_LEDS; i++){
      if(idx == 6 && i == NUM_LEDS-1){
         //edge case 
        leds[idx*NUM_LEDS] = CRGB( 0,0,0);
      }
      else{
        leds[idx*NUM_LEDS+i] = CRGB( 0,0,0);
      }
    }
  }
}
}

void checkState(){
  int num = analogRead(A3);

//  // dance, game of life, clock, sonar
  if(num < 256){
    Serial.println("changed to danceee");
    state = 1;
  }
  else if(num < 512){
    Serial.println("chnaged to GoL");
    state = 2;
  }
  else if(num < 768){
    Serial.println("changed to clock");
    state = 3;
  }
  else{
    Serial.println("changed to sonar");
    state = 4;
  }
}
