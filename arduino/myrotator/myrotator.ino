
#include "Controller.h"


Controller controllerAzm = Controller(Controller::Azimuth);
Controller controllerElv = Controller(Controller::Elevation);

int ms = 100;

unsigned long t0 = 0;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(UART_SPEED);
    controllerAzm.begin();
    controllerElv.begin();    

    t0 = millis();
  
    digitalWrite(LED_BUILTIN, HIGH);
}

void timerEvent() {
    Serial.print("state:");
    Serial.print(controllerAzm.getMotor()->getCurrentSensorValue());
    Serial.print(",");
    Serial.print(controllerElv.getMotor()->getCurrentSensorValue());
    Serial.print(",");
    Serial.print(controllerAzm.getMotor()->getEnDiagValue());
    Serial.print(",");
    Serial.print(controllerElv.getMotor()->getEnDiagValue());
    Serial.print(",");
    Serial.print(controllerAzm.getMotor()->getPwm());
    Serial.print(",");
    Serial.print(controllerElv.getMotor()->getPwm());
    Serial.print(",");
    Serial.print(controllerAzm.getMotor()->getInaValue());
    Serial.print(",");
    Serial.print(controllerAzm.getMotor()->getInbValue());
    Serial.print(",");
    Serial.print(controllerElv.getMotor()->getInaValue());
    Serial.print(",");
    Serial.print(controllerElv.getMotor()->getInbValue());

    Serial.print(",");
    Serial.print(controllerAzm.getSensor()->getAngle());
    Serial.print(",");
    Serial.print(controllerElv.getSensor()->getAngle());


    Serial.print(",");
    Serial.print(controllerAzm.getEndstop()->isEnd());
    Serial.print(",");
    Serial.print(controllerElv.getEndstop()->isEnd());

    Serial.println();

    controllerAzm.execute();
    controllerElv.execute();
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
                controllerAzm.setMode(Controller::Default);
            }
            else {
                controllerAzm.getMotor()->setMotion(value);
            }
        }
        else if (line.startsWith("motion2")) {
            line = line.substring(7);
            line.trim();
            long value = line.toInt();
            if (value == 0) {
                controllerElv.setMode(Controller::Default);
            }
            else {
                controllerElv.getMotor()->setMotion(value);
            }
        }
        else if (line.startsWith("pid1")) {
            controllerAzm.setMode(Controller::Pid);
        }
        else if (line.startsWith("pid2")) {
            controllerElv.setMode(Controller::Pid);
        }
        else if (line.startsWith("homing1")) {
            controllerAzm.setMode(Controller::Homing);
        }
        else if (line.startsWith("homing2")) {
            controllerElv.setMode(Controller::Homing);
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
        else if (line.startsWith("pwm_homing1")) {
            line = line.substring(11);
            line.trim();
            controllerAzm.getMotor()->setPwmHoming(line.toInt());
        }
        else if (line.startsWith("pwm_homing2")) {
            line = line.substring(11);
            line.trim();
            controllerElv.getMotor()->setPwmHoming(line.toInt());
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
