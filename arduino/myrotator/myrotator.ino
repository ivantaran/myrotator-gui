
#include "Controller.h"

#define MAXN_COMMANDS 64

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
    controller[0].execute();
    controller[1].execute();
}

typedef enum {
    CommandAz,
    CommandEl,
    CommandUp,
    CommandDn,
    CommandDm,
    CommandUm,
    CommandDr,
    CommandUr,
    CommandMl,
    CommandMr,
    CommandMu,
    CommandMd,
    CommandSa,
    CommandSe,
    CommandAo,
    CommandLo,
    CommandOp,
    CommandIp,
    CommandAn,
    CommandSt,
    CommandVe,
} command_e;

static const char* command_table[] {
    "AZ",   // Azimuth             number - 1 decimal place [deg]
    "EL",   // Elevation           number - 1 decimal place [deg]
    "UP",   // Uplink freq         in Hertz
    "DN",   // Downlink freq		in Hertz
    "DM",   // Downlink Mode       ascii, eg SSB, FM
    "UM",   // Uplink Mode         ascii, eg SSB, FM
    "DR",   // Downlink Radio      number
    "UR",   // Uplink Radio        number
    "ML",   // Move Left
    "MR",   // Move Right
    "MU",   // Move Up
    "MD",   // Move Down
    "SA",   // Stop azimuth moving
    "SE",   // Stop elevation moving
    "AO",   // AOS
    "LO",   // LOS
    "OP",   // Set output          number
    "IP",   // Read an input       number
    "AN",   // Read analogue input number
    "ST",   // Set time            YY:MM:DD:HH:MM:SS
    "VE",   // Request Version

    "VL",   // Velocity Left	    number [mdeg/s]
    "VR",   // Velocity Right	    number [mdeg/s]
    "VU",   // Velocity Up	    number [mdeg/s]
    "VD",   // Velocity Down	    number [mdeg/s]
    "CR",   // Read config         register [0-x]	1
    "CW",   // Write config        register [0-x]	2
    "GS",   // Get status register			3
    "GE",   // Get error register			4

    "VE",   // Request Version				5
    "IP",   // Read an input       number		6
    "OP",   // Set output          number		7
    "AN",   // Read analogue input number		8

};

void set_parameter(size_t addr, const char *str) {
    float value;

    switch (addr) {
    case CommandAz:
        value = atof(str);
        controller[0].setTargetDegrees(value);
        Serial.println(command_table[addr]);
        break;
    case CommandEl:
        value = atof(str);
        controller[1].setTargetDegrees(value);
        Serial.println(command_table[addr]);
        break;
    default:
        break;
    }
}

void send_parameter(size_t addr) {
    switch (addr) {
    case CommandAz:
        Serial.print(command_table[addr]);
        Serial.print((float)controller[0].getSensor()->getAngleDegrees(), 1);
        Serial.print(' ');
        break;
    case CommandEl:
        Serial.print(command_table[addr]);
        Serial.print((float)controller[1].getSensor()->getAngleDegrees(), 1);
        Serial.print(' ');
        break;
    default:
        break;
    }
}

void acceptCommand(char *buffer) {
    size_t i;
    int n, sendnum;
    char *str;
    sendnum = 0;
    n = MAXN_COMMANDS;
    str = strtok(buffer, " ");
        
    while (n-- && str) {
        for (i = 0; i < sizeof(command_table) / sizeof(command_table[0]); i++) {
            if (strncmp(command_table[i], str, 2) == 0) {
                if (str[2] != '\0' && str[2] != ' ') {
                    set_parameter(i, str + 2);
                }
                else {
                    send_parameter(i);
                    sendnum++;
                }
                break;
            }
        }
        str = strtok(NULL, " ");
    }

    if (sendnum > 0) {
        Serial.println();
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
