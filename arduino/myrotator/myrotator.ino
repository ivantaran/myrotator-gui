
#include "MyMotor.h"
#include "As5601.h"
#include "Endstop.h"

MyMotor motorAzm = MyMotor(PIN_INA1, PIN_INB1, PIN_CS1, PIN_EN1, PIN_PWM1);
MyMotor motorElv = MyMotor(PIN_INA2, PIN_INB2, PIN_CS2, PIN_EN2, PIN_PWM2);

As5601 sensorAzm = As5601(As5601::Hardware);
As5601 sensorElv = As5601(As5601::Software);

Endstop endstopAzm = Endstop(10);
Endstop endstopElv = Endstop(11);

int ms = 100;

unsigned long t0 = 0;

bool controller_enabled = false;
bool moveToHome = false;

long ctrl_ki = 1000;
long ctrl_kp = 30;
long ctrl_angle = 0;

bool homing() {
    static bool isEndAzm = false;

    if (endstopAzm.isEnd()) {
        motorAzm.setMotion(0);
        isEndAzm = true;
    } else if (!isEndAzm) {
        motorAzm.setMotion(motorAzm.getPwmHoming());
    }

    return isEndAzm;
}

void controller(void) {
    long error1 = 0;
    long amp = 1000;
    static long error0 = 0;
    static long value = 0;
    
    if (!controller_enabled || sensorAzm.isValid()) {
        error0 = 0;
        error1 = 0;
        value = 0;
        return;
    }
    
    error1 = error0;
    error0 = ctrl_angle - sensorAzm.getAngle();
    
    value -= ((ctrl_ki + ctrl_kp) * error0 - ctrl_ki * error1);
    
    if (value > 511 * amp) {
        value = 511 * amp;
    }
    else if (value < -511 * amp) {
        value = -511 * amp;
    }

    motorAzm.setMotion(value / amp);
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
    
    motorAzm.begin();
    motorElv.begin();
    
    endstopAzm.begin();
    endstopElv.begin();

    sensorAzm.begin();
    sensorElv.begin();

    t0 = millis();
  
    digitalWrite(LED_BUILTIN, HIGH);
}

void timerEvent() {
    Serial.print("state:");
    Serial.print(motorAzm.getCurrentSensorValue());
    Serial.print(",");
    Serial.print(motorElv.getCurrentSensorValue());
    Serial.print(",");
    Serial.print(motorAzm.getEnDiagValue());
    Serial.print(",");
    Serial.print(motorElv.getEnDiagValue());
    Serial.print(",");
    Serial.print(motorAzm.getPwm());
    Serial.print(",");
    Serial.print(motorElv.getPwm());
    Serial.print(",");
    Serial.print(motorAzm.getInaValue());
    Serial.print(",");
    Serial.print(motorAzm.getInbValue());
    Serial.print(",");
    Serial.print(motorElv.getInaValue());
    Serial.print(",");
    Serial.print(motorElv.getInbValue());

    Serial.print(",");
    Serial.print(sensorAzm.getAngle());
    Serial.print(",");
    Serial.print(sensorElv.getAngle());


    Serial.print(",");
    Serial.print(endstopAzm.isEnd());
    Serial.print(",");
    Serial.print(endstopElv.isEnd());

    Serial.println();

    sensorAzm.requestSensorValue();
    sensorElv.requestSensorValue();

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
            if (value == 0) {
                controller_enabled = false;
            }
            motorAzm.setMotion(value);
        }
        else if (line.startsWith("motion2")) {
            line = line.substring(7);
            line.trim();
            long value = line.toInt();
            if (value == 0) {
                controller_enabled = false;
            }
            motorElv.setMotion(value);
        }
        else if (line.startsWith("con")) {
            controller_enabled = !controller_enabled;
        }
        else if (line.startsWith("homing")) {
            moveToHome = !moveToHome;
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
    
    if (moveToHome) {
        moveToHome = !homing();
    }

    accept_serial();
}
