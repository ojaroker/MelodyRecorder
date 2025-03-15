#include <ILI9341_t3.h>
#include <font_Arial.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "pitches.h"


#define CS_PIN 8
#define TFT_DC 9
#define TFT_CS 10
XPT2046_Touchscreen ts(CS_PIN);
#define TIRQ_PIN 2
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

#define CLK_PIN 14
#define WE_PIN 15 // LOW ACTIVE
#define CE_PIN 16 // LOW ACTIVE
#define D0_PIN 20
#define D1_PIN 19
#define D2_PIN 18
#define D3_PIN 17
#define SPEAKER_PIN 22
#define MSB_PIN 4
#define DIR_PIN 21
#define LED_PIN 3
#define A6_PIN 2


struct NoteMapping {
  int key;
  int note;
};

// Create an array of NoteMapping structs
NoteMapping notes[] = {
  {1, NOTE_C4},  // Key 1 -> C4
  {2, NOTE_D4},  // Key 2 -> D4
  {3, NOTE_E4},  // Key 3 -> E4
  {4, NOTE_F4},  // Key 4 -> F4
  {5, NOTE_G4},  // Key 5 -> G4
  {6, NOTE_A4},  // Key 6 -> A4
  {7, NOTE_B4},  // Key 7 -> B4
  {8, NOTE_C5},  // Key 8 -> C5
  {9, NOTE_D5},  // Key 9 -> D5
  {10, NOTE_E5}, // Key 10 -> E5
  {11, NOTE_F5}, // Key 11 -> F5
  {12, NOTE_G5}, // Key 12 -> G5
  {13, NOTE_A5}, // Key 13 -> A5
  {14, NOTE_B5},  // Key 14 -> B5
  {15, NOTE_C6}  // Key 15 -> C6
};


int mode=0;
/*
 * If mode is 0, have option to RECORD
 * If mode is 1, RECORDING
 * If mode is 2, have option to PLAY
 * If mode is 3, PLAYING
 * If mode is 4, ERASING
 */


void setup() {
  pinMode(MSB_PIN,INPUT);
  pinMode(WE_PIN,OUTPUT);
  pinMode(CE_PIN,OUTPUT);
  pinMode(SPEAKER_PIN,OUTPUT);
  pinMode(CLK_PIN,OUTPUT);
  pinMode(LED_PIN,OUTPUT);
  pinMode(A6_PIN,INPUT);
  // IO pins change mode
  Serial.begin(9600);


  ts.begin();
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(3);
  tft.setCursor(10,3);
  tft.println("Music Boxes");
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);

  // mode button
  tft.drawRect(tft.width()-105,0,103,40,ILI9341_RED);


  resetSquares();

  

}

// update mode button in top right of display
void printButtonMode(){
  tft.setTextSize(2);
  if(mode==0){

    tft.setCursor(tft.width()-80,12);
    tft.setTextColor(ILI9341_RED);
    tft.println("START");
  } else if(mode==1){
    tft.setCursor(tft.width()-70,12);
    tft.setTextColor(ILI9341_RED);
    tft.println("REC");
  } else if(mode==2){
    
    tft.setCursor(tft.width()-70,12);
    tft.setTextColor(ILI9341_GREEN);
    tft.println("PLAY");
  } else if(mode==3){
    
    tft.setCursor(tft.width()-100,12);
    tft.setTextColor(ILI9341_GREEN);
    tft.println("PLAYING");
  } else if(mode==4){
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(90,100);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(4);
    tft.println("ERASING");
  }
}

void eraseOldMode(int mode){
  tft.fillRect(tft.width()-101,4,98,34,ILI9341_BLACK);
  if(mode==0){
    tft.setCursor(tft.width()-100,12);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("PLAYING");
  } else if(mode==1){
    tft.setCursor(tft.width()-80,12);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("START");
  } else if(mode==2){
    tft.setCursor(tft.width()-70,12);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("REC");
  } else if(mode==3){
    tft.setCursor(tft.width()-70,12); 
    tft.setTextColor(ILI9341_BLACK);
    tft.println("PLAY");
    
  } else if(mode==4){
    tft.setCursor(tft.width()-100,12);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("PLAYING");
  }
    
}

boolean wastouched = false;
boolean firstRun=true;
boolean MSB=false;
int prevNote=0;
int numA6=0;
boolean prevA6=false;
long endTime=0;

void loop() {
  // put your main code here, to run repeatedly:
  
  if (mode == 0) {  // PASSIVE
    printButtonMode();
    checkModeButton();
    analogWrite(LED_PIN,200);
    
  } else if (mode == 1) {  // RECORDING
    printButtonMode();
    digitalWrite(CLK_PIN, HIGH);  // Keep the CLK_PIN high to indicate recording is happening
    digitalWrite(DIR_PIN,HIGH);
    boolean currA6 = digitalRead(A6_PIN);
    boolean currMSB = digitalRead(MSB_PIN);
    if(currA6 != prevA6){
      numA6++;
      Serial.println(numA6);
    }
    prevA6=currA6;
    analogWrite(LED_PIN,255-16*numA6);
    
    if(firstRun && !currMSB && MSB){
      eraseOldMode(mode);
      prevA6=false;
      numA6=0;
      mode++;
      
    } else{ 
      MSB=currMSB;
    }

    int notePressed = 0;
    boolean istouched = ts.touched();
    if (istouched) {
      TS_Point p = ts.getPoint();
      if (!wastouched) {
        notePressed = fillSquare(p);  // Draw the filled square and return note #
//        Serial.print("Recognized button ");
//        Serial.println(notePressed);
        prevNote=notePressed;
        
      } else{
        notePressed = prevNote; //allows you to press and hold
      }
      if(notePressed>0){
         int note = notes[notePressed-1].note;
         tone(SPEAKER_PIN,note);
//         Serial.print("Playing note ");
//         Serial.println(note);
       } else{
         noTone(SPEAKER_PIN);
       }
    } else {
      if (wastouched) {
        clearFilledSquare();
        resetSquares();  // Reset the squares to red outline once the touch is released
        noTone(SPEAKER_PIN);
        
      }
    }
    wastouched = istouched;  // Track whether the touchscreen is being touched or not

    // initiate write cycle every 10ms. 
    // this will cause data to be overwritten if the address has not changed (fine)
    if(millis() % 10 == 0){

       writeNoteToRAM(notePressed);
//       Serial.print("Wrote note ");
//       Serial.println(notePressed);

    }

   
  } else if(mode==2){
    printButtonMode();
    noTone(SPEAKER_PIN);
    digitalWrite(CLK_PIN,LOW);
    analogWrite(LED_PIN,0);
    firstRun=true;
    MSB=false;
    checkModeButton();
    
  } else if(mode==3){ // PLAYING
    printButtonMode();
    digitalWrite(CLK_PIN,HIGH);
    digitalWrite(DIR_PIN,LOW);
    boolean currA6 = digitalRead(A6_PIN);
    if(currA6 != prevA6){
      numA6++;
      Serial.println(numA6);
    }
    prevA6=currA6;
    analogWrite(LED_PIN,256-16*numA6);
    if(millis() % 10 == 0){
      readFromRamAndPlay();
    }
    boolean currMSB = digitalRead(MSB_PIN);
    if(firstRun && !currMSB && MSB){
      digitalWrite(CLK_PIN,LOW);
      mode++;
      Serial.println("DONE PLAYING");
      noTone(SPEAKER_PIN);
      firstRun=true;
      endTime=millis();
    } else{
      MSB=currMSB;
    }
    
  } else if(mode==4){
    // ERASING (writing 0s)
    if(firstRun){
      printButtonMode();
      digitalWrite(CLK_PIN, HIGH);
      eraseOldMode(mode);
    }
    firstRun=false;
    
    
    if(millis() % 10 == 0){
      writeNoteToRAM(0);
      long currTime=millis();
      if(currTime-endTime > 15000){
        mode++;
        firstRun=true;
      }
    }
    
    
  } else if(mode==5){
    if(firstRun){
      digitalWrite(CLK_PIN,LOW);
      analogWrite(LED_PIN,0);
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_GREEN);
      tft.setCursor(50,70);
      tft.setTextSize(5);
      tft.print("Finished!");
      tft.setCursor(20,140);
      tft.setTextSize(3);
      tft.print("Press Button on Teensy to Restart");
    }
    firstRun=false;
  }

}

void writeNoteToRAM(int note) {

  pinMode(D0_PIN,OUTPUT);
  pinMode(D1_PIN,OUTPUT);
  pinMode(D2_PIN,OUTPUT);
  pinMode(D3_PIN,OUTPUT);
  
  // Print the note being sent
  
//  Serial.print("Writing note: ");
//  Serial.println(note);


  // Map the note to a 4-bit value (D3, D2, D1, D0)
  digitalWrite(WE_PIN, LOW);  // Set to write mode
  delayMicroseconds(1);
    

  // Set the data pins based on the note's 4-bit value
  digitalWrite(D0_PIN, note & 0x01);   // Set D0 based on the lowest bit of note
  digitalWrite(D1_PIN, (note >> 1) & 0x01); // Set D1
  digitalWrite(D2_PIN, (note >> 2) & 0x01); // Set D2
  digitalWrite(D3_PIN, (note >> 3) & 0x01); // Set D3

  digitalWrite(CE_PIN, LOW);   // Activate the chip 

  delay(1);


  digitalWrite(WE_PIN, HIGH);  // Set to read mode after writing
  delayMicroseconds(1);
  digitalWrite(CE_PIN, HIGH);  // Deactivate the chip

  delayMicroseconds(10);


//  Serial.println("Write complete.");
}


// Function to reset squares back to red outline
void resetSquares() {
  // 1st row: y between 60 and 90, x between 10 and 270
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(2);
  // 1st row
tft.drawRect(10, 60, 35, 30, ILI9341_RED);  // 1st square
tft.setCursor(23, 70);
tft.print(1);

tft.drawRect(75, 60, 35, 30, ILI9341_RED);  // 2nd square
tft.setCursor(88, 70);
tft.print(2);

tft.drawRect(140, 60, 35, 30, ILI9341_RED);  // 3rd square
tft.setCursor(153, 70);
tft.print(3);

tft.drawRect(205, 60, 35, 30, ILI9341_RED);  // 4th square
tft.setCursor(218, 70);
tft.print(4);

tft.drawRect(270, 60, 35, 30, ILI9341_RED);  // 5th square
tft.setCursor(283, 70);
tft.print(5);

// 2nd row
tft.drawRect(10, 130, 35, 30, ILI9341_RED);  // 6th square
tft.setCursor(23, 140);
tft.print(6);

tft.drawRect(75, 130, 35, 30, ILI9341_RED);  // 7th square
tft.setCursor(88, 140);
tft.print(7);

tft.drawRect(140, 130, 35, 30, ILI9341_RED); // 8th square
tft.setCursor(153, 140);
tft.print(8);

tft.drawRect(205, 130, 35, 30, ILI9341_RED); // 9th square
tft.setCursor(218, 140);
tft.print(9);

tft.drawRect(270, 130, 35, 30, ILI9341_RED); // 10th square
tft.setCursor(278, 140);
tft.print(10);

// 3rd row
tft.drawRect(10, 200, 35, 30, ILI9341_RED);  // 11th square
tft.setCursor(18, 210);
tft.print(11);

tft.drawRect(75, 200, 35, 30, ILI9341_RED);  // 12th square
tft.setCursor(83, 210);
tft.print(12);

tft.drawRect(140, 200, 35, 30, ILI9341_RED); // 13th square
tft.setCursor(148, 210);
tft.print(13);

tft.drawRect(205, 200, 35, 30, ILI9341_RED); // 14th square
tft.setCursor(213, 210);
tft.print(14);

tft.drawRect(270, 200, 35, 30, ILI9341_RED); // 15th square
tft.setCursor(278, 210);
tft.print(15);

}

// Function to clear the filled square by drawing over it with the background color
void clearFilledSquare() {
  // 1st row: y between 60 and 90, x between 10 and 270
  tft.fillRect(10, 60, 35, 30, ILI9341_BLACK);   // Clear 1st square
  tft.fillRect(75, 60, 35, 30, ILI9341_BLACK);   // Clear 2nd square
  tft.fillRect(140, 60, 35, 30, ILI9341_BLACK);  // Clear 3rd square
  tft.fillRect(205, 60, 35, 30, ILI9341_BLACK);  // Clear 4th square
  tft.fillRect(270, 60, 35, 30, ILI9341_BLACK);  // Clear 5th square

  // 2nd row: y between 130 and 160, x between 10 and 270
  tft.fillRect(10, 130, 35, 30, ILI9341_BLACK);  // Clear 6th square
  tft.fillRect(75, 130, 35, 30, ILI9341_BLACK);  // Clear 7th square
  tft.fillRect(140, 130, 35, 30, ILI9341_BLACK); // Clear 8th square
  tft.fillRect(205, 130, 35, 30, ILI9341_BLACK); // Clear 9th square
  tft.fillRect(270, 130, 35, 30, ILI9341_BLACK); // Clear 10th square

  // 3rd row: y between 200 and 230, x between 10 and 270
  tft.fillRect(10, 200, 35, 30, ILI9341_BLACK);  // Clear 11th square
  tft.fillRect(75, 200, 35, 30, ILI9341_BLACK);  // Clear 12th square
  tft.fillRect(140, 200, 35, 30, ILI9341_BLACK); // Clear 13th square
  tft.fillRect(205, 200, 35, 30, ILI9341_BLACK); // Clear 14th square
  tft.fillRect(270, 200, 35, 30, ILI9341_BLACK); // Clear 15th square
}

// Function to fill a square when touched
int fillSquare(TS_Point p) {
  int x = p.x;
  int y = p.y;

  int screenWidth = tft.width();
  int screenHeight = tft.height();

  // Map touchscreen raw coordinates to screen coordinates
  // Make sure to adjust this based on the actual calibration values for your touchscreen
  x = ((float)(3800 - x) / 3500) * screenWidth + 12;  // Adjust scaling if necessary
  y = ((float)(3800 - y) / 3500) * screenHeight;
  
//  Serial.print("Coords: ");
//  Serial.print(x);  // Print the x value
//  Serial.print(", ");
//  Serial.println(y);  // Print the y value

  // Check if the touch point is inside any of the predefined rectangles
  // 1st row: y between 60 and 90, x between 10 and 270
  if (y > 60 && y < 90) {
    if (x > 10 && x < 45) {
      tft.fillRect(10, 60, 35, 30, ILI9341_RED);  // 1st square
      Serial.println("Button 1 pressed");
      return 1;
    } else if (x > 75 && x < 110) {
      tft.fillRect(75, 60, 35, 30, ILI9341_RED);  // 2nd square
      Serial.println("Button 2 pressed");
      return 2;
    } else if (x > 140 && x < 175) {
      tft.fillRect(140, 60, 35, 30, ILI9341_RED); // 3rd square
      Serial.println("Button 3 pressed");
      return 3;
    } else if (x > 205 && x < 240) {
      tft.fillRect(205, 60, 35, 30, ILI9341_RED); // 4th square
      Serial.println("Button 4 pressed");
      return 4;
    } else if (x > 270 && x < 305) {
      tft.fillRect(270, 60, 35, 30, ILI9341_RED); // 5th square
      Serial.println("Button 5 pressed");
      return 5;
    }
  }
  // 2nd row: y between 130 and 160, x between 10 and 270
  else if (y > 130 && y < 160) {
    if (x > 10 && x < 45) {
      tft.fillRect(10, 130, 35, 30, ILI9341_RED); // 6th square
      Serial.println("Button 6 pressed");
      return 6;
    } else if (x > 75 && x < 110) {
      tft.fillRect(75, 130, 35, 30, ILI9341_RED); // 7th square
      Serial.println("Button 7 pressed");
      return 7;
    } else if (x > 140 && x < 175) {
      tft.fillRect(140, 130, 35, 30, ILI9341_RED); // 8th square
      Serial.println("Button 8 pressed");
      return 8;
    } else if (x > 205 && x < 240) {
      tft.fillRect(205, 130, 35, 30, ILI9341_RED); // 9th square
      Serial.println("Button 9 pressed");
      return 9;
    } else if (x > 270 && x < 305) {
      tft.fillRect(270, 130, 35, 30, ILI9341_RED); // 10th square
      Serial.println("Button 10 pressed");
      return 10;
    }
  }
  // 3rd row: y between 200 and 230, x between 10 and 270
  else if (y > 200 && y < 230) {
    if (x > 10 && x < 45) {
      tft.fillRect(10, 200, 35, 30, ILI9341_RED); // 11th square
      Serial.println("Button 11 pressed");
      return 11;
    } else if (x > 75 && x < 110) {
      tft.fillRect(75, 200, 35, 30, ILI9341_RED); // 12th square
      Serial.println("Button 12 pressed");
      return 12;
    } else if (x > 140 && x < 175) {
      tft.fillRect(140, 200, 35, 30, ILI9341_RED); // 13th square
      Serial.println("Button 13 pressed");
      return 13;
    } else if (x > 205 && x < 240) {
      tft.fillRect(205, 200, 35, 30, ILI9341_RED); // 14th square
      Serial.println("Button 14 pressed");
      return 14;
    } else if (x > 270 && x < 305) {
      tft.fillRect(270, 200, 35, 30, ILI9341_RED); // 15th square
      Serial.println("Button 15 pressed");
      return 15;
    }
  }

  // If no button was pressed
//  Serial.println("No button pressed");
  return 0;
}


void checkModeButton(){

  boolean istouched = ts.touched();
  if (istouched) {
    TS_Point p = ts.getPoint();

    int x = p.x;
    int y = p.y;
  
    int screenWidth = tft.width();
    int screenHeight = tft.height();


    x = ((float)(3800 - x) / 3500) * screenWidth + 12;
    y = ((float)(3800 - y) / 3500) * screenHeight;

    if(x>screenWidth-106 && y<41){
      eraseOldMode(mode);
      eraseOldMode(mode);
      mode++;
      firstRun=true;
      clearFilledSquare();
      resetSquares();
      
    }

  }
}

int lastNote = 0;
int lengthOfLastNote = 0;  // Time in milliseconds that the current note has been active
unsigned long timeOfLastNote = 0;  // Time when the last note was read (in milliseconds)
int noiseThreshold = 1;  // Threshold to ignore very small changes between notes (to reduce noise)

void readFromRamAndPlay() {

  // Set the chip to read mode
  digitalWrite(WE_PIN, HIGH);  // Set to read mode
  delayMicroseconds(1);
  digitalWrite(CE_PIN, LOW);  // Enable the chip (active low)
  
  delayMicroseconds(1);

  // Read the 4-bit data from the RAM
  int note = 0;
  note |= digitalRead(D0_PIN);  // Read the D0 pin (Least significant bit)
  note |= digitalRead(D1_PIN) << 1;  // Read D1 and shift left by 1
  note |= digitalRead(D2_PIN) << 2;  // Read D2 and shift left by 2
  note |= digitalRead(D3_PIN) << 3;  // Read D3 and shift left by 3

  delay(1);

  // Disable the chip after the read
  digitalWrite(CE_PIN, HIGH);  
  delayMicroseconds(1);
  digitalWrite(WE_PIN, LOW);
  
  

  // Optionally, print the read note for debugging
//  Serial.print("Read note: ");
//  Serial.println(note);


  // If a valid note is read, check if it should be played
  if (note > 0 && note <= 15) {
      lastNote = note;  // Update the last played note
      int noteFrequency = notes[lastNote - 1].note;
      tone(SPEAKER_PIN, noteFrequency, 10);  // Play the note for 10 ms
//      Serial.print("Playing note: ");
//      Serial.println(noteFrequency);

  } else {
    // If no valid note, stop playing the sound
    noTone(SPEAKER_PIN);
    lastNote = 0;  // Reset the last note if no valid note is detected
  }
}
