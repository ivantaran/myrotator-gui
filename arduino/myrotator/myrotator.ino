
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

void timer_event() {
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
    CommandSt,
    
    CommandVl,
    CommandVr,
    CommandVu,
    CommandVd,
    CommandCr,
    CommandCw,
    CommandGs,
    CommandGe,

    CommandVe,
    CommandIp,
    CommandOp,
    CommandAn,

    CommandAr,
    CommandEr,
    CommandAw,
    CommandEw,
    CommandAl,
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
    "ST",   // Set time            YY:MM:DD:HH:MM:SS

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
    "AR",
    "ER",
    "AW",
    "EW",
    "AL",
};

int accept_command_xr(Controller *c, const char *str) {
    int sendnum;
    
    sendnum = 0;

    if (str[2] != '\0') {
        Serial.write(str, 3);
        Serial.print(',');

        switch (str[2]) {
        case '0':
            Serial.print(c->getMotor()->getPwmHoming());
            break;
        case '1':
            Serial.print(c->getMotor()->getPwmMin());
            break;
        case '2':
            Serial.print(c->getMotor()->getPwmMax());
            break;
        case '3':
            Serial.print(c->getAngleMin());
            break;
        case '4':
            Serial.print(c->getAngleMax());
            break;
        case '5':
            Serial.print(c->getTolerance());
            break;
        case '6':
            Serial.print(c->getKp());
            break;
        case '7':
            Serial.print(c->getKi());
            break;
        case '8':
            Serial.print(c->getKd());
            break;
        default:
            Serial.print('0');
            break;
        }

        Serial.print(' ');
        sendnum++;
    }

    return sendnum;
}

void accept_command_xw(Controller *c, const char *str) {
    int vi;
    long vl;

    if (str[2] != '\0' && str[3] != '\0' && str[3] == ',') {
        switch (str[2]) {
        case '0':
            vi = atoi(str + 4);
            c->getMotor()->setPwmHoming(vi);
            break;
        case '1':
            vi = atoi(str + 4);
            c->getMotor()->setPwmMin((uint8_t)vi);
            break;
        case '2':
            vi = atoi(str + 4);
            c->getMotor()->setPwmMax((uint8_t)vi);
            break;
        case '3':
            vl = atol(str + 4);
            c->setAngleMin(vl);
            break;
        case '4':
            vl = atol(str + 4);
            c->setAngleMax(vl);
            break;
        case '5':
            vl = atol(str + 4);
            c->setTolerance(vl);
            break;
        case '6':
            vl = atol(str + 4);
            c->setKp(vl);
            break;
        case '7':
            vl = atol(str + 4);
            c->setKi(vl);
            break;
        case '8':
            vl = atol(str + 4);
            c->setKd(vl);
            break;
        }
    }
}

int accept_parameters(size_t addr, const char *str) {
    float value;
    int sendnum;
    
    sendnum = 0;

    switch (addr) {
    case CommandAz:
        if (str[2] == '\0' || str[2] == ' ') {
            Serial.print(command_table[addr]);
            Serial.print((float)controller[0].getSensor()->getAngleDegrees(), 1);
            Serial.print(' ');
            sendnum++;
        }
        else {
            value = atof(str + 2);
            controller[0].setTargetDegrees(value);
        }
        break;
    case CommandEl:
        if (str[2] == '\0' || str[2] == ' ') {
            Serial.print(command_table[addr]);
            Serial.print((float)controller[1].getSensor()->getAngleDegrees(), 1);
            Serial.print(' ');
            sendnum++;
        }
        else {
            value = atof(str + 2);
            controller[1].setTargetDegrees(value);
        }
        break;
    case CommandAr:
        sendnum += accept_command_xr(&controller[0], str);
        break;
    case CommandEr:
        sendnum += accept_command_xr(&controller[1], str);
        break;
    case CommandAw:
        accept_command_xw(&controller[0], str);
        break;
    case CommandEw:
        accept_command_xw(&controller[1], str);
        break;
    case CommandVe:
        Serial.print("myrotator");
        sendnum++;
        break;
    default:
        Serial.print("ALcommand[");
        Serial.print(addr);
        Serial.print("] skipped: ");
        Serial.println(str);
        sendnum++;
        break;
    }
    
    return sendnum;
}

void accept_command(char *buffer) {
    size_t i;
    int n, sendnum;
    char *str;
    
    sendnum = 0;
    n = MAXN_COMMANDS;
    str = strtok(buffer, " ");
        
    while (n-- && str) {
        for (i = 0; i < sizeof(command_table) / sizeof(command_table[0]); i++) {
            if (strncmp(command_table[i], str, 2) == 0) {
                    sendnum += accept_parameters(i, str);
                break;
            }
        }
        str = strtok(NULL, " ");
    }
    
    if (sendnum > 0) {
        Serial.println();
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
        timer_event();
    }

    accept_serial();
}
