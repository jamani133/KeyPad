//   4  5  6
// 0  1  2  3

#define SpinnerRate 400 //step size for spinner (smaller number = more agressive reaction, 4096 = full rotation)



#include <EEPROM.h>
#include "FastLED.h"
#include "AS5600.h"
#include <HID-Project.h>

#define LED_COUNT 33
#define LED_PIN   16

const uint16_t deadzone           = 4;
const uint16_t RTDeadzone         = 3;
//                              key0    key1    key2    key3    key4    key5    key6  spinner  base
const int           pins[7] = {     A8,     A9,     A0,     A1,     A6,     A2,     10 };
bool           keyStates[7] = {  false,  false,  false,  false,  false,  false,  false };
bool       prevKeyStates[7] = {  false,  false,  false,  false,  false,  false,  false };
uint16_t     changePoint[6] = {      0,      0,      0,      0,      0,      0 };
uint16_t         topTrim[6] = {      0,      0,      0,      0,      0,      0 };
uint16_t      bottomTrim[6] = {      0,      0,      0,      0,      0,      0 };
char                keys[7] = {    'x',    'c',    'v',    'b',    'd',    'f',    'g'};

AS5600 as5600;   //  use default Wire
int prevAngle = 0;
int angle = 0;
int spinnerSpeed = 0;
CRGBArray<LED_COUNT> leds;
uint8_t rainbow = 0;
bool calibrate = false;
void setup(){
    Serial.begin(9600);
    Wire.begin();
    Consumer.begin();
    as5600.begin();
    loadSettings();
    FastLED.addLeds<NEOPIXEL,LED_PIN>(leds, LED_COUNT);
    pinMode(pins[6],INPUT_PULLUP);
    delay(1000);
}
void loop(){
    EVERY_N_MILLISECONDS( 10 ) {
        handleSpinner();
    }
    EVERY_N_MILLISECONDS( 25 ) {
        updateLeds();
        if(calibrate){
            calib();
        }
    }


    EVERY_N_MILLISECONDS( 1000 ) {
        if(Serial.available()){
            String cmd = Serial.readString();
            if (cmd.equals("save\n")){
                Serial.println("Calibration Saved");
                saveSettings();
                calibrate = false;
            }
            if (cmd.equals("calibrate\n")){
                Serial.println("Calibration Started");
                calibrate = true;
                resetCalibration();
            }
            if (cmd.equals("stop\n")){
                Serial.println("Calibration Stopped");
                calibrate = false;
            }
            if (cmd.equals("load\n")){
                Serial.println("Calibration Loaded");
                calibrate = false;
                loadSettings();
            }
        }
    }

    delayMicroseconds(500);
    readButtons();
    sendEvents();
    for(int i = 0; i<7;i++){
        prevKeyStates[i] = keyStates[i];
    }
    
}


void sendEvents(){
    for(int i = 0; i<7;i++){
        if(keyStates[i] && !prevKeyStates[i]){
            Keyboard.press(keys[i]);
        }
        if(!keyStates[i] && prevKeyStates[i]){
            Keyboard.release(keys[i]);
        }
    }
}

void calib(){
    for(int i = 0; i<6;i++){
        int analogVal = 0;
        for(int j = 0;j<20;j++){
            analogVal += analogRead(pins[i]);
        }
        uint16_t pos = analogVal/20;
        if(pos < bottomTrim[i]){
            bottomTrim[i] = pos;
        }else if(pos > topTrim[i]){
            topTrim[i] = pos;
        }
    }
}


void resetCalibration(){
    for(int i = 0; i<6;i++){
        topTrim[i] = 0;
        bottomTrim[i] = 1023;
    }
}


void readButtons(){
    for(int i = 0; i<6;i++){
        int analogVal = 0;
        for(int j = 0;j<5;j++){
            analogVal += analogRead(pins[i]);
        }
        uint16_t pos = sqrt(map(analogVal/5,bottomTrim[i],topTrim[i],1024,0));
        if(pos < deadzone){
            keyStates[i] = false;
        }else{
            if(changePoint[i] < pos){
                changePoint[i] = pos;
                keyStates[i] = true;
            }
            if(changePoint[i] > pos+RTDeadzone){
                changePoint[i] = pos+RTDeadzone;
                keyStates[i] = false;
            }
        }
        
        //if(i == 2){  
        //    Serial.print(pos);
        //    Serial.print("\t0\t32\n");
        //}
    }
    keyStates[6] = !digitalRead(pins[6]);
}

void loadSettings(){
    for(int i = 0;i<6;i++){
        topTrim[i]     = EEPROM.read(2*i)|EEPROM.read(2*i+1)<<8;
        bottomTrim[i]  = EEPROM.read((2*i)+12)|EEPROM.read(2*i+13)<<8;
    }
}

void saveSettings(){
    for(int i = 0;i<6;i++){
        EEPROM.write(2*i,topTrim[i]&0x00ff);
        EEPROM.write((2*i)+1,(topTrim[i]&0xff00)>>8);
        EEPROM.write((2*i)+12,bottomTrim[i]&0x00ff);
        EEPROM.write((2*i)+13,(bottomTrim[i]&0xff00)>>8);
        Serial.print("Key\t");
        Serial.print(i);
        Serial.print("  : Bottom =\t");
        Serial.print(bottomTrim[i]);
        Serial.print("\tTop =\t");
        Serial.print(topTrim[i]);
        Serial.print("\n");
    }
}

void handleSpinner(){
    angle = as5600.getCumulativePosition();
    spinnerSpeed = as5600.getAngularSpeed(AS5600_MODE_RPM);
    if(abs(spinnerSpeed) < 2){
        as5600.resetCumulativePosition();
        prevAngle = angle;
    }else{
        if (prevAngle+SpinnerRate<angle && spinnerSpeed>0){
            Consumer.write(MEDIA_VOLUME_UP);
            prevAngle = angle;
        }
        if (prevAngle-SpinnerRate>angle && spinnerSpeed<0){
            Consumer.write(MEDIA_VOLUME_DOWN);
            prevAngle = angle;
        }
    }
}

void updateLeds(){
    for(int i = 0; i<7;i++){
        if(keyStates[i]){
            leds[i] = CRGB(127,64,0);
        }else{
            leds[i] = CRGB(0,64,0);
        }
    }
    for(int i = 0; i<4;i++){
        leds[i+7] = CHSV(rainbow+(64*i),255,64);
    }
    for(int i = 0; i<22;i++){
        leds[i+11] = CHSV(rainbow+(11.63*i),255,64);
    }
    if(spinnerSpeed > 5){
        leds[7] = CRGB(100,40,0);
    }
    if(spinnerSpeed < -5){
        leds[9] = CRGB(100,40,0);
    }
    rainbow++;
    FastLED.show();
}