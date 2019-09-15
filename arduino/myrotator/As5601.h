#ifndef AS5601_H_
#define AS5601_H_

#define SDA_PIN 2
#define SDA_PORT PORTD
#define SCL_PIN 3
#define SCL_PORT PORTD
#define I2C_PULLUP 1
#include <SoftWire.h>
#include <Wire.h>

#define AS5601_ADDR       0x36
#define AS5601_ANGLE_ADDR 0x0e

#define AS5601_RATIO            2
#define AS5601_TURNOVER_VALUE   4096

SoftWire softWire = SoftWire();

class As5601 {
public:
    As5601(bool isHardware) {
        m_isHardware = isHardware;
        
        if (m_isHardware) {
            Wire.begin();
        }
        else {
            softWire.begin();
        }

        m_turnover = 0;
        m_rawAngle[0] = -1;
        m_rawAngle[1] = -1;
        m_angleOffset = 0;
        m_angle = 0;
        m_isValid = false;
    }

    ~As5601() {

    }
    
    void setAngleOffset(int16_t offset) {
        m_angleOffset = offset;
    }
    
    void setZero() {
        m_angleOffset = m_angle;
    }
    
    int16_t getAngle(bool *ok = nullptr) {
        if (ok != nullptr) {
            (*ok) = m_isValid;
        }
        return m_angle;
    }

    inline bool isValid() {
        return m_isValid;
    }

    void requestSensorValue() {
        if (m_isHardware) {
            Wire.beginTransmission(AS5601_ADDR);
            Wire.write(byte(AS5601_ANGLE_ADDR));
            Wire.endTransmission();
            Wire.requestFrom(AS5601_ADDR, 2);
        }
        else {
            softWire.beginTransmission(AS5601_ADDR);
            softWire.write(byte(AS5601_ANGLE_ADDR));
            softWire.endTransmission();
            softWire.requestFrom(AS5601_ADDR, 2);
        }

        int16_t value = 0;
        uint8_t n = 0;
        
        while (Wire.available()) {
            byte c = Wire.read();
            value = (value << 8) | c;
            n++;
        }
        
        m_rawAngle[1] = m_rawAngle[0];
        m_rawAngle[0] = (n == 2) ? value : -1;
        
        if (m_rawAngle[1] - m_rawAngle[0] > AS5601_TURNOVER_VALUE / 2) {
            m_turnover++;
        }
        else if (m_rawAngle[0] - m_rawAngle[1] > AS5601_TURNOVER_VALUE / 2) {
            m_turnover--;
        }
        
        if (m_rawAngle[0] != -1 && m_rawAngle[1] != -1) {
            m_angle = m_rawAngle[0] + AS5601_TURNOVER_VALUE * m_turnover - m_angleOffset;
        }
    }

private:
    bool m_isHardware;
    int8_t m_turnover;
    int16_t m_rawAngle[2];
    int16_t m_angleOffset;
    int16_t m_angle;
    bool m_isValid;

};

#endif /* AS5601_H_ */
