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
    typedef enum { Hardware = 0, Software = 1 } As5601Type;

    As5601(const As5601Type &type) {
        m_isHardware = (type == As5601::Hardware);
        
        m_angle = 0;
        m_isValid = false;
        m_offset = 0;
        m_rawAngle[0] = -1;
        m_rawAngle[1] = -1;
        m_turnover = 0;
        m_velocity = 0;
    }

    virtual ~As5601() {

    }
    
    void begin() {
        if (m_isHardware) {
            Wire.begin();
        }
        else {
            softWire.begin();
        }
    }

    void resetOffset() {
        m_offset = 0;
    }
    
    void setZero(int16_t hardwareOffset) {
        m_offset = m_angle + hardwareOffset;  // TODO: remove turnover's
    }
    
    int16_t getAngle(bool *ok = nullptr) {
        if (ok != nullptr) {
            (*ok) = m_isValid;
        }
        return m_angle;
    }

    int16_t getVelocity(bool *ok = nullptr) {
        if (ok != nullptr) {
            (*ok) = m_isValid;
        }
        return m_velocity;
    }

    int16_t getVelocityMilliDegrees(bool *ok = nullptr) {
        int16_t mdeg = getVelocity(ok);
        if (abs(mdeg) < 45) {
            mdeg *= 220;
        } else {
            mdeg = (mdeg > 0) ? 9999 : -9999;
        }
        return mdeg;
    }

    float getAngleDegrees(bool *ok = nullptr) {
        return (float)getAngle(ok) * 180.0f / -4096.0f;
    }

    inline bool isValid() {
        return m_isValid;
    }

    void requestSensorValue() {
        int16_t value = 0;
        uint8_t n = 0;
        uint8_t err = 0;

        if (m_isHardware) {
            Wire.beginTransmission(AS5601_ADDR);
            Wire.write(byte(AS5601_ANGLE_ADDR));
            err = Wire.endTransmission();
            if (err == 0) {
                Wire.requestFrom(AS5601_ADDR, 2);

                while (Wire.available()) {
                    byte c = Wire.read();
                    value = (value << 8) | c;
                    n++;
                }
            } else {
                m_isValid = false;
            }
        }
        else {
            softWire.beginTransmission(AS5601_ADDR);
            softWire.write(byte(AS5601_ANGLE_ADDR));
            err = softWire.endTransmission();
            if (err == 0) {
                softWire.requestFrom(AS5601_ADDR, 2);

                while (softWire.available()) {
                    byte c = softWire.read();
                    value = (value << 8) | c;
                    n++;
                }
            } else {
                m_isValid = false;
            }
        }
        
        m_rawAngle[1] = m_rawAngle[0];
        m_rawAngle[0] = (n == 2) ? value : -1;
        m_velocity = m_rawAngle[1] - m_rawAngle[0];

        if (m_velocity > AS5601_TURNOVER_VALUE / 2) {
            m_turnover++;
            m_velocity -= AS5601_TURNOVER_VALUE;
        } else if (m_velocity < -AS5601_TURNOVER_VALUE / 2) {
            m_turnover--;
            m_velocity += AS5601_TURNOVER_VALUE;
        }
        
        if (m_rawAngle[0] != -1 && m_rawAngle[1] != -1) {
            m_angle = m_rawAngle[0] + AS5601_TURNOVER_VALUE * m_turnover - m_offset;
            m_isValid = true;
        } else {
            m_isValid = false;
        }
    }

private:
    bool m_isHardware;
    bool m_isValid;
    int16_t m_angle;
    int16_t m_offset;
    int16_t m_rawAngle[2];
    int16_t m_velocity;
    int8_t m_turnover;

};

#endif /* AS5601_H_ */
