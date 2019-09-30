
#include "Controller.h"


Controller controller[2] = {
    Controller(Controller::Azimuth), 
    Controller(Controller::Elevation)
};

int ms = 100;

unsigned long t0 = 0;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(UART_SPEED);
    controller[0].begin();
    controller[1].begin();    

    t0 = millis();
  
    digitalWrite(LED_BUILTIN, HIGH);
}

void timerEvent() {
    Serial.print("state:");
    
    for (uint8_t i = 0; i < 2; i++) {
        Serial.print(static_cast<uint8_t>(controller[i].getMode()));
        Serial.print(",");
        Serial.print(static_cast<uint8_t>(controller[i].getError()));
        Serial.print(",");
        Serial.print(controller[i].getMotor()->getCurrentSensorValue());
        Serial.print(",");
        Serial.print(controller[i].getMotor()->getEnDiagValue());
        Serial.print(",");
        Serial.print(controller[i].getMotor()->getPwm());
        Serial.print(",");
        Serial.print(controller[i].getMotor()->getInaValue());
        Serial.print(",");
        Serial.print(controller[i].getMotor()->getInbValue());
        Serial.print(",");
        Serial.print(controller[i].getSensor()->getAngle());
        Serial.print(",");
        Serial.print(controller[i].getEndstop()->isEnd());
        Serial.print(",");
    }

    Serial.println();

    controller[0].execute();
    controller[1].execute();
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
            controller[0].getMotor()->setMotion(value);
        }
        else if (line.startsWith("motion2")) {
            line = line.substring(7);
            line.trim();
            long value = line.toInt();
            controller[1].getMotor()->setMotion(value);
        }
        else if (line.startsWith("pid1")) {
            controller[0].setMode(Controller::Pid);
        }
        else if (line.startsWith("pid2")) {
            controller[1].setMode(Controller::Pid);
        }
        else if (line.startsWith("homing1")) {
            controller[0].setMode(Controller::Homing);
        }
        else if (line.startsWith("homing2")) {
            controller[1].setMode(Controller::Homing);
        }
        else if (line.startsWith("ctrl1_kp")) {
            line = line.substring(8);
            line.trim();
            controller[0].setKp(line.toInt());
        }
        else if (line.startsWith("ctrl1_ki")) {
            line = line.substring(8);
            line.trim();
            controller[0].setKi(line.toInt());
        }
        else if (line.startsWith("ctrl1_kd")) {
            line = line.substring(8);
            line.trim();
            controller[0].setKd(line.toInt());
        }
        else if (line.startsWith("ctrl1_target")) {
            line = line.substring(12);
            line.trim();
            controller[0].setTarget(line.toInt());
        }
        else if (line.startsWith("ctrl2_kp")) {
            line = line.substring(8);
            line.trim();
            controller[1].setKp(line.toInt());
        }
        else if (line.startsWith("ctrl2_ki")) {
            line = line.substring(8);
            line.trim();
            controller[1].setKi(line.toInt());
        }
        else if (line.startsWith("ctrl2_kd")) {
            line = line.substring(8);
            line.trim();
            controller[1].setKd(line.toInt());
        }
        else if (line.startsWith("ctrl2_target")) {
            line = line.substring(12);
            line.trim();
            controller[1].setTarget(line.toInt());
        }
        else if (line.startsWith("pwm_homing1")) {
            line = line.substring(11);
            line.trim();
            controller[0].getMotor()->setPwmHoming(line.toInt());
        }
        else if (line.startsWith("pwm_homing2")) {
            line = line.substring(11);
            line.trim();
            controller[1].getMotor()->setPwmHoming(line.toInt());
        }
        else if (line.startsWith("reset_error1")) {
            controller[0].resetError();
            Serial.println("OLOLO1");
        }
        else if (line.startsWith("reset_error2")) {
            controller[1].resetError();
            Serial.print("OLOLO2");
        }
    }
    else if (line.startsWith("get")) {
        line = line.substring(3);
        line.trim();
        if (line.startsWith("config")) {
            Serial.print("config:");
            for (uint8_t i = 0; i < 2; i++) {
                Serial.print(controller[i].getMotor()->getPwmHoming());
                Serial.print(",");
                Serial.print(controller[i].getMotor()->getPwmMin());
                Serial.print(",");
                Serial.print(controller[i].getMotor()->getPwmMax());
                Serial.print(",");
                Serial.print(controller[i].getAngleMin());
                Serial.print(",");
                Serial.print(controller[i].getAngleMax());
                Serial.print(",");
                Serial.print(controller[i].getTolerance());
                Serial.print(",");
            }
            Serial.println();
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
