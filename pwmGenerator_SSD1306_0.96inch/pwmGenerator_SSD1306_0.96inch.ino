//oled 1306 final: logo X
#include <PWM.h>
#include "U8glib.h"
#define CLK 2    // 2번핀을 CLK에 연결
#define DT 3     // 3번핀을 DT에 연결


int counter = 0;   // 카운팅 저장용 변수
int currentStateCLK;     // 현재 CLK의 상태 저장용 변수
int lastStateCLK;         // 이전 CLK의 상태 저장용 변수
String currentDir = "";    // 현재 방향 출력용 문자열 변수

int sw = 4;

int mode = 0;

int pwm = 9;
int32_t frequency = 241; //frequency (in Hz)

int pwmdutyper = 0;

bool out = false;

//OLED
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 |
                          U8G_I2C_OPT_NO_ACK |
                          U8G_I2C_OPT_FAST);


void setup() {

  // 엔코더 핀을 입력으로 설정
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(sw, INPUT);

  InitTimersSafe();
  bool success = SetPinFrequencySafe(pwm, frequency);
  if (success) {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
  }

  // 시리얼 출력 개시
  Serial.begin(9600);

  // CLK핀의 최초 상태 저장
  lastStateCLK = digitalRead(CLK);

  //외부 인터럽트 등록, 핀의 상태가 변할 때(HIGH에서 LOW 또는 LOW에서 HIGH) 마다 updateEncoder함수가 실행됨.
  // 인터럽트 0번은 2번핀과 연결되어 있고 1번은 3번 핀과 연결되어 있음
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

  //OLED
  u8g.setFont(u8g_font_unifont);
  u8g.setColorIndex(1);
}

void loop() {
  oledRun();
  switch (mode) {
    case 0:
      pwmdutyper = 0;
      break;
    case 1:
      pwmdutyper = 25;
      break;
    case 2:
      pwmdutyper = 50;
      break;
    case 3:
      pwmdutyper = 75;
      break;
    case 4:
      pwmdutyper = 100;
      break;
  }

  if (digitalRead(sw) == LOW) {
    if (out == false) {
      setduty(pwmdutyper);
      out = true;
      Serial.print("ON & duty : ");
      Serial.println(pwmdutyper);

    }
    else {
      setduty(0);
      out = false;
      Serial.println("OFF");
    }
    while (1) {
      if (digitalRead(sw) == HIGH)
        break;
    }
  }

}

int setduty(int dutyper) {
  int duty = map(dutyper, 0, 100, 0, 255);
  pwmWrite(pwm, duty);
}


void updateEncoder() {  // 인터럽트 발생시 실행되는 함수
  // CLK의 현재 상태를 읽어서
  currentStateCLK = digitalRead(CLK);

  // CLK핀의 신호가 바뀌었고(즉, 로터리엔코더의 회전이 발생했했고), 그 상태가 HIGH이면(최소 회전단위의 회전이 발생했다면)
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {

    // DT핀의 신호를 확인해서 엔코더의 회전 방향을 확인함.
    if (digitalRead(DT) != currentStateCLK) {    // 신호가 다르다면 시계방향 회전
      counter ++;                                // 카운팅 용 숫자 1 증가
      if (out == false) {
        if (mode >= 4) mode = 0;
        else mode++;
        Serial.println(mode);
      }
      currentDir = "시계방향 회전";
    } else {                                   // 신호가 같다면 반시계방향 회전
      counter --;                              // 카운팅 용 숫자 1 감소
      if (out == false) {
        if (mode == 0) mode = 4;
        else mode--;
        Serial.println(mode);
      }
      currentDir = "반시계 방향 회전";
    }



    //    Serial.print("회전방향: ");
    //    Serial.print(currentDir);               // 회전방향 출력
    //    Serial.print(" | Counter: ");
    //    Serial.println(counter);              // 카운팅 출력
  }

  // 마지막 상태 변수 저장
  lastStateCLK = currentStateCLK;
}

//OLED문자 출력 함수

void oledRun() {
  u8g.firstPage();
  do {
    draw();
  } while ( u8g.nextPage() );
  delay(10);
}

void draw()
{
  //Frequencyh
  u8g.drawStr( 0, 20, "Hz :");
  u8g.setPrintPos(50, 20);
  u8g.print(frequency - 1);

  //DUTY
  u8g.drawStr(0, 40, "Duty :");
  u8g.setPrintPos(60, 40);
  u8g.print(pwmdutyper);


  //ON/OFF
  u8g.drawStr(0, 60, "OUTPUT : ");
  if (out == 0) {
    u8g.setPrintPos(65, 60);
    u8g.print("OFF");

  }
  else if (out == 1) {

    u8g.setPrintPos(65, 60);
    u8g.print("ON");

  }
}
