// Shelly D'Silva   2-2-2014
// This project was built on the Arduino Uno - ATmega328P
// This code sets up the DS1307 Real Time clock on the Arduino board to controll 4 dosing pumps
// The RTC keeps track of time, the code checks it and turns on the pumps at a specified time
// to dose your aquarium


#include <Time.h>

#include <LiquidCrystal.h>     // initialise LCD library
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);


// Utils
/*byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}*/

// Convert binary coded decimal to normal decimal numbers
/*byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}*/


// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// Assumes you're passing in valid numbers
void setDateTime(int hr, int mins, int secs, int month, int dy, int yr)
{
  setTime(hr,mins,secs,month,dy,yr);
}

/*Function to display all numbers in two width*/
void twoFieldDisplay(int val)
{
  switch(val)
  {
  case 0: 
    lcd.print("00");
    break;
  case 1: 
    lcd.print("01");
    break;
  case 2: 
    lcd.print("02");
    break;
  case 3: 
    lcd.print("03");
    break;
  case 4: 
    lcd.print("04");
    break;
  case 5: 
    lcd.print("05");
    break;
  case 6: 
    lcd.print("06");
    break;
  case 7: 
    lcd.print("07");
    break;
  case 8: 
    lcd.print("08");
    break;
  case 9: 
    lcd.print("09");
    break;
  default: 
    lcd.print(val,DEC);
  }
  
 }

// Displays the date and time from the ds1307
void displayDateTime(int dayOfWeek,int hr, int mins, int secs, int mnth, int day, int yr)
{

  switch(dayOfWeek)
  {
  case 1:
    lcd.print("SUN");
    break;
  case 2:
    lcd.print("MON");
    break;
  case 3:
    lcd.print("TUE");
    break;
  case 4:
    lcd.print("WED");
    break;
  case 5:
    lcd.print("THU");
    break;
  case 6:
    lcd.print("FRI");
    break;
  case 7:
    lcd.print("SAT");
    break;
  }
  lcd.print("  ");
  
  twoFieldDisplay(day);
    lcd.print("/");
  twoFieldDisplay(mnth);  
   lcd.print("/");
  twoFieldDisplay(yr); 
   lcd.print(" ");
  twoFieldDisplay(hr); 
   lcd.print(":");
  twoFieldDisplay(mins); 
}

//define pin
const int motorPin0 = 8;
const int motorPin1 = 9;
const int motorPin2 = 10;
const int motorPin3 = 11;
const int left = A0;
const int right = A1;
const int up = 12;
const int down = 13;
const int set = A2;
const int alarm = A3;


boolean time_set = false;
int time_set_ptr = 0;
int alarm_set_ptr = 0;
int alarm_no =1;
int LCD_Disp_ctr = 14; //LCD counter for Second Line Scrolling
int hr, mins, secs, mnth, dy,  yr, weekDay;

/*Alarm Variables and EEPROM variables for same*/
boolean alarm_set = false; //EEPROM location 1 -- NOT REQUIRED
boolean dosing_set = false; //EEPROM location 2 -- NOT REQUIRED
boolean al_set[5] = {
  false,false,false,false,false}; //EEPROM location 3-7
boolean mon_flag[5] = {
  false,false,false,false,false}; //EEPROM location 8-12
boolean tue_flag[5] = {
  false,false,false,false,false}; //EEPROM location 13-17
boolean wed_flag[5] = {
  false,false,false,false,false}; //EEPROM location 18-22
boolean thu_flag[5] = {
  false,false,false,false,false}; //EEPROM location 23-27
boolean fri_flag[5] = {
  false,false,false,false,false}; //EEPROM location 28-32
boolean sat_flag[5] = {
  false,false,false,false,false}; //EEPROM location 33-37
boolean sun_flag[5] = {
  false,false,false,false,false}; //EEPROM location 38-42
boolean MOTOR_ON[5] = {
  false,false,false,false,false}; //EEPROM location 43-47
byte al_minute[5] = {
  0,0,0,0,0}
, //EEPROM location 48-52
al_hour[5] = {
  0,0,0,0,0}
, //EEPROM location 53-57
dosing_ml[5] = {
  0,0,0,0,0}; //EEPROM location 58-62

/* Factory reset of all variables */
void factoryReset()
{
  lcd.clear();
  delay(50);
  lcd.print("  Factory Reset");
  delay(50);
  lcd.clear();
  delay(50);
  lcd.print("  Factory Reset");
  delay(50);
  for(alarm_no=1;alarm_no<5;alarm_no++)
  {
    al_set[alarm_no] = false;
    al_hour[alarm_no] = 0;
    al_minute[alarm_no] = 0;
    sun_flag[alarm_no] = false;
    mon_flag[alarm_no] = false;
    tue_flag[alarm_no] = false;
    wed_flag[alarm_no] = false;
    thu_flag[alarm_no] = false;
    fri_flag[alarm_no] = false;
    sat_flag[alarm_no] = false;
    MOTOR_ON[alarm_no] = false;
    dosing_ml[alarm_no] = 0;
  }
}


unsigned long prev_time;
void setup()  // run once, when the sketch starts
{
  lcd.begin(16, 2); // initializing the LCD display of 16x2
  Serial.begin(9600);
 
  pinMode(motorPin0, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(left, INPUT);
  pinMode(right, INPUT);
  pinMode(up, INPUT);
  pinMode(down, INPUT);
  pinMode(set, INPUT);
  pinMode(alarm, INPUT);
  

  LCD_Disp_ctr = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" SMART AQUARIUM");
  delay(800);
  /*while(LCD_Disp_ctr<10)       //Welcome Screen
  {
    //lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print(" SMART AQUARIUM");
    delay(800);
    lcd.setCursor(LCD_Disp_ctr,1);
    lcd.print("an aqua@home product. www.aqua@home.com");
    LCD_Disp_ctr ++;
    //delay(200);
  }
  LCD_Disp_ctr = 0;*/

  lcd.clear();
  delay(550);
  lcd.print(" Please Set Time");
  delay(550);
  lcd.clear();
  delay(550);
  lcd.print(" Please Set Time");
  delay(550);
  lcd.clear();
  delay(550);
  lcd.print(" Please Set Time");
  delay(550);
  

  Serial.println("Control1 entered here");
  //getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &mnth, &year);
  hr = hour();
  mins = minute();
  secs = second();
  mnth = month();
  dy = day();
  yr = year();
  weekDay = weekday();

  while(time_set == false)
  {
    Serial.println("Control2 entered here");
    lcd.clear();
    
    displayDateTime(weekDay , hr, mins, secs, mnth, dy, yr);
    
     // For setting time and date in project
    if (digitalRead(set) == LOW) { 
      while(digitalRead(set) == LOW); 
      time_set = true; 
    }
    if (digitalRead(left) == LOW) {
      while(digitalRead(left) == LOW);
      time_set_ptr--;
    } 
    if(time_set_ptr<0||time_set_ptr==255) time_set_ptr =0;
    if (digitalRead(right) == LOW) {
      while(digitalRead(right) == LOW);
      time_set_ptr++;
    } 
    if(time_set_ptr>5) time_set_ptr =5;
    switch(time_set_ptr)
    {
    case 0: 
      lcd.setCursor(1,1);
      lcd.print("^");
      lcd.home();
      if (digitalRead(up) == LOW) {
        while(digitalRead(up) == LOW);
        weekDay++;
      } 
      if(weekDay>7) weekDay = 7;
      if (digitalRead(down) == LOW){ 
        while(digitalRead(down) == LOW);
        weekDay--;
      } 
      if(weekDay<1) weekDay = 1;
      break;
    case 1: 
      lcd.setCursor(5,1);
      lcd.print("^");
      lcd.home(); 
      if (digitalRead(up) == LOW) {
        while(digitalRead(up) == LOW);
        dy++;
      } 
      if(dy>31) dy = 31;
      if (digitalRead(down) == LOW) {
        while(digitalRead(down) == LOW);
        dy--;
      } 
      if(dy<1) dy = 1;
      break; 
    case 2: 
      lcd.setCursor(8,1);
      lcd.print("^");
      lcd.home();
      if (digitalRead(up) == LOW) {
        while(digitalRead(up) == LOW);
        mnth++;
      } 
      if(mnth>12) mnth =12;
      if (digitalRead(down) == LOW) {
        while(digitalRead(down) == LOW);
        mnth--;
      } 
      if(mnth<1) mnth =1;
      break;
    case 3: 
      lcd.setCursor(11,1);
      lcd.print("^");
      lcd.home();
      if (digitalRead(up) == LOW) {
        while(digitalRead(up) == LOW);
        yr++;
      }  
      if(yr>99) yr = 99;
      if (digitalRead(down) == LOW) {
        while(digitalRead(down) == LOW);
        yr--;
      } 
      if(yr<0||yr==255) yr = 0;
      break;
    case 4: 
      lcd.setCursor(14,1);
      lcd.print("^");
      lcd.home();
      if (digitalRead(up) == LOW) {
        while(digitalRead(up) == LOW);
        hr ++;
      } 
      if(hr >23) hr  = 23;
      if (digitalRead(down) == LOW) {
        while(digitalRead(down) == LOW);
        hr --;
      } 
      if(hr <0||hr ==255) hr  = 0;
      break;      
    case 5: 
      lcd.setCursor(17,1);
      lcd.print("^");
      lcd.home();
      if (digitalRead(up) == LOW) {
        while(digitalRead(up) == LOW);
        mins ++;
      } 
      if(mins >59) mins  =59;
      if (digitalRead(down) == LOW) {
        while(digitalRead(down) == LOW);
        mins --;
      } 
      if(mins <0||mins ==255) mins  =0;
      break;         
      /*case 6: lcd.setCursor(19,1);lcd.print("^");lcd.home();
       second = 0;
       break;  */

    }

    delay(5);
  }

  setDateTime(hr, mins, secs, mnth, dy,  yr);
}


void loop() // run over and over again
{

  //byte second, minute, hours, dayOfWeek, dayOfMonth, month, year;
  int dayOfWeek,hours,minutes,seconds,mnth,dayOfMonth, years;
  //getDateDs1307(&second, &minute, &hours, &dayOfWeek, &dayOfMonth, &mnth, &year);
  hours = hour();
  minutes = minute();
  seconds = second();
  mnth = month();
  dayOfMonth = day();
  years = year();
  dayOfWeek = weekday();
  lcd.clear();
  lcd.setCursor(0,0);
  displayDateTime(dayOfWeek,hours,minutes,seconds,mnth,dayOfMonth, years);
  //displayDateDs1307(second,minute,hour,dayOfWeek,dayOfMonth,month,year);

  if((MOTOR_ON[1] == false)&&(MOTOR_ON[2] == false)&&(MOTOR_ON[3] == false)&&(MOTOR_ON[4] == false))
  {
    lcd.setCursor(LCD_Disp_ctr,1);
    lcd.print("PUMP 01:");
    if(al_set[1]==true)lcd.print("ON");
    else lcd.print("OFF");
    lcd.print(" 02:");
    if(al_set[2]==true)lcd.print("ON");
    else lcd.print("OFF"); 
    lcd.print(" 03:");
    if(al_set[3]==true)lcd.print("ON");
    else lcd.print("OFF"); 
    lcd.print(" 04:");
    if(al_set[4]==true)lcd.print("ON");
    else lcd.print("OFF"); 
    LCD_Disp_ctr --;
    delay(50);
    if(LCD_Disp_ctr == -21) LCD_Disp_ctr=16;

    /* ------Factory Reset
     * Factory reset is done by pressing up and down keys together for some time
     */
    if ((digitalRead(up) == LOW)&&(digitalRead(down) == LOW))
    {
      factoryReset();
    }


    if (digitalRead(alarm) == LOW)
    {
      alarm_no = 1;
      alarm_set = false;
      while(alarm_set ==false)
      { 
        lcd.clear();
        lcd.print("P");
        lcd.print(alarm_no);
        lcd.print(" SU M TU W TH F SA");
        if (digitalRead(set) == LOW) { 
          while(digitalRead(set) == LOW);
          alarm_no++; 
          if(alarm_no>4)alarm_set = true; 
        }
        if (digitalRead(left) == LOW) {
          while(digitalRead(left) == LOW);
          alarm_set_ptr--;
        } 
        if(alarm_set_ptr<0||alarm_set_ptr==255) alarm_set_ptr =0;
        if (digitalRead(right) == LOW) {
          while(digitalRead(right) == LOW);
          alarm_set_ptr++;
        } 
        if(alarm_set_ptr>7) alarm_set_ptr =7;
        lcd.setCursor(0,1);
        if(al_set[alarm_no]==true)lcd.print("ON ");
        else lcd.print("OFF");
        if(sun_flag[alarm_no]==true)lcd.print(" *");
        else lcd.print(" _");
        if(mon_flag[alarm_no]==true)lcd.print(" *");
        else lcd.print(" _");
        if(tue_flag[alarm_no]==true)lcd.print("  *");
        else lcd.print("  _");
        if(wed_flag[alarm_no]==true)lcd.print(" *");
        else lcd.print(" _");
        if(thu_flag[alarm_no]==true)lcd.print("  *");
        else lcd.print("  _");
        if(fri_flag[alarm_no]==true)lcd.print(" *");
        else lcd.print(" _");
        if(sat_flag[alarm_no]==true)lcd.print("  *");
        else lcd.print("  _");
        switch(alarm_set_ptr)
        {

        case 0: 
          if (digitalRead(up) == LOW) al_set[alarm_no] = true;
          if (digitalRead(down) == LOW) al_set[alarm_no] = false;
          break;
        case 1:  
          if (digitalRead(up) == LOW) sun_flag[alarm_no] = true;
          if (digitalRead(down) == LOW) sun_flag[alarm_no] = false;
          break;
        case 2:  
          if (digitalRead(up) == LOW) mon_flag[alarm_no] = true;
          if (digitalRead(down) == LOW) mon_flag[alarm_no] = false;
          break;
        case 3: 
          if (digitalRead(up) == LOW) tue_flag[alarm_no] = true;
          if (digitalRead(down) == LOW) tue_flag[alarm_no] = false;
          break;  
        case 4: 
          if (digitalRead(up) == LOW) wed_flag[alarm_no] = true;
          if (digitalRead(down) == LOW) wed_flag[alarm_no] = false;
          break;
        case 5:  
          if (digitalRead(up) == LOW) thu_flag[alarm_no] = true;
          if (digitalRead(down) == LOW) thu_flag[alarm_no] = false;
          break;        
        case 6:  
          if (digitalRead(up) == LOW) fri_flag[alarm_no] = true;
          if (digitalRead(down) == LOW) fri_flag[alarm_no] = false;
          break;
        case 7:  
          if (digitalRead(up) == LOW) sat_flag[alarm_no] = true;
          if (digitalRead(down) == LOW) sat_flag[alarm_no] = false;
          break;           

        }
        delay(10);
      }
      alarm_no=1;
      alarm_set_ptr = 0;
      dosing_set = false;
      while(dosing_set ==false)
      { 
        while(al_set[alarm_no]==false) alarm_no++;
        if(alarm_no>4)break;
        lcd.clear();
        lcd.print("P");
        lcd.print(alarm_no);
        lcd.print(" ");
        switch(al_hour[alarm_no])
        {
        case 0: 
          lcd.print("00");
          break;
        case 1: 
          lcd.print("01");
          break;
        case 2: 
          lcd.print("02");
          break;
        case 3: 
          lcd.print("03");
          break;
        case 4: 
          lcd.print("04");
          break;
        case 5: 
          lcd.print("05");
          break;
        case 6: 
          lcd.print("06");
          break;
        case 7: 
          lcd.print("07");
          break;
        case 8: 
          lcd.print("08");
          break;
        case 9: 
          lcd.print("09");
          break;
        default: 
          lcd.print(al_hour[alarm_no],DEC);
        }
        //lcd.print(al_hour[alarm_no]);
        lcd.print(":");
        switch(al_minute[alarm_no])
        {
        case 0: 
          lcd.print("00");
          break;
        case 1: 
          lcd.print("01");
          break;
        case 2: 
          lcd.print("02");
          break;
        case 3: 
          lcd.print("03");
          break;
        case 4: 
          lcd.print("04");
          break;
        case 5: 
          lcd.print("05");
          break;
        case 6: 
          lcd.print("06");
          break;
        case 7: 
          lcd.print("07");
          break;
        case 8: 
          lcd.print("08");
          break;
        case 9: 
          lcd.print("09");
          break;
        default: 
          lcd.print(al_minute[alarm_no],DEC);
        }
        //lcd.print(al_minute[alarm_no]);
        lcd.print("  Dosing:");
        lcd.print(dosing_ml[alarm_no]);
        if (digitalRead(set) == LOW) {
          while(digitalRead(set) == LOW); 
          alarm_no++; 
          if(alarm_no>4)dosing_set = true; 
        }
        if (digitalRead(left) == LOW) {
          while(digitalRead(left) == LOW); 
          alarm_set_ptr--;
        } 
        if(alarm_set_ptr<0||alarm_set_ptr==255) alarm_set_ptr =0;
        if (digitalRead(right) == LOW) {
          while(digitalRead(right) == LOW); 
          alarm_set_ptr++;
        } 
        if(alarm_set_ptr>2) alarm_set_ptr =2;

        switch(alarm_set_ptr)
        {

        case 0:   
          lcd.setCursor(3,1);
          lcd.print("^");
          lcd.home();
          if (digitalRead(up) == LOW) {
            while(digitalRead(up) == LOW);
            al_hour[alarm_no]++;
          } 
          if(al_hour[alarm_no]>23) al_hour[alarm_no] = 23;
          if (digitalRead(down) == LOW) {
            while(digitalRead(down) == LOW);
            al_hour[alarm_no]--;
          } 
          if(al_hour[alarm_no]<0||al_hour[alarm_no]==255) al_hour[alarm_no] = 0;
          break;
        case 1:   
          lcd.setCursor(6,1);
          lcd.print("^");
          lcd.home();
          if (digitalRead(up) == LOW) {
            while(digitalRead(up) == LOW);
            al_minute[alarm_no]++;
          } 
          if(al_minute[alarm_no]>59) al_minute[alarm_no] = 59;
          if (digitalRead(down) == LOW) {
            while(digitalRead(down) == LOW);
            al_minute[alarm_no]--;
          } 
          if(al_minute[alarm_no]<0||al_minute[alarm_no]==255) al_minute[alarm_no] = 0;
          break; 
        case 2:   
          lcd.setCursor(17,1);
          lcd.print("^");
          lcd.home();
          if (digitalRead(up) == LOW) {
            while(digitalRead(up) == LOW);
            dosing_ml[alarm_no] = dosing_ml[alarm_no]+10;
          } 
          if (digitalRead(down) == LOW) {
            while(digitalRead(up) == LOW);
            dosing_ml[alarm_no] = dosing_ml[alarm_no]-10;
          } 
          if(dosing_ml[alarm_no]<0||dosing_ml[alarm_no]==255) dosing_ml[alarm_no] = 0;
          break;

        }
        delay(10);
      }
    }

    if (digitalRead(set) == LOW)
    {
      lcd.clear();
      if((al_set[1]==false)&&(al_set[2]==false)&&(al_set[3]==false)&&(al_set[4]==false)){
        lcd.print("   No Alarms Set"); 
        delay(200);
      }

      for(alarm_no=1;alarm_no<5;alarm_no++)
      {
        if(al_set[alarm_no]==true)
        {
          lcd.clear();
          lcd.print("P");
          lcd.print(alarm_no);
          lcd.print(" ");
          lcd.print(al_hour[alarm_no]);
          lcd.print(":");
          lcd.print(al_minute[alarm_no]);
          lcd.print("  Dosing:");
          lcd.print(dosing_ml[alarm_no]);
          lcd.setCursor(0,1);
          if(sun_flag[alarm_no]==true) lcd.print("SU");
          if(mon_flag[alarm_no]==true) lcd.print(" M");
          if(tue_flag[alarm_no]==true) lcd.print(" TU");
          if(wed_flag[alarm_no]==true) lcd.print(" W");
          if(thu_flag[alarm_no]==true) lcd.print(" TH");
          if(fri_flag[alarm_no]==true) lcd.print(" F");
          if(sat_flag[alarm_no]==true) lcd.print(" SA");
          delay(100);
        }

      }

    }

    MOTOR_ON[1] = false;
    MOTOR_ON[2] = false;
    MOTOR_ON[3] = false;
    MOTOR_ON[4] = false;

    for(alarm_no=1;alarm_no<5;alarm_no++)
    {
      if((dayOfWeek == 1)&&(sun_flag[alarm_no]==true)&&(al_hour[alarm_no]==hours)&&(al_minute[alarm_no]==minutes)) MOTOR_ON[alarm_no] = true;
      if((dayOfWeek == 2)&&(mon_flag[alarm_no]==true)&&(al_hour[alarm_no]==hours)&&(al_minute[alarm_no]==minutes)) MOTOR_ON[alarm_no] = true;
      if((dayOfWeek == 3)&&(tue_flag[alarm_no]==true)&&(al_hour[alarm_no]==hours)&&(al_minute[alarm_no]==minutes)) MOTOR_ON[alarm_no] = true;
      if((dayOfWeek == 4)&&(wed_flag[alarm_no]==true)&&(al_hour[alarm_no]==hours)&&(al_minute[alarm_no]==minutes)) MOTOR_ON[alarm_no] = true;
      if((dayOfWeek == 5)&&(thu_flag[alarm_no]==true)&&(al_hour[alarm_no]==hours)&&(al_minute[alarm_no]==minutes)) MOTOR_ON[alarm_no] = true;
      if((dayOfWeek == 6)&&(fri_flag[alarm_no]==true)&&(al_hour[alarm_no]==hours)&&(al_minute[alarm_no]==minutes)) MOTOR_ON[alarm_no] = true;
      if((dayOfWeek == 7)&&(sat_flag[alarm_no]==true)&&(al_hour[alarm_no]==hours)&&(al_minute[alarm_no]==minutes)) MOTOR_ON[alarm_no] = true;
    }

    prev_time = millis();
  }
  while((MOTOR_ON[1] == true)||(MOTOR_ON[2] == true)||(MOTOR_ON[3] == true)||(MOTOR_ON[4] == true))
  {
    lcd.clear();
    lcd.print("    Dosing.....");
    if(MOTOR_ON[1] == true)
    {
      analogWrite(motorPin0, 255);
      if(millis()-prev_time>=dosing_ml[1]) {
        MOTOR_ON[1] = false;
        analogWrite(motorPin0, 0);
      }
    }
    if(MOTOR_ON[2] == true)
    {
      analogWrite(motorPin1, 255);
      if(millis()-prev_time>=dosing_ml[2]) {
        MOTOR_ON[2] = false;
        analogWrite(motorPin1, 0);
      }
    }
    if(MOTOR_ON[3] == true)
    {
      analogWrite(motorPin2, 255);
      if(millis()-prev_time>=dosing_ml[3]) {
        MOTOR_ON[3] = false;
        analogWrite(motorPin2, 0);
      }
    }
    if(MOTOR_ON[4] == true)
    {
      analogWrite(motorPin3, 255);
      if(millis()-prev_time>=dosing_ml[4]) {
        MOTOR_ON[4] = false;
        analogWrite(motorPin3, 0);
      }
    }

  }
  delay(10);
  //analogWrite(motorPin0, 0);
  //analogWrite(motorPin1, 0);
  //analogWrite(motorPin2, 0);
  //
}


