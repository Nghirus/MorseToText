/**********************
 * 
 * Program: Morse to Text
 * Authors: Vinh-Nghi Nguyen, Andrew Trinh, Tristan Vuong
 * 
 * About Us: We developed this project keeping in mind 
 * that it should be easily reproduceable. As members 
 * of BSA Troop 680, we felt that this would be a fun 
 * project for leaders and merit badge counselors to 
 * introduce their members to programming with an Arduino.
 * 
 * Description: This program is a simple morse to 
 * text converter for the arduino that uses button 
 * input and light input.
 * 
 * ------List of Parts------
 * Arduino Uno
 * Base Shield v2.0 by Seeed Studio
 * GROVE Button v1.1
 * GROVE Buzzer v1.2
 * GROVE Light sensor v1.1
 * GROVE LCD RGB Backlight
 * 
 *********************/
#include "rgb_lcd.h"

const int BUZZER = 4;                                       // Digital Output
const int BUTTON = 8;                                       // Digital Input
const int LIGHTSENSOR = A0;                                 // Analog Output

const int NUMOFCHAR = 36;                                   // Size of character array
const int DELETE = 10;                                      // Beep length for clearing output
const int LONGBEEP = 2;                                     // Beep length for "-"
const int SHORTBEEP = 1;                                    // Beep length for "."
const int LIGHT_THRESH = 750;                               // Sensor threshold for the light sensor (tune to fit application)
const int WAIT = 5;                                         // Idle time before translating character

int beepLength = 0;
int offLength = 0;
int col = 0;
int row = 0;
int light_sensor = 0;
rgb_lcd lcdDisplay;

const char MORSE[NUMOFCHAR] {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','1','2','3','4','5','6','7','8','9','0'};
const String TRANSLATE[NUMOFCHAR] {
  "01",   // A                                              // This encoding uses 0 for '.', and 1 for '-'
  "1000", // B
  "1010", // C
  "100",  // D
  "0",    // E
  "0010", // F
  "110",  // G
  "0000", // H
  "00",   // I
  "0111", // J
  "101",  // K
  "0100", // L
  "11",   // M
  "10",   // N
  "111",  // O
  "0110", // P
  "1101", // Q
  "010",  // R
  "000",  // S
  "1",    // T
  "001",  // U
  "0001", // V
  "011",  // W
  "1001", // X
  "1011", // Y 
  "1100", // Z
  "01111",// 1
  "00111",// 2
  "00011",// 3
  "00001",// 4
  "00000",// 5
  "10000",// 6
  "11000",// 7
  "11100",// 8
  "11110",// 9
  "11111",// 0
};

struct Node                                                  // for LinkedList
{
  char onOff;
  Node * next = NULL;
};

class LinkedList                                             // This LinkedList stores the input which is later translated
{
  public:
  LinkedList(){
    head = NULL;
    tail = NULL;
    count = 0;
  }
  void insert(char newData)
  {
    if(count == 0)
    {
      head = new Node;
      head->onOff = newData;
      head->next = NULL;
      tail = head;
    }
    else
    {
      Node* temp = tail;
      tail = new Node;
      tail->onOff = newData;
      temp->next = tail;
    }
    count++;
  }
    char readMorse() const                                  // Decodes the current morse code in the linked list
    {
      Node* temp = head;
      String code = "";

      while(temp != NULL)
      {
        code += temp->onOff;
        temp = temp->next;
      }
      for (int i = 0; i < NUMOFCHAR; i++)
      {
        if (code == TRANSLATE[i])
        {
          lcdDisplay.print(MORSE[i]);
          Serial.print(MORSE[i]);
          col++;
          if (col > 15 && row == 0)
          {
            row = 1;
            col = 0;
            lcdDisplay.setCursor(0,1);
          }
          
          break;
        }
      }

    }

    int getCount() const
    {
      return count;
    }

    void deleteList()
    {
      Node* temp = head; 
      while (temp != NULL)
      {
        head = head->next;
        delete temp;
        temp = head;
      }

      count = 0;
      }
  private:
  Node* head;
  Node* tail;
  int count;
};

LinkedList morseLink;

void setup() {
  pinMode (BUZZER, OUTPUT);                                                 // Pin setup
  pinMode (BUTTON, INPUT);
  pinMode (LIGHTSENSOR, INPUT);
  
  Serial.begin(9600);
  lcdDisplay.begin(16,2);
  lcdDisplay.setRGB(0,255,255);
  lcdDisplay.blink();
}


void loop() {
  light_sensor = analogRead(LIGHTSENSOR);


  while(digitalRead(BUTTON) == HIGH || light_sensor > LIGHT_THRESH)           // Pressing button or exposing to light
    {
      digitalWrite(BUZZER,HIGH);
      delay(100);
      beepLength++;
      light_sensor = analogRead(LIGHTSENSOR);
    }


  if (beepLength > DELETE)
  {
    Serial.println("\n\n****CLEAR****\n");
    lcdDisplay.clear();
    row = 0;
    col = 0;
  }
  else if (beepLength > LONGBEEP)
    morseLink.insert('1');
  else if(beepLength >= SHORTBEEP)
    morseLink.insert('0');

  beepLength = 0;

  while(digitalRead(BUTTON) == LOW && light_sensor <= LIGHT_THRESH)         // Neither pressing button nor exposing to light
    {
      digitalWrite(BUZZER,LOW);
      delay(100);
      offLength++;
      light_sensor = analogRead(LIGHTSENSOR);
      if(offLength > WAIT && morseLink.getCount() != 0)
        {
          morseLink.readMorse();
          morseLink.deleteList();
        }
    }
  offLength = 0;
}
