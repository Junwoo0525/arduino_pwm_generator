#include <Arduino.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define CLK 2    // 2번핀을 CLK에 연결
#define DT 3     // 3번핀을 DT에 연결

Encoder myEnc(CLK, DT);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int sw = 4;
int mode = 0;
int encoderBuffer = 0;
int encodercnt = 0;

int32_t frequency = 241; //frequency (in Hz)

//PWM
#include <PWM.h>
int pwm = 9;
int pwmdutyper = 0;

//ON&OFF
bool out = false;

long oldPosition  = 0;

void setup() {
  lcd.init();
  lcd.init();

  pinMode(sw, INPUT);

  InitTimersSafe();
  bool success = SetPinFrequencySafe(pwm, frequency);
  if (success) {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
  }

  // 시리얼 출력 개시
  Serial.begin(9600);
  
  lcd.setCursor(10, 0);
  lcd.print("0  ");
  lcd.setCursor(10, 1);
  lcd.print("OFF");
}

void loop() {
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Duty : ");
  lcd.setCursor(0, 1);
  lcd.print("Output : ");

  if (digitalRead(sw) == LOW) {
    if (out == false) {
      //setduty(pwmdutyper);
      out = true;
      lcd.setCursor(10, 1);
      lcd.print("ON ");
      Serial.print("ON & duty : ");
      Serial.println(pwmdutyper);
    }
    else {
      //setduty(0);
      out = false;
      lcd.setCursor(10, 1);
      lcd.print("OFF");
      Serial.println("OFF");
    }
    while (1) {
      if (digitalRead(sw) == HIGH)
        break;
    }
  }

  readEncoder();

}

int setduty(int dutyper) {
  int duty = map(dutyper, 0, 100, 0, 255);
  pwmWrite(pwm, duty);
}

void readEncoder() {
  long newPosition = myEnc.read();
  long newPositionDiv4 = newPosition / 4;
  if ((newPosition % 4 == 0) and (newPositionDiv4 != oldPosition)) {
    if (oldPosition > newPositionDiv4) {
      if (encoderBuffer == 1){
        encodercnt++;
      }
      else{
        encodercnt = 0;
      }
      encoderBuffer = 1;
      if (encodercnt > 1){
        encodercnt = 0;
        encoderBuffer = 0;
        if (mode >= 4) mode = 0;
        else mode++;
        Serial.println(mode);
        pwmMode();
      }
      
    }
    else if (oldPosition < newPositionDiv4) {
      if (encoderBuffer == -1){
        encodercnt++;
      }
      else{
        encodercnt = 0;
      }
      encoderBuffer = -1;
      if (encodercnt > 1){
        encodercnt = 0;
        encoderBuffer = 0;
        if (mode == 0) mode = 4;
        else mode--;
        Serial.println(mode);
        pwmMode();
      }
    }
    else {
      Serial.println("error");
    }
    oldPosition = newPositionDiv4;

  }
}

void pwmMode() {
  lcd.setCursor(10, 0);
  switch (mode) {
    case 0:
      pwmdutyper = 0;
      lcd.print("0  ");
      setduty(pwmdutyper);
      break;
    case 1:
      pwmdutyper = 25;
      lcd.print("25 ");
      setduty(pwmdutyper);
      break;
    case 2:
      pwmdutyper = 50;
      lcd.print("50 ");
      setduty(pwmdutyper);
      break;
    case 3:
      pwmdutyper = 75;
      lcd.print("75 ");
      setduty(pwmdutyper);
      break;
    case 4:
      pwmdutyper = 100;
      lcd.print("100");
      setduty(pwmdutyper);
      break;
  }
}
