// Unzip folder into Libraries. RENAME folder IRremote
#include "IRremote.h"
#include <EEPROM.h>
#include <LiquidCrystal.h>


//pin declarations
//Pin 0-5 are liquid crystal
LiquidCrystal lcd(11, 10, 5, 4, 3, 2);
int buzzerPin = 6;     
int Motion_Input = 7;  // Input pin for PIR/Motion sensor
int IR_Input = 8; // IR Reciever
int arm_Input = 9; //ARM/Disarm button on board
           
//

//ARM/Disarm/ Button variables, alarm alert 
bool ac = false; //arm command
bool dc = false; //disarm command
bool active = false; //Determines if the alarm will react to motion. 
int oldKey = 0;
bool alert = false; 

//

//EEPROM Variables
int passcode[4]; 
int passcodeEntered[4]; 
int passCounter = 0; //counter that increments when a number is inputed. 
bool storePass;
int count = 0 ;
//

//IR Variables
IRrecv irrecv(IR_Input);           // create instance of 'irrecv'
decode_results results;            // create instance of 'decode_results'
//

//Motion sensor variables
int pirState = LOW;             // Initially no motion is detected
int val = 0;                    // reading the pin status
//
void writeLCD(char text[])
{
lcd.clear();
  lcd.write(text);
}


void readPin()
{
  
for(int i= 0; i < 4; i++)
{
  passcode[i] = EEPROM.read(i);

}
}

void setup()  //Multiple setup methods will be called here. 
{
Serial.begin(9600);

 IREEPROM_Setup();
 motion_Setup();
readPin();
 LCD_Setup();

pinMode(arm_Input,INPUT_PULLUP); //setup button with internal pull-up.
 
}

void clear()
{
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}


void LCD_Setup()
{
   // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  writeLCD("PRESS UP ARROW");

}

void motion_Setup()
{
  pinMode(buzzerPin, OUTPUT);      // declare LED as output
  pinMode(Motion_Input, INPUT);     // declare sensor as input  
}


void IREEPROM_Setup()
{



    //Serial.println("IR Receiver Raw Data + Button Decode Test");
  irrecv.enableIRIn(); // Start the receiver

}
//Pin setup
void setPass()
{
for (int i=0; i<4; ++i) {
passcode[i] = passcodeEntered[i];
  }
}
int i =0;


void getPin(){    

 char ch = Serial.read();
    if (ch >= '0' && ch <= '9')
    {

      int pin = ch - '0';
  
      EEPROM.write(i,pin);
       i++;
        readPin();
 
    
   }
   
  }








void loop()  
{
    //Read the passcode
getPin();
  IR_Loop();
 motion_Loop();
 LCD_Loop();
  buttonLoop();
outputAlarm(active);

}

void LCD_Loop()
{
    // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
 // lcd.setCursor(0, 1);
  // print the number of seconds since reset:
//  lcd.print(millis() / 1000);
  
}

void IR_Loop()
{
   if (irrecv.decode(&results)) // have we received an IR signal?
  {
//Serial.println(results.value, HEX);  //UN Comment to see raw values
    translateIR(); 
    irrecv.resume(); // receive the next value
  }  
}

void outputAlarm(bool detection)
{

  if(detection)
  {
       // digitalWrite(buzzerPin, HIGH);  
      //  writeLCD("Buzzer high");
  }
  else
  {
       // digitalWrite(buzzerPin, LOW);  
      //  writeLCD("Buzzer low");
  }
}
void motion_Loop()
{
  
  if(active) //motion will only be used if the alarm is armed. if not armed wait to be armed and do nothing. 
  {
  lcd.setCursor(0, 1); //new line
  val = digitalRead(Motion_Input);  // read input value
  if (val == HIGH) {            // check if the input is HIGH

    if (pirState == LOW) {
      // we have just turned on

       alert = true; 
       
 writeLCD("DETECTED");
 digitalWrite(buzzerPin, HIGH);  
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
   
    if (pirState == HIGH){
      // we have just turned of
   //  lcd.print("Motion ended!");
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
   }

}

void buttonLoop()
{
lcd.setCursor(0, 0);
int key = digitalRead(arm_Input);
  if(key!= oldKey)
  {
    if (key == LOW)
    {
      if(!active)
      {
        active = true; 
        writeLCD("ARMED");
      
      }
      else
      {
        active = false; 
        writeLCD("DISARMED");
      
        pirState = LOW;             //Reset to  to avoid a detection output as soon as armed. 
        val = 0;       
         
        
      }
    }
  //  Serial.println(active);
    
  }
oldKey=key;
  
}

void translateIR() // takes action based on IR code received
{
   checkPassword();
 switch(results.value)
 {
  
  //Check for arm or disarm command before checking for pass
 case 0xFF906F:  
writeLCD("PASS TO ARM?"); //Arm 
ac = true; 
 break;

 case 0xFFE01F:  
 writeLCD("PASS TO DISARM?");  //Disarm
 dc = true; 
  break;

   case 0xFF9867:  //Set new pass
    


    break;
    
 }

if(ac|| dc )
  {
  switch(results.value)
 {

  case 0xFFA25D:  
    lcd.print(" CH-            "); 
    break;

  case 0xFF629D:  
    lcd.print(" CH             "); 
    break;

  case 0xFFE21D:  
   lcd.print(" CH+            "); 
    break;

  case 0xFF22DD:  
   lcd.print(" PREV           "); 
    break;

  case 0xFF02FD:  
 //checkPassword();

    break;
  case 0xFFC23D:  
  // lcd.print("PLAY/PAUSE     "); 
    break;

 
  case 0xFFA857:  
 //   lcd.print(" VOL+           "); 
    break;

 

  case 0xFF6897:  
 lcd.print(" 0              "); 
       passcodeEntered[passCounter] = 0;
    passCounter++;
    break;

  case 0xFF9867:  //Set new pass



    break;

  case 0xFFB04F:  
  lcd.print(" 200+           "); 
    break;

  case 0xFF30CF:  

writeLCD("1");
   
    passcodeEntered[passCounter] = 1;
    passCounter++;
    break;

  case 0xFF18E7:  
writeLCD("2");
     passcodeEntered[passCounter] = 2;
     passCounter++;
    break;

  case 0xFF7A85:  
writeLCD("3");
      passcodeEntered[passCounter] = 3;
      passCounter ++;
    break;

  case 0xFF10EF:  
writeLCD("4");
    passcodeEntered[passCounter] = 4;
    passCounter++;
    break;

  case 0xFF38C7:  
writeLCD("5"); 
     passcodeEntered[passCounter] = 5;
    passCounter++;
    break;

  case 0xFF5AA5:  
writeLCD("6");
     passcodeEntered[passCounter] = 6;
    passCounter++;
    break;

  case 0xFF42BD:  
writeLCD("7");
     passcodeEntered[passCounter] = 7;
    passCounter++;
    break;

  case 0xFF4AB5:  
writeLCD("8");
     passcodeEntered[passCounter] = 8;
    passCounter++;
    break;


  case 0xFF52AD:  
writeLCD("9");
     passcodeEntered[passCounter] = 9;
    passCounter++;
    break;
    
  case 0xFFFFFFFF: //test
  // lcd.print("held down key"); 
    break;
 
  default: 
//writeLCD("BAD SIGNAL");
  break;

  delay(500);
   }
 }
} 

void printPasscode()
{
 //  Serial.println("The passcode is");
  for(int i = 0; i< 4; i++)
  {
    lcd.setCursor(0, i);
   lcd.write( passcodeEntered[i]);
 // Serial.print(passcodeEntered[i]);
  }
}




void checkPassword()
{

if(passCounter > 4) //Passcode is only 4 digits. Please enter passcode again.
{
writeLCD("TRY AGAIN");
 passCounter = 0;
 return; 
}

 if(passCounter == 4)
{
int i, N;
bool is_equal = true;
for (i=0; i<4; ++i) {
  if (passcode[i] != passcodeEntered[i]) {
        is_equal = false;
        }  
}
if (is_equal == true)
{
 // Serial.println("Passcode matches");
  if(ac)
  {
     active = true; //arm/activate the alarm
  writeLCD("ARMED");
  passCounter = 0;
  }
  else if(dc)
  {
     active = false; //arm/activate the alarm
  writeLCD("DISARMED");
  digitalWrite(buzzerPin, LOW);  
     passCounter = 0;
  }
 
}
else
{
//  Serial.println("Passcode Incorect");
writeLCD("TRY AGAIN");
   passCounter = 0;
}
 ac = false; 
 dc = false; 
}


}

