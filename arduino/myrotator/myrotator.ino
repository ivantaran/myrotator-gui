
#include "Controller.h"


static MyMotor motor[2] = {
    MyMotor(PIN_INA1, PIN_INB1, PIN_CS1, PIN_EN1, PIN_PWM1), 
    MyMotor(PIN_INA2, PIN_INB2, PIN_CS2, PIN_EN2, PIN_PWM2), 
};

static Endstop endstop[2] = {
    Endstop(10), 
    Endstop(11),
};

static As5601 sensor[2] = {
    As5601(As5601::Hardware),
    As5601(As5601::Software),
};

static Controller controller[2] = {
    Controller(&motor[0], &endstop[0], &sensor[0]), 
    Controller(&motor[1], &endstop[1], &sensor[1]), 
};

int ms = 100;

unsigned long t0 = 0;
const char *delimeters = " ,";

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

typedef enum {
    CommandGet, 
    CommandSet, 
    CommandMotion, 
    CommandDefault,  
    CommandPid, 
    CommandHoming, 
    CommandTarget, 
    CommandResetError,
    CommandConfig
} command_e;

const char* command_table[] {
    "get"     ,
    "set"     ,
    "motion"  ,
    "default" ,
    "pid"     ,
    "homing"  ,
    "target"  ,
    "reseterr",
    "config"  ,
};

void sendConfig() {
    Serial.print(command_table[CommandConfig]);
    Serial.print(':');
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

void acceptConfig(size_t addr, char *string){
    int value;
    uint8_t i;
    for (i = 0 ; i < 6; i++) {
        value = atoi(string);
        switch (i) {
        case 0:
            controller[addr].getMotor()->setPwmHoming(value);
            break;
        case 1:
            controller[addr].getMotor()->setPwmMin(value);
            break;
        case 2:
            controller[addr].getMotor()->setPwmMax(value);
            break;
        case 3:
            controller[addr].setAngleMin(value);
            break;
        case 4:
            controller[addr].setAngleMax(value);
            break;
        case 5:
            controller[addr].setTolerance(value);
            break;
        default:
            break;
        }
        string = strtok(NULL, delimeters);
        if (string == NULL) {
            return;
        }
    }
}

void acceptPid(size_t addr, char *string){
    int value;
    uint8_t i;
    for (i = 0 ; i < 3; i++) {
        value = atoi(string);
        switch (i) {
        case 0:
            controller[addr].setKp(value);
            break;
        case 1:
            controller[addr].setKi(value);
            break;
        case 2:
            controller[addr].setKd(value);
            break;
        default:
            break;
        }
        string = strtok(NULL, delimeters);
        if (string == NULL) {
            return;
        }
    }
}

void acceptCommand(char *buffer) {
    size_t addr;
    int value;
    char *string = buffer;

    string = strtok(string, delimeters);
    if (string == NULL) {
        return;
    }

    if (strstr(string, command_table[CommandSet]) != NULL) {
        string = strtok(NULL, delimeters);

        if (string == NULL) {
            return;
        }
        else if (string[0] == '0') {
            addr = 0;
        }
        else if (string[0] == '1') {
            addr = 1;
        }
        else {
            return;
        }

        string = strtok(NULL, delimeters);
        if (string == NULL) {
            return;
        }
        else if (strstr(string, command_table[CommandConfig]) != NULL) {
            string = strtok(NULL, delimeters);
            if (string != NULL) {
                acceptConfig(addr, string);
            }
            sendConfig();
        }
        else if (strstr(string, command_table[CommandDefault]) != NULL) {
            controller[addr].setMode(Controller::ModeDefault);
        }
        else if (strstr(string, command_table[CommandPid]) != NULL) {
            string = strtok(NULL, delimeters);
            if (string != NULL) {
                acceptPid(addr, string);
                controller[addr].setMode(Controller::ModePid);
            }
        }
        else if (strstr(string, command_table[CommandHoming]) != NULL) {
            controller[addr].setMode(Controller::ModeHoming);
        }
        else if (strstr(string, command_table[CommandTarget]) != NULL) {
            string = strtok(NULL, delimeters);
            if (string != NULL) {
                value = atoi(string);
                controller[addr].setTarget(value);
            }
        }
        else if (strstr(string, command_table[CommandMotion]) != NULL) {
            string = strtok(NULL, delimeters);
            if (string != NULL) {
                value = atoi(string);
                controller[addr].getMotor()->setMotion(value);
            }
        }
        else if (strstr(string, command_table[CommandResetError]) != NULL) {
            controller[addr].resetError();
        }
    }
    else if (strstr(string, command_table[CommandGet]) != NULL) {
        sendConfig();
    }
}

void acceptSerial() {
    static char buffer[64];
    static unsigned char pointer = 0;
    
    if (Serial.available() > 0) {
        buffer[pointer] = (char)Serial.read();

        if (buffer[pointer] == '\n') {
            buffer[pointer] = '\0';
            acceptCommand(buffer);
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
    
    acceptSerial();
}
