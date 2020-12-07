#include "/home/stefano/Documentos/Projeto Siscom/fault_prob.h"
#include <Arduino.h>
#include <SPI.h>          //Library for using SPI Communication 
#include <mcp2515.h>      //Library for using CAN Communication
#include <Wire.h>


uint8_t fault_prob = 10;
// uint8_t fault_injected;



#define SENDER
uint8_t checksum8(uint8_t a, uint8_t b);
uint8_t checksum(struct can_frame &canMsg);

void ascii_transfer(void);

struct can_frame canMsg;
struct can_frame canMsgStart;
struct can_frame canMsgInjected;
MCP2515 mcp2515(10);


void setup() 
{
  while (!Serial);
  randomSeed(analogRead(A0));
  Serial.begin(9600);
  SPI.begin();               
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();

  // Mensagem de start
  canMsgStart.can_id  = 0x036; 
  canMsgStart.can_dlc = 8;               
  canMsgStart.data[0] = 0x01;
  canMsgStart.data[1] = 0x00;               
  canMsgStart.data[2] = 0x00;            
  canMsgStart.data[3] = 0x00;
  canMsgStart.data[4] = 0x00;
  canMsgStart.data[5] = 0x00;
  canMsgStart.data[6] = 0x00;
  canMsgStart.data[7] = 0x00;
  mcp2515.sendMessage(&canMsgStart);
}

void loop() 
{
  for (int i = 0; i <= 50; i++)
  {
    canMsg.can_id  = 0x037;           
    canMsg.can_dlc = 8;               
    canMsg.data[0] = 0x53;//83
    canMsg.data[1] = 0x00;               
    canMsg.data[2] = 0x00;            
    canMsg.data[3] = 0x4e;
    canMsg.data[4] = 0x00;
    canMsg.data[5] = 0x00;
    canMsg.data[6] = 0x00;
    canMsg.data[7] = checksum(canMsg); // Sending last frame as checksum
    mcp2515.sendMessage(&canMsg);
    Serial.println("Enviando: ");
    for (int i = 0; i < 8; i++)
    {
      Serial.println(canMsg.data[i]);
    }

    Serial.println("############");
    delay(500);
  }
  

  // Mensagem final de quantos foram injetados
    canMsgInjected.can_id  = 0x038;           
    canMsgInjected.can_dlc = 8;               
    canMsgInjected.data[0] = 99;// fault_injected;
    canMsgInjected.data[1] = 0x00;               
    canMsgInjected.data[2] = 0x00;            
    canMsgInjected.data[3] = 0x00;
    canMsgInjected.data[4] = 0x00;
    canMsgInjected.data[5] = 0x00;
    canMsgInjected.data[6] = 0x00;
    mcp2515.sendMessage(&canMsgInjected);
    Serial.println("Enviando mensagem final: ");
    Serial.print(canMsgInjected.data[0]);
    Serial.println(" ");
    delay(4000);
    Serial.println(fault_injected);
  
}

uint8_t checksum(struct can_frame &canMsg){
  uint8_t sum = 0;
  for(int i = 0;i < 7; i++){
    sum = checksum8(sum, canMsg.data[i]); 
  }
  return sum;
}


uint8_t checksum8(uint8_t a, uint8_t b)
{
    unsigned s = a+b;
    while(s>255)
        s = (s&0xff) + (s>>8);
        
    return s;
}

