//   4  5  6
// 0  1  2  3

#include <Keyboard.h>
#include <EEPROM.h>

const uint8_t RTDeadzone         = 2;
//                              key0    key1    key2    key3    key4    key5    key6  spinner  base
const int          pins[7] = {     A0,     A1,     A2,     A3,     A7,     A8,     10 };
bool          keyStates[7] = {  false,  false,  false,  false,  false,  false,  false };
uint8_t     changePoint[6] = {      0,      0,      0,      0,      0,      0 };
uint8_t         topTrim[6] = {      0,      0,      0,      0,      0,      0 };
uint8_t      bottomTrim[6] = {      0,      0,      0,      0,      0,      0 };
uint8_t     changePoint[6] = {      0,      0,      0,      0,      0,      0 };
uint8_t    RBbrightness[9] = {      0,      0,      0,      0,      0,      0,      0,      0,      0};
uint8_t   actbrightness[9] = {      0,      0,      0,      0,      0,      0,      0,      0,      0};

uint8_t SpinnerRate        = 0;



void setup(){
    Keyboard.begin();
    Serial.begin(9600);
}
void loop(){

}

void readButtons(){
    for(int i = 0; i<6;i++){
        
    }
}

void loadSettings(){
    for(int i = 0;i<6;i++){
        topTrim[i]     = EEPROM.read(i);
        bottomTrim[i]  = EEPROM.read(i+6);
    }
    for(int i = 0;i<9;i++){
        RBbrightness[i] = EEPROM.read(i+12);
        actbrightness[i]= EEPROM.read(i+21);
    }
    enableRT    =  EEPROM.read(30);
    SpinnerRate = EEPROM.read(31);
}

void saveSettings(){
    for(int i = 0;i<6;i++){
        
        EEPROM.write(i,topTrim[i]);
        EEPROM.write(i+6,bottomTrim[i]);
    }
    for(int i = 0;i<9;i++){
        EEPROM.write(i+12,RBbrightness[i]);
        EEPROM.write(i+21,actbrightness[i]);
    }
    EEPROM.write(30,enableRT);
    EEPROM.write(31,SpinnerRate);
}