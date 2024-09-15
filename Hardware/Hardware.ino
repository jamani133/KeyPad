//   4  5  6
// 0  1  2  3

#define SpinnerRate 400 //step size for spinner (smaller number = more agressive reaction, 4096 = full rotation)



#include <EEPROM.h>
#include "FastLED.h"
#include "AS5600.h"
#include <HID-Project.h>

#define LED_COUNT 33
#define LED_PIN    16
Adafruit_NeoPixel leds(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


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


void setup(){
    Serial.begin(9600);
    Wire.begin();
    Consumer.begin();
    as5600.begin();
    loadSettings();
    leds.begin();
    leds.show();
    leds.setBrightness(255);
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
    int spinnerSpeed = as5600.getAngularSpeed(AS5600_MODE_RPM);
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
    
}