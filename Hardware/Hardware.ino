//   4  5  6
// 0  1  2  3

#define SpinnerRate 400 //step size for spinner (smaller number = more agressive reaction, 4096 = full rotation)



#include <EEPROM.h>
#include "FastLED.h"
#include "AS5600.h"
#include <HID-Project.h>

#define LED_COUNT 33
#define LED_PIN    16


const uint8_t RTDeadzone         = 2;
//                              key0    key1    key2    key3    key4    key5    key6  spinner  base
const int           pins[7] = {     A0,     A1,     A2,     A3,     A7,     A8,     10 };
bool           keyStates[7] = {  false,  false,  false,  false,  false,  false,  false };
uint16_t     changePoint[6] = {      0,      0,      0,      0,      0,      0 };
uint16_t         topTrim[6] = {      0,      0,      0,      0,      0,      0 };
uint16_t      bottomTrim[6] = {      0,      0,      0,      0,      0,      0 };


AS5600 as5600;   //  use default Wire
int prevAngle = 0;
int angle = 0;
int spinnerSpeed = 0;
CRGBArray<LED_COUNT> leds;

void setup(){
    Serial.begin(9600);
    Wire.begin();
    Consumer.begin();
    as5600.begin();
    loadSettings();
    FastLED.addLeds<NEOPIXEL,LED_PIN>(leds, LED_COUNT);
    delay(1000);
}
void loop(){
    EVERY_N_MILLISECONDS( 10 ) {
        handleSpinner();
        updateLeds();
    }
}

void readButtons(){
    for(int i = 0; i<6;i++){
        uint16_t pos = map(analogRead(pins[i],bottomTrim,topTrim,0,1023));
        if(keyStates[i]){

        }else{

        }
        if(pos < deadzone){
            keyStates[i] = false;
        }
        if(pos > 1023-deadzone){
            keyStates[i] = true;
        }
    }
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
            leds[i] = CRGB(255,127,0);
        }else{
            leds[i] = CRGB(0,127,0);
        }
        if(int i = 0; i<4;i++){
            leds[i+7] = CHSV(rainbow+(64*i),255,64);
        }
        if(int i = 0; i<22;i++){
            leds[i+11] = CHSV(rainbow+(11.63*i),255,64);
        }
        if(spinnerSpeed < 2){
            leds[7] = CRGB(255,127,0);
        }
        if(spinnerSpeed > 2){
            leds[9] = CRGB(255,127,0);
        }
        rainbow=(rainbow+1)%255;
    }
}