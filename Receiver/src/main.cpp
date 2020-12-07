#include "/home/stefano/Documentos/Projeto Siscom/fault_prob.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>              //Library for using SPI Communication 
#include <mcp2515.h>          //Library for using CAN Communication
#include <LiquidCrystal.h>    //Library for using LCD display


// uint8_t fault_injected;
uint8_t checksum8(uint8_t a, uint8_t b);
void test_routine_crc(int *fault_counter, int *round_test);
int numdigits(int n);

const int rs = 3, en = 4, d4 = 5, d5 = 6, d6 = 7, d7 = 8;
int err_turn = 0;
int many_errors = 0;
int fault_counter = 0;
int round_test = 0;
bool started = false;  

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  //Define LCD display pins RS,E,D4,D5,D6,D7

struct can_frame canMsgStart;
struct can_frame canMsg;
struct can_frame canMsgEnd; 
MCP2515 mcp2515(10);                 // SPI CS Pin 10 

 
void setup() {
  lcd.begin(16,2);                   //Sets LCD as 16x2 type
  lcd.setCursor(0,0);                //Display Welcome Message
  lcd.print("DETECCAO DE ERROS");
  lcd.setCursor(0,1);
  lcd.print("CRC E CHECKSUM");
  delay(3000);
  lcd.clear();
  SPI.begin();                       //Begins SPI communication
  Serial.begin(9600);                //Begins Serial Communication at 9600 baudrate 
  mcp2515.reset();                          
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz 
  mcp2515.setNormalMode();                  //Sets CAN at normal mode
  canMsgStart.can_id = 0x36;
  canMsg.can_id = 0x37;
  canMsgEnd.can_id = 0x38;
}

void loop() 
{    
    while(!started)
    {
      while(mcp2515.readMessage(&canMsgStart) == MCP2515::ERROR_NOMSG)
      {
        lcd.print("Esperando start");
        delay(100);
        lcd.clear();
      }
      if(canMsgStart.data[0] == 0x01)
      {
        started = true;
        break;
      }
    }
  
  auto status = mcp2515.readMessage(&canMsg);

  if (status == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {
    lcd.setCursor(0,0);   
    uint8_t sum = 0x00;
    
    Serial.println("");
    Serial.println("###############");
    Serial.println("Recebendo: "); 
    for (int i = 0; i < 8; i++)
    {
      Serial.println(canMsg.data[i]); 
    }
    Serial.println("###############");

    //Checking sum
    for(int i=0; i<7; i++){
      sum = checksum8(sum, canMsg.data[i]);
    }

    if(sum == canMsg.data[7]){
      Serial.print("==> Passou na verificacao com checksum = ");
      Serial.print(sum);
      lcd.setCursor(0,0);
      lcd.print(">Checksum OK");
      lcd.setCursor(0,1);
      lcd.print(sum);
    }else{
      Serial.print("==> Reprovou na verificacao com checksum = ");
      Serial.print(sum);
      lcd.setCursor(0,0);
      lcd.print(">Checksum FAILED");
      lcd.setCursor(0,1);
      lcd.print(sum);
      fault_counter+=1;
    }
      
    delay(500);
    lcd.clear();

    if(++round_test >=50)
    {
      test_routine_crc(&fault_counter, &round_test);
      started = false;
    }
    
    }
    
    else if(status == MCP2515::ERROR_NOMSG)
    {
      // vazio
    }
    
    else{
      if(err_turn == 1){
        lcd.setCursor(0,0); 
        lcd.print("Erro de leitura");
        Serial.println("Erro");
      }
      if(err_turn==2){
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("!!!!!!!!!!!!!!!!");
        Serial.println("Erro!");
        err_turn=0;
      }
      err_turn++;
      delay(500);
      lcd.clear();
      many_errors+=1;
      if(many_errors == 10){
        mcp2515.reset();
        many_errors = 0;
      } 
    }
}

uint8_t checksum8(uint8_t a, uint8_t b)
{
    unsigned s = a+b;
    while(s>255)
        s = (s&0xff) + (s>>8);
        
    return s;
}

void test_routine_crc(int *fault_counter, int *round_test){
  unsigned status;
  do
  {
    status = mcp2515.readMessage(&canMsgEnd);
  }while(status != MCP2515::ERROR_OK && canMsgEnd.can_id != 0x38);

  uint8_t f_injected = 0;
  // mcp2515.readMessage(&canMsgEnd);
  for(int i = 0; i < 8; i++)
  Serial.println(canMsgEnd.data[i]);
  f_injected = canMsgEnd.data[0];
  Serial.println("");
  Serial.println("> Rodada de 50 testes terminou!!");
      Serial.print("Erros detectados: ");
      Serial.print(*fault_counter);
      Serial.print("/");
      Serial.println(f_injected);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Fim da rodada de");
      lcd.setCursor(0,1);
      lcd.print("50 testes");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Erros detectados");
      lcd.setCursor(0,1);
      lcd.print(*fault_counter);
      int l = numdigits(*fault_counter);
      lcd.setCursor(l,1);
      // lcd.print("/");

      lcd.setCursor(l+1, 1);
      // lcd.print(f_injected);
      delay(3000);
      *fault_counter = 0;
      *round_test = 0;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("FIM");
      while(1){}
}


int numdigits(int n)
{
    char buf[64];
    return(sprintf(buf,"%d",n));
}