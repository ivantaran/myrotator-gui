

#include <Wire.h>

#define PIN_EN1   A0
#define PIN_EN2   A1
#define PIN_CS1   A2
#define PIN_CS2   A3

#define PIN_INA1  7
#define PIN_INA2  4
#define PIN_INB1  8
#define PIN_INB2  9
#define PIN_PWM1  5
#define PIN_PWM2  6

#define PWM_MASK  0xff

#define TIMER_PERIOD  200ul

/* Sensor defines */
#define UART_SPEED            115200
#define I2C_SENSOR_ADDR       0x36
#define I2C_SENSOR_ANGLE_ADDR 0x0e
/**/

int ms = 100;
int pwm1 = 0;
int pwm2 = 0;
int angle1 = -1;
int angle2 = -1;

unsigned long t0 = 0;

void motor_brake(void) {
  digitalWrite(PIN_INA1, LOW);
  digitalWrite(PIN_INA2, LOW);
  digitalWrite(PIN_INB1, LOW);
  digitalWrite(PIN_INB2, LOW);
  motor_set_pwm(0, 0);
}

void motor_move_right(void) {
  digitalWrite(PIN_INA1, HIGH);
  digitalWrite(PIN_INB1, LOW);
  digitalWrite(PIN_INA2, HIGH);
  digitalWrite(PIN_INB2, LOW);
  motor_set_pwm(0, 0);
}

void motor_move_left(void) {
  digitalWrite(PIN_INA1, LOW);
  digitalWrite(PIN_INB1, HIGH);
  digitalWrite(PIN_INA2, LOW);
  digitalWrite(PIN_INB2, HIGH);
  motor_set_pwm(0, 0);
}

void motor_set_pwm(int value1, int value2) {
  pwm1 = value1;
  pwm2 = value2;
  analogWrite(PIN_PWM1, pwm1);
  analogWrite(PIN_PWM2, pwm2);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
//  TCCR1A = TCCR1A & 0xe0 | 1;
//  TCCR1B = TCCR1B & 0xe0 | 0x0b;

//  TCCR0A = TCCR0A & 0xe0 | 1;
//  TCCR0B = TCCR0B & 0xe0 | 0x0a;  
  
//  TCCR1A = TCCR1A & 0xe0 | 1;
//  TCCR1B = TCCR1B & 0xe0 | 0x0d;
//  TCCR0B = TCCR0B & 0b11111000 | 0x04;
  
  motor_brake();
  
  pinMode(PIN_INA1, OUTPUT);
  pinMode(PIN_INA2, OUTPUT);
  pinMode(PIN_INB1, OUTPUT);
  pinMode(PIN_INB2, OUTPUT);
  pinMode(PIN_PWM1, OUTPUT);
  pinMode(PIN_PWM2, OUTPUT);
  
  motor_brake();
  
  analogWrite(PIN_PWM1, 0);
  analogWrite(PIN_PWM2, 0);
  
  Serial.begin(UART_SPEED);
  Wire.begin();

  t0 = millis();
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void requestSensorsValues() {
  angle1 = -1;
  angle2 = -1;
  
  Wire.beginTransmission(I2C_SENSOR_ADDR);
  Wire.write(byte(I2C_SENSOR_ANGLE_ADDR));
  Wire.endTransmission();
  Wire.requestFrom(I2C_SENSOR_ADDR, 2);

  int value = 0;
  int n = 0;
  while (Wire.available()) {
    byte c = Wire.read();
    value = (value << 8) | c;
    n++;
  }
  
  angle1 = (n == 2) ? value : -1;
}

void timerEvent() {
  int v1, v2;
  v1 = analogRead(PIN_CS1);
  v2 = analogRead(PIN_CS2);
  Serial.print("state:");
  Serial.print(v1);
  Serial.print(",");
  Serial.print(v2);
  Serial.print(",");
//  v1 = analogRead(PIN_EN1);
//  v2 = analogRead(PIN_EN2);
  v1 = digitalRead(PIN_EN1);
  v2 = digitalRead(PIN_EN2);
  Serial.print(v1);
  Serial.print(",");
  Serial.print(v2);
  Serial.print(",");
  Serial.print(pwm1);
  Serial.print(",");
  Serial.print(pwm2);
  Serial.print(",");
  v1 = digitalRead(PIN_INA1);
  v2 = digitalRead(PIN_INB1);
  Serial.print(v1);
  Serial.print(",");
  Serial.print(v2);
  Serial.print(",");
  v1 = digitalRead(PIN_INA2);
  v2 = digitalRead(PIN_INB2);
  Serial.print(v1);
  Serial.print(",");
  Serial.print(v2);

  Serial.print(",");
  Serial.print(angle1);
  Serial.print(",");
  Serial.print(angle2);
  
  Serial.println();
  
  requestSensorsValues();  
}

void loop() {
  int tmp, v1, v2;

  if (millis() - t0 >= TIMER_PERIOD) {
    t0 += TIMER_PERIOD;
    timerEvent();
  }
  
  if (Serial.available() > 0) {
    int c = Serial.read();
    switch (c) {
    case '+':
      tmp = pwm1 + 0x20;
      pwm1 = tmp > PWM_MASK ? PWM_MASK : tmp;
      tmp = pwm2 + 0x20;
      pwm2 = tmp > PWM_MASK ? PWM_MASK : tmp;
      break;
    case '-':
      tmp = pwm1 - 0x20;
      pwm1 = tmp < 0x00 ? 0x00 : tmp;
      tmp = pwm2 - 0x20;
      pwm2 = tmp < 0x00 ? 0x00 : tmp;
      break;
    case '0':
      pwm1 = 0x00;
      pwm2 = 0x00;
      break;
    case '1':
      pwm1 = PWM_MASK;
      pwm2 = PWM_MASK;
      break;
    case 's':
      v1 = analogRead(PIN_CS1);
      v2 = analogRead(PIN_CS2);
      Serial.print("cs: ");
      Serial.print(v1);
      Serial.print(", ");
      Serial.println(v2);
      v1 = analogRead(PIN_EN1);
      v2 = analogRead(PIN_EN2);
      Serial.print("en\\diag: ");
      Serial.print(v1);
      Serial.print(", ");
      Serial.println(v2);
      break;
    case 'b':
      motor_brake();
      break;
    case '>':
      motor_move_right();
      break;
    case '<':
      motor_move_left();
      break;
    default:
      break;
    }
    
    switch (c) {
    case '+':
    case '-':
    case '0':
    case '1':
      motor_set_pwm(pwm1, pwm2);
      break;
    default:
      break;
    }
    
  }
   
}
