
#include "Controller.h"


Controller controller[2] = {
    Controller(Controller::TypeAzimuth), 
    Controller(Controller::TypeElevation)
};

int ms = 100;

unsigned long t0 = 0;
const char *delimeters = " ";

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
    CommandPid, 
    CommandHoming, 
    CommandKp, 
    CommandKi, 
    CommandKd, 
    CommandTarget, 
    CommandResetError,
    CommandConfig
} command_e;

const char* command_table[] {
    "get"     ,
    "set"     ,
    "motion"  ,
    "pid"     ,
    "homing"  ,
    "kp"      ,
    "ki"      ,
    "kd"      ,
    "target"  ,
    "reseterr",
    "config"  ,
};

void sendConfig() {
    Serial.print(command_table[CommandConfig].name);
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

    controller[addr].getMotor()->setPwmHoming(line.toInt());
}

void acceptCommand(char *buffer) {
    String line(buffer);
    size_t addr;
    char *string = buffer;
    string = strtok(string, delimeters);
    
    if (strstr(string, command_table[CommandSet]) != NULL) {
        string = strtok(string, delimeters);
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

        string = strtok(string, delimeters);
        if (string == NULL) {
            return;
        }
        else if (strstr(string, command_table[CommandConfig]) != NULL) {
            string = strtok(string, delimeters);
            if (string != NULL) {
                acceptConfig(addr, string);
            }
        }
    }
    else if (strstr(string, command_table[CommandGet]) != NULL) {

    }

    // if (string != NULL) {
    //     line = line.substring(command_table[CommandSet].len);
    //     line.trim();
        
    //     if (line[0] == '0') {
    //         addr = 0;
    //     }
    //     else if (line[0] == '1') {
    //         addr = 1;
    //     }
    //     else {
    //         return;
    //     }
    //     line = line.substring(1);
    //     line.trim();

    //     if (line.startsWith(command_table[CommandMotion].name)) {
    //         line = line.substring(command_table[CommandMotion].len);
    //         line.trim();
    //         controller[addr].getMotor()->setMotion(line.toInt());
    //     }
    //     else if (line.startsWith(command_table[CommandPid].name)) {
    //         controller[addr].setMode(Controller::ModePid);
    //     }
    //     else if (line.startsWith(command_table[CommandHoming].name)) {
    //         controller[addr].setMode(Controller::ModeHoming);
    //     }
    //     else if (line.startsWith(command_table[CommandKp].name)) {
    //         line = line.substring(command_table[CommandKp].len);
    //         line.trim();
    //         controller[addr].setKp(line.toInt());
    //     }
    //     else if (line.startsWith(command_table[CommandKi].name)) {
    //         line = line.substring(command_table[CommandKi].len);
    //         line.trim();
    //         controller[addr].setKi(line.toInt());
    //     }
    //     else if (line.startsWith(command_table[CommandKd].name)) {
    //         line = line.substring(command_table[CommandKd].len);
    //         line.trim();
    //         controller[addr].setKd(line.toInt());
    //     }
    //     else if (line.startsWith(command_table[CommandTarget].name)) {
    //         line = line.substring(command_table[CommandTarget].len);
    //         line.trim();
    //         controller[0].setTarget(line.toInt());
    //     }
    //     else if (line.startsWith(command_table[CommandConfig].name)) {
    //         line = line.substring(command_table[CommandConfig].len);
    //         line.trim();
    //         acceptConfig(addr, line);
    //         sendConfig();
    //     }
    //     else if (line.startsWith(command_table[CommandResetError].name)) {
    //         controller[addr].resetError();
    //     }
    // }
    // else if (line.startsWith(command_table[CommandGet].name)) {
    //     line = line.substring(command_table[CommandGet].len);
    //     line.trim();
    //     if (line.startsWith(command_table[CommandConfig].name)) {
    //         sendConfig();
    //     }
    // }
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
