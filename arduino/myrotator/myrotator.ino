
#include "myrotator.h"

SoftWire softWire = SoftWire();
MyMotor motor1 = MyMotor(PIN_INA1, PIN_INB1, PIN_CS1, PIN_EN1, PIN_PWM1);
MyMotor motor2 = MyMotor(PIN_INA2, PIN_INB2, PIN_CS2, PIN_EN2, PIN_PWM2);

int ms = 100;
int angle1 = -1;
int angle2 = -1;

unsigned long t0 = 0;

bool controller_enabled = false;
long ctrl_ki = 1000;
long ctrl_kp = 20;
long ctrl_angle = 2048;

void controller(void) {
    long error1 = 0;
    long amp = 1000;
    static long error0 = 0;
    static long value = 0;
    
    if (!controller_enabled || angle1 < 0) {
        error0 = 0;
        error1 = 0;
        value = 0;
        return;
    }
    
    error1 = error0;
    error0 = ctrl_angle - angle1;
    
    if (error0 > 2047) {
        error0 -= 4096;
    }
    else if (error0 < -2047) {
        error0 += 4096;
    }

    value -= ((ctrl_ki + ctrl_kp) * error0 - ctrl_ki * error1);
    
    // if (value > 511 * amp) {
    //     value = 511 * amp;
    // }
    // else if (value < -511 * amp) {
    //     value = -511 * amp;
    // }
    // else if (value < 50 * amp && value > 0) {
    //     if (abs(error0) > 4) {
    //         value = 50 * amp;
    //     }
    //     else {
    //         value = 0;
    //     }
    // }
    // else if (value > -50 * amp && value < 0) {
    //     if (abs(error0) > 4) {
    //         value = -50 * amp;
    //     }
    //     else {
    //         value = 0;
    //     }
    // }

    motor1.setMotion(value / amp);
    Serial.print(value / amp);
    Serial.print(',');
    Serial.print(ctrl_kp);
    Serial.print(',');
    Serial.print(ctrl_ki);
    Serial.print(',');
    Serial.println(error0);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(UART_SPEED);
    Wire.begin();
    softWire.begin();

    t0 = millis();
  
    digitalWrite(LED_BUILTIN, HIGH);
}

void requestSensorValue() {
  angle1 = -1;
  
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

void requestSoftSensorValue() {
  angle2 = -1;
  
  softWire.beginTransmission(I2C_SENSOR_ADDR);
  softWire.write(byte(I2C_SENSOR_ANGLE_ADDR));
  softWire.endTransmission();
  softWire.requestFrom(I2C_SENSOR_ADDR, 2);

  int value = 0;
  int n = 0;
  while (softWire.available()) {
    byte c = softWire.read();
    value = (value << 8) | c;
    n++;
  }
  
  angle2 = (n == 2) ? value : -1;
}

void timerEvent() {
    Serial.print("state:");
    Serial.print(motor1.getCurrentSensorValue());
    Serial.print(",");
    Serial.print(motor2.getCurrentSensorValue());
    Serial.print(",");
    Serial.print(motor1.getEnDiagValue());
    Serial.print(",");
    Serial.print(motor2.getEnDiagValue());
    Serial.print(",");
    Serial.print(motor1.getPwm());
    Serial.print(",");
    Serial.print(motor2.getPwm());
    Serial.print(",");
    Serial.print(motor1.getInaValue());
    Serial.print(",");
    Serial.print(motor1.getInbValue());
    Serial.print(",");
    Serial.print(motor2.getInaValue());
    Serial.print(",");
    Serial.print(motor2.getInbValue());

    Serial.print(",");
    Serial.print(angle1);
    Serial.print(",");
    Serial.print(angle2);

    Serial.println();

    requestSensorValue();
    requestSoftSensorValue();
    controller();  
}

void accept_command(const char *buffer) {
    String line(buffer);
  
    if (line.startsWith("set")) {
            line = line.substring(3);
            line.trim();
            if (line.startsWith("motion1")) {
            line = line.substring(7);
            line.trim();
            long value = line.toInt();
            motor1.setMotion(value);
        }
        else if (line.startsWith("motion2")) {
            line = line.substring(7);
            line.trim();
            long value = line.toInt();
            motor2.setMotion(value);
        }
        else if (line.startsWith("con")) {
            controller_enabled = !controller_enabled;
        }
        else if (line.startsWith("ctrl_kp")) {
            line = line.substring(7);
            line.trim();
            ctrl_kp = line.toInt();
        }
        else if (line.startsWith("ctrl_ki")) {
            line = line.substring(7);
            line.trim();
            ctrl_ki = line.toInt();
        }
        else if (line.startsWith("ctrl_angle")) {
            line = line.substring(10);
            line.trim();
            ctrl_angle = line.toInt();
        }
    }
}

void accept_serial() {
    static char buffer[64];
    static unsigned char pointer = 0;
    
    if (Serial.available() > 0) {
        buffer[pointer] = (char)Serial.read();

        if (buffer[pointer] == '\n') {
            buffer[pointer] = '\0';
            accept_command(buffer);
            pointer = 0;
        }
        else if (pointer < sizeof(buffer) - 1) {
            pointer++;
        }
        else {
            pointer = 0;
        }
    }
}

void loop() {
    if (millis() - t0 >= TIMER_PERIOD) {
        t0 += TIMER_PERIOD;
        timerEvent();
    }

    accept_serial();
}
