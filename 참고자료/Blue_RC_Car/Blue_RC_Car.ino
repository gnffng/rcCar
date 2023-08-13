//초음파 핀번호
#define trigPin1 8 // 초음파1 Trigger Pin
#define echoPin1 9 // 초음파1 Echo Pin
#define trigPin2 7 // 초음파2 Trigger Pin
#define echoPin2 10 // 초음파2 Echo Pin

//초음파 최대-최소거리 설정
#define maximumRange   30   // 최대 거리 30cm
#define minimumRange   0    // 최소 거리 0cm

//모터 핀번호
#define MOTOR_A_a 3     //모터A의 +출력핀은 3번핀입니다
#define MOTOR_A_b 11    //모터A의 -출력핀은 11번핀입니다

#define MOTOR_B_a 5     //모터B의 +출력핀은 5번핀입니다
#define MOTOR_B_b 6     //모터B의 -출력핀은 6번핀입니다

//모터 기준속력
#define MOTOR_SPEED_A 255 //모터의 기준속력입니다(0~255)
#define MOTOR_SPEED_B 255 //모터의 기준속력입니다(0~255)

//LED 핀번호
#define CrushLed 4

//초음파 변수
const int system_time = 50;  // 전체 시스템 시간 
const int ultrasonic_time = 10;  // 초음파 측정 시간 
long duration1, distance1, duration2, distance2;  // 펄스 시간, 거리 측정용 변수
long distance1_a, distance2_a;                   // distance를 측정하기 위한 계산용 변수
int ultrasonic_count = 3;           // 초음파 Count만큼 감지 후 평균 값을 센싱함
const long distanse_set = 20;       // 초음파 사람 감지 세팅 값 - 해당 거리보다 가까울 경우에 물체를 인지함

//모터 제어 변수
unsigned char m_a_spd = 0, m_b_spd = 0; //모터의 속력을 저장하는 전역변수
boolean m_a_dir = 0, m_b_dir = 0;       //모터의 방향을 결정하는 전역변수

//시스템 변수
unsigned char bt_cmd = 's';   //시리얼 통신을 통해 문자를 받아올 변수
bool isStop = false, isCrush = false;   //전방에 물체가 있거나(isStop) 후방에 충돌이 있었는지를(isCrush) 판단하는 변수

void setup()
{
  //제어 핀 설정
  Serial.begin(9600);       //시리얼 통신 초기화

  //모터 핀 : 출력모드로 설정
  pinMode(MOTOR_A_a, OUTPUT);
  pinMode(MOTOR_A_b, OUTPUT);
  pinMode(MOTOR_B_a, OUTPUT);
  pinMode(MOTOR_B_b, OUTPUT);

  //초음파 핀 : trig핀은 출력, echo핀은 입력으로 설정
  pinMode(trigPin1, OUTPUT); 
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  //LED핀 : 출력모드로 설정
  pinMode(CrushLed, OUTPUT);
  digitalWrite(CrushLed, false);   //처음 켜졌을 때 LED가 꺼지도록 초기화
}

void loop()
{
  //초음파 거리 변수 초기화
  distance1_a = 0;
  distance2_a = 0;

  delay(system_time);   //각 센서들이 정상적으로 초기화될 수 있도록 delay 시킴

  //초음파를 ultrasonic_count 횟수만큼 감지하여 distance1_a에 합산함
  for(int i=1; i <= ultrasonic_count; i++){
    //trigPin을 통해 pulse신호를 만들어 보냄
    digitalWrite(trigPin1, LOW); 
    delayMicroseconds(2);  
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);  
    digitalWrite(trigPin1, LOW);

    duration1 = pulseIn(echoPin1, HIGH);   //echoPin을 통해 펄스신호가 반환되기까지 걸리는 시간을 측정함
    distance1 = duration1/58.2;   //측정된 시간을 cm로 환산
    distance1_a = distance1_a + distance1;   //distance1_a에 측정된 distance값을 더해줌
    delay(ultrasonic_time);   //초음파 측정 시간동안 delay시킴
  }

  //초음파를 ultrasonic_count 횟수만큼 감지하여 distance2_a에 합산함
  for(int i=1; i <= ultrasonic_count; i++){
    //trigPin을 통해 pulse신호를 만들어 보냄
    digitalWrite(trigPin2, LOW); 
    delayMicroseconds(2);  
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10);  
    digitalWrite(trigPin2, LOW);
    
    duration2 = pulseIn(echoPin2, HIGH);   //echoPin을 통해 펄스신호가 반환되기까지 걸리는 시간을 측정함
    distance2 = duration2/58.2;   //측정된 시간을 cm로 환산
    distance2_a = distance2_a + distance2;   //distance2_a에 측정된 distance값을 더해줌
    delay(ultrasonic_time);   //초음파 측정 시간동안 delay시킴
  }

  //감지한 데이터의 평균 값을 산출 한다. 
  distance1= long(distance1_a/ultrasonic_count);
  distance2= long(distance2_a/ultrasonic_count);

  //거리값을 통한 제어(초음파1)
  if(distance1 < distanse_set ){   //거리가 distanse_set보다 적으면
    isStop = true;   //긴급정지 상태로 변경
  }

  //거리값을 통한 제어(초음파2)
  if(distance2 < 5){   //거리가 5cm보다 적으면
    isCrush = true;   //충돌상태로 변경
  }

  //블루투스 통신을 통해 데이터가 입력되었을 때
  if (Serial.available()){
    bt_cmd = Serial.read();   //블루투스로 입력받은 문자를 저장
  }

  //긴급정지 상태일때
  if(isStop){
    //입력받은 문자가 q,w,e,a,d 이라면(좌회전, 전진, 우회전, 제자리좌회전, 제자리우회전) 's'(정지)로 바꾼다
    if(bt_cmd=='q' || bt_cmd=='w' || bt_cmd=='e' || bt_cmd=='a' || bt_cmd=='d'){
      bt_cmd = 's';
    }
  }

  //충돌상태일 경우
  if(isCrush){
    //bt_cmd(블루투스를 통해 입력받은 문자)가 'k'나 'j'가 아니면 bt_cmd를 's'(정지)로 바꾼다
    if(bt_cmd != 'k' && bt_cmd != 'j'){
      bt_cmd = 's';
    }
  }

  //충돌상태거나 긴급상태일때 긴급등을 켬
  if(isCrush || isStop){
    digitalWrite(CrushLed, true);
  }
  else{
    digitalWrite(CrushLed, false);
  }
  
  rc_ctrl_val(bt_cmd);    //bt_cmd 값에 따라 모터 제어변수를 설정함 
  motor_drive();   //모터를 구동하는 함수
}

void rc_ctrl_val(unsigned char cmd) //입력된 데이터에 따라 모터에 입력될 변수를 조정하는 함수
{
  if(cmd == 'w')  //'w'가 입력되었을 때, 전진
  {
    m_a_dir = 0;  //모터A 정방향
    m_b_dir = 0;  //모터B 정방향
    m_a_spd = MOTOR_SPEED_A*(0.8);  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED_B;  //모터B의 속력값 조정
  }
  else if(cmd == 'a')  //'a'가 입력되었을 때, 제자리 좌회전
  {
    m_a_dir = 1;  //모터A 역방향
    m_b_dir = 0;  //모터B 정방향
    m_a_spd = MOTOR_SPEED_A;  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED_B;  //모터B의 속력값 조정
  }
  else if(cmd == 'q')  //'q'가 입력되었을 때, 직전 좌회전
  {
    m_a_dir = 0;  //모터A 정방향
    m_b_dir = 0;  //모터B 정방향
    m_a_spd = MOTOR_SPEED_A/2;  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED_B;  //모터B의 속력값 조정
  }
  else if(cmd == 'd')  //'d'가 입력되었을 때, 제자리 우회전
  {
    m_a_dir = 0;  //모터A 정방향
    m_b_dir = 1;  //모터B 역방향
    m_a_spd = MOTOR_SPEED_A;  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED_B;  //모터B의 속력값 조정
  }
  else if(cmd == 'e')  //'a'가 입력되었을 때, 제자리 좌회전
  {
    m_a_dir = 0;  //모터A 정방향
    m_b_dir = 0;  //모터B 정방향
    m_a_spd = MOTOR_SPEED_A;  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED_B/2;  //모터B의 속력값 조정
  }
  else if(cmd == 'x')  //'s'가 입력되었을 때, 후진
  {
    m_a_dir = 1;  //모터A 역방향
    m_b_dir = 1;  //모터B 역방향
    m_a_spd = MOTOR_SPEED_A;  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED_B;  //모터B의 속력값 조정
  }
  else if(cmd == 'z')  //'s'가 입력되었을 때, 후진
  {
    m_a_dir = 1;  //모터A 역방향
    m_b_dir = 1;  //모터B 역방향
    m_a_spd = MOTOR_SPEED_A/2;  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED_B;  //모터B의 속력값 조정
  }
  else if(cmd == 'c')  //'s'가 입력되었을 때, 후진
  {
    m_a_dir = 1;  //모터A 역방향
    m_b_dir = 1;  //모터B 역방향
    m_a_spd = MOTOR_SPEED_A;  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED_B/2;  //모터B의 속력값 조정
  }
  else if(cmd == 's')
  {
    m_a_dir = 0;  //모터A 정방향
    m_b_dir = 0;  //모터B 정방향
    m_a_spd = 0;  //모터A의 정지
    m_b_spd = 0;  //모터B의 정지
  }
  else if(cmd == 'j')   //입력된 값이 j일 경우 긴급정지상태를 해제함
  {
    isStop = false;
  }
  else if(cmd == 'k')   //입력된 값이 k일 경우 충돌상태를 해제하고 LED를 끔
  {
    isCrush = false;
  }
}

void motor_drive()  //모터를 구동하는 함수
{
  //모터A 구동
  if(m_a_dir == 0)   //정방향=0, 역방향=1
  {
    digitalWrite(MOTOR_A_a, LOW);     //모터A+ LOW
    analogWrite(MOTOR_A_b, m_a_spd);  //모터A-의 속력을 PWM 출력
  }
  else   
  {
    analogWrite(MOTOR_A_a, m_a_spd);  //모터A+의 속력을 PWM 출력
    digitalWrite(MOTOR_A_b, LOW);     //모터A- LOW
  }
  
  if(m_b_dir == 1)   //정방향=1, 역방향=0
  {
    digitalWrite(MOTOR_B_a, LOW);     //모터B+ LOW
    analogWrite(MOTOR_B_b, m_b_spd);  //모터B-의 속력을 PWM 출력
  }
  else
  {
    analogWrite(MOTOR_B_a, m_b_spd);  //모터B+의 속력을 PWM 출력
    digitalWrite(MOTOR_B_b, LOW);     //모터B- LOW
  }
}
