
#include "MyMotor.h"
#include "As5601.h"
#include "Endstop.h"
#include "Controller.h"

MyMotor motorAzm = MyMotor(PIN_INA1, PIN_INB1, PIN_CS1, PIN_EN1, PIN_PWM1);
MyMotor motorElv = MyMotor(PIN_INA2, PIN_INB2, PIN_CS2, PIN_EN2, PIN_PWM2);

As5601 sensorAzm = As5601(As5601::Hardware);
As5601 sensorElv = As5601(As5601::Software);

Endstop endstopAzm = Endstop(10);
Endstop endstopElv = Endstop(11);

Controller controllerAzm = Controller();
Controller controllerElv = Controller();

int ms = 100;

unsigned long t0 = 0;

bool controller_enabled = false;
bool moveToHome = false;

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

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(UART_SPEED);
    
    motorAzm.begin();
    motorElv.begin();
    
    endstopAzm.begin();
    endstopElv.begin();

    sensorAzm.begin();
    sensorElv.begin();
    
    controllerAzm.setMode(Controller::Pid);
    controllerElv.setMode(Controller::Pid);

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

    if (controller_enabled) {
        if (!sensorAzm.isValid()) {
            motorAzm.setMotion(controllerAzm.pid(sensorAzm.getAngle()));
        }
        if (!sensorElv.isValid()) {
            motorElv.setMotion(controllerElv.pid(sensorElv.getAngle()));
        }
        Serial.print(controllerAzm.getMode());
        Serial.print(",");
        Serial.print(controllerAzm.getKp());
        Serial.print(",");
        Serial.print(controllerAzm.getKi());
        Serial.print(",");
        Serial.print(controllerAzm.getKd());
        Serial.print(",");
        Serial.print(controllerElv.getMode());
        Serial.print(",");
        Serial.print(controllerElv.getKp());
        Serial.print(",");
        Serial.print(controllerElv.getKi());
        Serial.print(",");
        Serial.print(controllerElv.getKd());
        Serial.println();
    }
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
            controllerAzm.resetPid();
            controllerElv.resetPid();
            controller_enabled = !controller_enabled;
        }
        else if (line.startsWith("homing")) {
            moveToHome = !moveToHome;
        }
        else if (line.startsWith("ctrl1_kp")) {
            line = line.substring(8);
            line.trim();
            controllerAzm.setKp(line.toInt());
        }
        else if (line.startsWith("ctrl1_ki")) {
            line = line.substring(8);
            line.trim();
            controllerAzm.setKi(line.toInt());
        }
        else if (line.startsWith("ctrl1_kd")) {
            line = line.substring(8);
            line.trim();
            controllerAzm.setKd(line.toInt());
        }
        else if (line.startsWith("ctrl1_target")) {
            line = line.substring(12);
            line.trim();
            controllerAzm.setTarget(line.toInt());
        }
        else if (line.startsWith("ctrl2_kp")) {
            line = line.substring(8);
            line.trim();
            controllerElv.setKp(line.toInt());
        }
        else if (line.startsWith("ctrl2_ki")) {
            line = line.substring(8);
            line.trim();
            controllerElv.setKi(line.toInt());
        }
        else if (line.startsWith("ctrl2_kd")) {
            line = line.substring(8);
            line.trim();
            controllerElv.setKd(line.toInt());
        }
        else if (line.startsWith("ctrl2_target")) {
            line = line.substring(12);
            line.trim();
            controllerElv.setTarget(line.toInt());
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
