#ifndef MYROTATOR_H_
#define MYROTATOR_H_

#define SDA_PIN 2
#define SDA_PORT PORTD
#define SCL_PIN 3
#define SCL_PORT PORTD
#define I2C_PULLUP 1
#include <SoftWire.h>
#include <Wire.h>

#define PIN_EN1   A0
#define PIN_EN2   A1
#define PIN_CS1   A2
#define PIN_CS2   A3

#define PIN_INA1  7
#define PIN_INB1  8

#define PIN_INA2  4
#define PIN_INB2  9

#define PIN_PWM1  5
#define PIN_PWM2  6

#define PWM_MASK  0xff

#define TIMER_PERIOD  200ul

/* Sensor defines */
#define UART_SPEED            115200
#define I2C_SENSOR_ADDR       0x36
#define I2C_SENSOR_ANGLE_ADDR 0x0e
/**/

void motor1_brake(void);

class MyMotor {
public:    
    
    MyMotor(uint8_t pin_ina, uint8_t pin_inb, uint8_t pin_cs, uint8_t pin_en, uint8_t pin_pwm) {
        m_pin_ina = pin_ina;
        m_pin_inb = pin_inb;
        m_pin_cs = pin_cs;
        m_pin_en = pin_en;
        m_pin_pwm = pin_pwm;

        pinMode(m_pin_ina, OUTPUT);
        pinMode(m_pin_inb, OUTPUT);
        pinMode(m_pin_pwm, OUTPUT);
        
        m_pwm = 0;

        this->setMotion(0);
    }
    
    virtual ~MyMotor() {}
    
    inline uint8_t getPwm() {
        return m_pwm;
    }

    void setMotion(long value) {
        if (value > 0) {
            if (value > 255) {
                m_pwm = 255;
                this->setMotionRight();
            }
            // else if (value < 50) {
            //     m_pwm = 50;
            //     this->setMotionRight();
            // }
            else {
                m_pwm = value;
                this->setMotionRight();
            }
        }
        else if (value < 0) {
            if (value < -255) {
                m_pwm = 255;
                this->setMotionLeft();
            }
            // else if (value > -50) {
            //     m_pwm = 50;
            //     this->setMotionLeft();
            // }
            else {
                m_pwm = -value;
                this->setMotionLeft();
            }
        }
        else {
            m_pwm = 0;
            this->brake();
        }
        analogWrite(m_pin_pwm, m_pwm);
    }
    
    inline int getCurrentSensorValue() {
        return analogRead(m_pin_cs);
    }

    inline int getEnDiagValue() {
        return digitalRead(m_pin_en);
    }

    inline int getInaValue() {
        return digitalRead(m_pin_ina);
    }

    inline int getInbValue() {
        return digitalRead(m_pin_inb);
    }

private:
    uint8_t m_pin_ina;
    uint8_t m_pin_inb; 
    uint8_t m_pin_cs;
    uint8_t m_pin_en;
    uint8_t m_pin_pwm;
    uint8_t m_pwm;

    void brake() {
        digitalWrite(m_pin_ina, LOW);
        digitalWrite(m_pin_inb, LOW);
    }
    
    void setMotionRight() {
        digitalWrite(m_pin_ina, HIGH);
        digitalWrite(m_pin_inb, LOW);
    }

    void setMotionLeft() {
        digitalWrite(m_pin_ina, LOW);
        digitalWrite(m_pin_inb, HIGH);
    }

};

#endif /* MYROTATOR_H_ */
