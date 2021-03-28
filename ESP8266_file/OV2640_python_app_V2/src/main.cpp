#include <Arduino.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

#define CS_Cam 16
#define button 15
#define led_pin 10

uint8_t temp_button = 0;
uint8_t counter = 0;
uint8_t stage = 0;

ArduCAM myCAM( OV2640, CS_Cam );

bool is_header = false;

//Important Arrays
uint8_t VH_[320];
uint8_t VL_[320]; 
//
//Functions
void take_photo();
void arduCam_setup();
void print_values();

//--

void setup() {

  Wire.begin();

  Serial.begin(115200);
  pinMode(CS_Cam, OUTPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(button, INPUT);

  digitalWrite(CS_Cam, 1);

  Serial.println("Here");
  delay(100);
  
  arduCam_setup();

}

void loop() {
  // send data only when you receive data:
  byte inByte = ' ';
  if (Serial.available() > 0) {
    // read the incoming byte:
    inByte = Serial.read();

    if (inByte == 'a') {
      //Serial.println("Start");
      take_photo();
    } else {
      Serial.println("Wrong byte");
    }
    
  }
  delay(100);
}

void wait_for_response(){
  byte inByte = ' ';
  bool check = 1;
  while(check){
    if (Serial.available() > 0) {
      // read the incoming byte:
      inByte = Serial.read();
      if (inByte == 'b'){
        check = 0;
      }
    }
  }
}

void arduCam_setup(){
  uint8_t vid, pid;
  uint8_t temp;
  
  //Serial.println(F("ArduCAM Start!"));

  SPI.begin();

  myCAM.write_reg(0x07, 0x80);  // RESET the CPLD
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);                   //--

  while(1){
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      //Serial.println(F("SPI interface Error!"));
      delay(1000); continue;    
    } else{
      //Serial.println(F("SPI interface OK"));
      break;
    } 	  
  }

  while(1){
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      //Serial.println(F("Can't find OV2640 module!"));
      delay(1000);continue; 
    } else{
      //Serial.println(F("OV2640 detected."));
      break;    
    }
  } 

  //Change to JPEG capture mode and initialize the OV5642 module
  myCAM.set_format(BMP);
  
  myCAM.InitCAM();

  myCAM.clear_fifo_flag();
  
  

  delay(500);
}


void take_photo(){  
  
  digitalWrite(CS_Cam, 1);
  delay(100);
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();

  myCAM.start_capture();

  //Polling the capture done flag
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));

  uint32_t length = 0;
  length = myCAM.read_fifo_length();
  if (length >= MAX_FIFO_SIZE ) 
  {
    //Serial.println(F("ACK CMD Over size. END"));
    myCAM.clear_fifo_flag();
    return;
  }
  if (length == 0 ) //0 kb
  {
    //Serial.println(F("ACK CMD Size is 0. END"));
    myCAM.clear_fifo_flag();
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode

  delay(100);

  uint8_t VH,VL;

  Serial.println('O');
  Serial.println('K');

  myCAM.read_fifo();

  Serial.println("Start");

  for (int i = 0; i < 240; i++){
    for (int j = 0; j < 320; j++){
      VH = myCAM.read_fifo();
      VL = myCAM.read_fifo();
      VH_[j] = VH;
      VL_[j] = VL;
    }

    print_values();

  }

  Serial.println("done");

  myCAM.CS_HIGH();
  myCAM.clear_fifo_flag();

}

void print_values(){
  //for (int count = 0; count < 320; count++){
      
    //VH_[count]; = R4, R3, R2, R1, R0, G5, G4, G3
    //VL_[count]; = G2, G1, G0, B4, B3, B2, B1, B0 

  //}
  int i;

  for (i = 0; i < 80; i++){ //- First 80
    Serial.write((char)VH_[i]);
  }
  for (i = 0; i < 80; i++){
    Serial.write((char)VL_[i]);
  }
  for (i = 80; i < 160; i++){ //- Second 80
    Serial.write((char)VH_[i]);
  }
  for (i = 80; i < 160; i++){
    Serial.write((char)VL_[i]);
  }
  for (i = 160; i < 240; i++){ //- Third 80
    Serial.write((char)VH_[i]);
  }
  for (i = 160; i < 240; i++){
    Serial.write((char)VL_[i]);
  }
  for (i = 240; i < 320; i++){ //- Final 80
    Serial.write((char)VH_[i]);
  }
  for (i = 240; i < 320; i++){
    Serial.write((char)VL_[i]);
  }
  
  wait_for_response();

}


