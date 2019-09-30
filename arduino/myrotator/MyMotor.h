#ifndef MYMOTOR_H_
#define MYMOTOR_H_

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

#define TIMER_PERIOD    200ul
#define UART_SPEED      115200

class MyMotor {
public:    
    MyMotor(uint8_t pinIna, uint8_t pinInb, uint8_t pinCs, uint8_t pinEn, uint8_t pinPwm) {
        m_pinIna = pinIna;
        m_pinInb = pinInb;
        m_pinCs = pinCs;
        m_pinEn = pinEn;
        m_pinPwm = pinPwm;

        m_pwm = 0;
        m_pwmMin = 50;
        m_pwmMax = 255;
        m_pwmHoming = 0;
    }
    
    virtual ~MyMotor() {}
    
    void begin() {
        pinMode(m_pinIna, OUTPUT);
        pinMode(m_pinInb, OUTPUT);
        pinMode(m_pinPwm, OUTPUT);
        this->setMotion(0);
    }

    void setMotion(long value) {
        if (value > 0) {
            if (value > 255) {
                m_pwm = 255;
                this->setMotionRight();
            }
            else if (value < 50) {
                m_pwm = 0;
                this->brake();
            }
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
            else if (value > -50) {
                m_pwm = 0;
                this->brake();
            }
            else {
                m_pwm = -value;
                this->setMotionLeft();
            }
        }
        else {
            m_pwm = 0;
            this->brake();
        }
        analogWrite(m_pinPwm, m_pwm);
    }
    
    void setPwmMin(uint8_t value) {
        m_pwmMin = (value < m_pwmMax) ? value : m_pwmMax;
    }

    void setPwmMax(uint8_t value) {
        m_pwmMax = (value > m_pwmMin) ? value : m_pwmMin;
    }

    void setPwmHoming(int value) {
        m_pwmHoming = value;
    }

    inline int getCurrentSensorValue() {
        return analogRead(m_pinCs);
    }

    inline int getEnDiagValue() {
        return digitalRead(m_pinEn);
    }

    inline int getInaValue() {
        return digitalRead(m_pinIna);
    }

    inline int getInbValue() {
        return digitalRead(m_pinInb);
    }

    inline uint8_t getPwm() {
        return m_pwm;
    }

    inline uint8_t getPwmMin() {
        return m_pwmMin;
    }

    inline uint8_t getPwmMax() {
        return m_pwmMax;
    }

    inline long getPwmHoming() {
        return m_pwmHoming;
    }

private:
    uint8_t m_pinIna;
    uint8_t m_pinInb; 
    uint8_t m_pinCs;
    uint8_t m_pinEn;
    uint8_t m_pinPwm;
    uint8_t m_pwm;
    uint8_t m_pwmMin;
    uint8_t m_pwmMax;
    int m_pwmHoming;

    void brake() {
        digitalWrite(m_pinIna, LOW);
        digitalWrite(m_pinInb, LOW);
    }
    
    void setMotionRight() {
        digitalWrite(m_pinIna, HIGH);
        digitalWrite(m_pinInb, LOW);
    }

    void setMotionLeft() {
        digitalWrite(m_pinIna, LOW);
        digitalWrite(m_pinInb, HIGH);
    }

};

#endif /* MYMOTOR_H_ */
