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
int modeOverflowCnt = 0;
int encoderBuffer = 0;
int encodercnt = 0;

double sw_time_cur = 0;
double sw_time_last = 0;
int sw_filter_time = 200;


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

  lcd.backlight();

  pinMode(sw, INPUT_PULLUP);

  InitTimersSafe();
  bool success = SetPinFrequencySafe(pwm, frequency);
  if (success) {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
  }

  // 시리얼 출력 개시
  Serial.begin(9600);

  lcd.setCursor(1, 0);
  lcd.print("Made by");
  lcd.setCursor(5, 1);
  lcd.print("Angchicken");

  delay(1000);

  lcd.setCursor(1, 0);
  lcd.print("Duty(%) :   0");
  lcd.setCursor(1, 1);
  lcd.print("Output  : OFF   ");
}

void loop() {
  if (digitalRead(sw) == LOW) {
    sw_time_cur = millis();
    if (sw_time_cur - sw_time_last > sw_filter_time) {
      sw_time_last = millis();
      if (out == false) {
        out = true;
        lcd.setCursor(11, 1);
        lcd.print(" ON");
        Serial.print("ON & duty : ");
        Serial.println(pwmdutyper);
        pwmSW();
      }
      else {
        out = false;
        lcd.setCursor(11, 1);
        lcd.print("OFF");
        Serial.println("OFF");
        pwmSW();
      }
    }
    else{
      Serial.println("skiped!!");
    }
    while (1) {
      if (digitalRead(sw) == HIGH)
        break;
    }
  }

  readEncoder();

}

int setduty(int dutyper) {
  int duty = map(dutyper, 0, 99, 0, 255);
  pwmWrite(pwm, duty);
}

void readEncoder() {
  long newPosition = myEnc.read();
  long newPositionDiv4 = newPosition / 4;
  if ((newPosition % 4 == 0) and (newPositionDiv4 != oldPosition)) {
    if (oldPosition > newPositionDiv4) {
      if (mode >= 4) {
        if (modeOverflowCnt > 2) {
          mode = 0;
          Serial.println("mode overflow");
          modeOverflowCnt = 0;
        }
        else {
          mode = 4;
          modeOverflowCnt++;
        }
      }
      else {
        mode++;
        modeOverflowCnt = 0;
      }
      Serial.println(mode);
      pwmMode();
    }
    else if (oldPosition < newPositionDiv4) {
      if (mode <= 0) {
        if (modeOverflowCnt > 2) {
          mode = 4;
          Serial.println("mode overflow");
          modeOverflowCnt = 0;
        }
        else {
          mode = 0;
          modeOverflowCnt++;
        }
      }
      else {
        mode--;
        modeOverflowCnt = 0;
      }
      Serial.println(mode);
      pwmMode();
    }
    else {
      Serial.println("error");
    }
    oldPosition = newPositionDiv4;

  }
}

void pwmSW() {
  if (out == true) {
    setduty(pwmdutyper);
  }
  else {
    setduty(0);
  }
}

void pwmMode() {
  lcd.setCursor(11, 0);
  switch (mode) {
    case 0:
      pwmdutyper = 0;
      lcd.print("  0");
      pwmSW();
      break;
    case 1:
      pwmdutyper = 25;
      lcd.print(" 25");
      pwmSW();
      break;
    case 2:
      pwmdutyper = 50;
      lcd.print(" 50");
      pwmSW();
      break;
    case 3:
      pwmdutyper = 75;
      lcd.print(" 75");
      pwmSW();
      break;
    case 4:
      pwmdutyper = 99;
      lcd.print("100");
      pwmSW();
      break;
    default :
      break;
  }
}
