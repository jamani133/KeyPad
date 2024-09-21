//   4  5  6
// 0  1  2  3

#define SpinnerRate 400 //step size for spinner (smaller number = more agressive reaction, 4096 = full rotation)



#include <EEPROM.h>
#include "FastLED.h"
#include "AS5600.h"
#include <HID-Project.h>
#define LAYOUT_GERMAN

#define LED_COUNT 33
#define LED_PIN   16

const uint16_t deadzone           = 4;
const uint16_t RTDeadzone         = 3;
const uint16_t AnalogDeadzone     = 4;
//                              key0    key1    key2    key3    key4    key5    key6  spinner  base
const int           pins[7] = {     A8,     A9,     A0,     A1,     A6,     A2,     10 };
bool           keyStates[7] = {  false,  false,  false,  false,  false,  false,  false };
bool       prevKeyStates[7] = {  false,  false,  false,  false,  false,  false,  false };
bool      prevKeyStates2[7] = {  false,  false,  false,  false,  false,  false,  false };
uint16_t     changePoint[6] = {      0,      0,      0,      0,      0,      0 };
uint16_t         topTrim[6] = {      0,      0,      0,      0,      0,      0 };
uint16_t      bottomTrim[6] = {      0,      0,      0,      0,      0,      0 };

uint16_t             pos[6] = {      0,      0,      0,      0,      0,      0 };

uint8_t mode = 0;
AS5600 as5600;   //  use default Wire
int prevAngle = 0;
int angle = 0;
int spinnerSpeed = 0;
CRGBArray<LED_COUNT> leds;
uint8_t rainbow = 0;
bool calibrate = false;
bool ledState = false;
void setup(){
    Serial.begin(9600);
    loadSettings();
    Wire.begin();
    if(mode == 1 || mode == 2){
        Gamepad.begin();
    }else{
        Keyboard.begin();
    }
    Consumer.begin();
    as5600.begin();
    FastLED.addLeds<NEOPIXEL,LED_PIN>(leds, LED_COUNT);
    pinMode(pins[6],INPUT_PULLUP);
    delay(1000);
    
}
void loop(){
    EVERY_N_MILLISECONDS( 10 ) {
        handleSpinner();
    }
    EVERY_N_MILLISECONDS( 25 ) {
        if(ledState){
            updateLeds();
        }
        if(calibrate){
            calib();
        }
    }
    EVERY_N_MILLISECONDS( 1000 ) {
        getSerial();
    }

    //delayMicroseconds(500);
    readButtons();
    if(!calibrate){
        sendEvents();
    }
    for(int i = 0; i<7;i++){
        prevKeyStates[i] = keyStates[i];
    }
    
}
void getSerial(){
    if(Serial.available()){
        String cmd = Serial.readString();
        clearLeds();
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
        if (cmd.equals("led_on\n")){
            Serial.println("led on");
            
            ledState = true;
        }
        if (cmd.equals("led_off\n")){
            Serial.println("led off");
            
            ledState = false;
        }
        if (cmd.equals("osu\n")){
            Serial.println("Reboot to switch to osu! Mode");
            mode = 0;
            ledState = true;
        }
        if (cmd.equals("zeep\n")){
            Serial.println("Reboot to switch to Zeepkist Mode");
            mode = 1;
        }
        if (cmd.equals("controller\n")){
            Serial.println("Reboot to switch to controller Mode");
            mode = 2;
        }
        if (cmd.equals("all\n")){
            Serial.println("Reboot to switch to all inputs Mode");
            mode = 3;
        }
        if (cmd.equals("help\n")){
            Serial.print("\n\nPossible commands :\n----------------------------------------------\n");
            Serial.print("          save : save settings to EEPROM\n");
            Serial.print("     calibrate : calibrate analog keys\n");
            Serial.print("          stop : stop calibrating\n");
            Serial.print("          save : stop and save calibration\n");
            Serial.print("          load : load settings from EEPROM\n");
            Serial.print("led_on/led_off : turn leds on / off\n");
            Serial.print("           osu : key config for osu!\n");
            Serial.print("          zeep : key config for Zeepkist\n");
            Serial.print("    controller : analog controller loayout\n");
            Serial.print("           all : all buttons mapped to keys\n\n");
        }
    }
}


void clearLeds(){
    for(int i = 0 ; i<33 ; i++){
        leds[i] = CRGB(0,0,0);
    }
}

void sendEvents(){
    switch(mode){
        case 0:
            sendOSU();
        break;
        case 1:
            sendZEEP();
        break;
        case 2:
            sendCTRL();
        break;
        case 3:
            sendALL();
        break;
    }
}
void sendOSU(){
    char                keys[2] = {    'x',    'c'};
    for(int i = 0; i<2;i++){
        if(keyStates[i] && !prevKeyStates[i]){
            Keyboard.press(keys[i]);
        }
        if(!keyStates[i] && prevKeyStates[i]){
            Keyboard.release(keys[i]);
        }
    }
    if(pos[3] > 25 && !prevKeyStates2[3]){
        Keyboard.press(KEY_ENTER);
        prevKeyStates2[3] = true;
    }else if(pos[3] < 20 && prevKeyStates2[3]){
        Keyboard.release(KEY_ENTER);
        prevKeyStates2[3] = false;
    }
    


    if(pos[5] > 25 && !prevKeyStates2[5]){
        Keyboard.press(KEY_ESC);
        prevKeyStates2[5] = true;
    }else if(pos[5] < 20 && prevKeyStates2[5]){
        Keyboard.release(KEY_ESC);
        prevKeyStates2[5] = false;
    }
    


    if(keyStates[6] && !prevKeyStates[6]){
            Keyboard.press(KEY_F2);
        }
        if(!keyStates[6] && prevKeyStates[6]){
            Keyboard.release(KEY_F2);
        }
    

}
void sendALL(){
    char                keys[7] = {    'a',    's',    'd',    'f',    'w',    'e',    'r'};
    for(int i = 0; i<7;i++){
        if(keyStates[i] && !prevKeyStates[i]){
            Keyboard.press(keys[i]);
        }
        if(!keyStates[i] && prevKeyStates[i]){
            Keyboard.release(keys[i]);
        }
    }
}
void sendZEEP(){
    //char                keys[7] = {    'AL',    'DOWN',    'AR',    '',    'UP',    '',    'RSHIFT'};
    
    if(keyStates[1] && !prevKeyStates[1]){
        Keyboard.press(KEY_DOWN);
    }
    if(!keyStates[1] && prevKeyStates[1]){
        Keyboard.release(KEY_DOWN);
    }

    if(keyStates[4] && !prevKeyStates[4]){
        Keyboard.press(KEY_UP);
    }
    if(!keyStates[4] && prevKeyStates[4]){
        Keyboard.release(KEY_UP);
    }

    if(pos[6] > 25 && !prevKeyStates2[6]){
        Keyboard.press(KEY_RIGHT_SHIFT);
        prevKeyStates2[6] = true;
    }else if(pos[6] < 20 && prevKeyStates2[6]){
        Keyboard.release(KEY_RIGHT_SHIFT);
        prevKeyStates2[6] = false;
    }
    
    Gamepad.xAxis(analogToAxis(pos[0],pos[2],32768));
    
}
void sendCTRL(){
    Gamepad.yAxis(analogToAxis(pos[2],pos[5],32768));
    Gamepad.xAxis(analogToAxis(pos[1],pos[3],32768));
    Gamepad.zAxis(analogToAxis(pos[0],pos[4],127));
}

int analogToAxis(int neg, int pos,int max){
    if(neg <= AnalogDeadzone){
        neg = 0;
    }
    if(pos <= AnalogDeadzone){
        pos = 0;
    }
    return map(pos-neg,-28,28,-max,max);
}

void calib(){
    for(int i = 0; i<6;i++){
        int analogVal = 0;
        for(int j = 0;j<20;j++){
            analogVal += analogRead(pins[i]);
        }
        pos[i] = analogVal/20;
        if(pos[i] < bottomTrim[i]){
            bottomTrim[i] = pos[i];
        }else if(pos[i] > topTrim[i]){
            topTrim[i] = pos[i];
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
        pos[i] = sqrt(map(analogVal/5,bottomTrim[i],topTrim[i],1024,0));
        if(pos[i] < deadzone){
            keyStates[i] = false;
        }else{
            if(changePoint[i] < pos[i]){
                changePoint[i] = pos[i];
                keyStates[i] = true;
            }
            if(changePoint[i] > pos[i]+RTDeadzone){
                changePoint[i] = pos[i]+RTDeadzone;
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
    ledState = EEPROM.read(68);
    mode = EEPROM.read(69);
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
    EEPROM.write(68,ledState);
    EEPROM.write(69,mode);
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
    switch(mode){
        case 0:
            if(keyStates[0]){leds[0] = CRGB(128,64,0);}else{leds[0] = CRGB(0,0,64);}
            if(keyStates[1]){leds[1] = CRGB(128,64,0);}else{leds[1] = CRGB(0,0,64);}

            if(prevKeyStates2[3]){leds[3] = CRGB(128,64,0);}else{leds[3] = CRGB(0,64,0);}
            if(prevKeyStates2[5]){leds[5] = CRGB(128,64,0);}else{leds[5] = CRGB(64,0,0);}
            if(keyStates[6]){leds[6] = CRGB(128,64,0);}else{leds[6] = CRGB(0,64,0);}
        break;

        case 1:
            leds[0] = CRGB(map(pos[0],0,28,0,128),64,0);
            leds[2] = CRGB(map(pos[2],0,28,0,128),64,0);
            if(keyStates[1]){leds[1] = CRGB(128,64,0);}else{leds[1] = CRGB(0,0,64);}
            if(keyStates[4]){leds[4] = CRGB(128,64,0);}else{leds[4] = CRGB(0,0,64);}
            if(keyStates[6]){leds[6] = CRGB(128,64,0);}else{leds[6] = CRGB(64,0,0);}
        break;

        case 2:
            for(int i = 0;i<6;i++){
                leds[i] = CRGB(map(pos[i],0,28,0,128),64,0);
            }
            if(keyStates[6]){leds[6] = CRGB(128,64,0);}else{leds[6] = CRGB(0,64,0);}
        break;

        case 3:
            for(int i = 0;i<7;i++){
                if(keyStates[i]){leds[i] = CRGB(128,64,0);}else{leds[i] = CRGB(0,64,0);}
            }
        break;
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