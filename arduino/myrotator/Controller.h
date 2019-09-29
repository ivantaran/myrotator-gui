#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "MyMotor.h"
#include "As5601.h"
#include "Endstop.h"

class Controller {

public:
    typedef enum {
        Azimuth = 0, 
        Elevation = 1
    } ControllerType;

    typedef enum {
        Default = 0,
        Pid = 1,
        Homing = 2,
        AngleSpeed = 3
    } ControllerMode;

    Controller(const ControllerType &type) {
        m_kp = 0;
        m_ki = 0;
        m_kd = 0;
        m_target = 0;
        resetPid();
        setScale(1000);

        switch (type) {
        case ControllerType::Elevation:
            m_motor = new MyMotor(PIN_INA2, PIN_INB2, PIN_CS2, PIN_EN2, PIN_PWM2);
            m_endstop = new Endstop(11);
            m_sensor = new As5601(As5601::Software);
            break;
        case ControllerType::Azimuth:
        default:
            m_motor = new MyMotor(PIN_INA1, PIN_INB1, PIN_CS1, PIN_EN1, PIN_PWM1);
            m_endstop = new Endstop(10);
            m_sensor = new As5601(As5601::Hardware);
            break;
        }
    }

    virtual ~Controller() {

    }
    
    void begin() {
        m_motor->begin();
        m_endstop->begin();
        m_sensor->begin();
    }

    void setMode(const ControllerMode &mode) {
        m_mode = mode;
        resetPid();
        m_motor->setMotion(0);
    }

    void setKp(long value) {
        m_kp = value;
    }

    void setKi(long value) {
        m_ki = value;
    }

    void setKd(long value) {
        m_kd = value;
    }
    
    void setScale(long value) {
        m_scale = value;
        m_pidValueLimit = 511 * m_scale;
    }
    
    void setTarget(long value) {
        m_target = value;
    }

    inline const ControllerMode &getMode() {
        return m_mode;
    }
    
    inline long getKp() {
        return m_kp;
    }

    inline long getKi() {
        return m_ki;
    }

    inline long getKd() {
        return m_kd;
    }

    void execute() {
        m_sensor->requestSensorValue();

        switch (m_mode) {
        case ControllerMode::Pid:
            pid();
            break;
        case ControllerMode::Homing:
            homing();
            break;
        case ControllerMode::AngleSpeed:
            break;
        case ControllerMode::Default:
        default:
            break;

        }
    }
    
    MyMotor *getMotor() {
        return m_motor;
    }

    As5601 *getSensor() {
        return m_sensor;
    }

    Endstop *getEndstop() {
        return m_endstop;
    }


private:
    MyMotor *m_motor;
    As5601 *m_sensor;
    Endstop *m_endstop;

    ControllerMode m_mode = ControllerMode::Default;

    long m_kp;
    long m_ki;
    long m_kd;
    long m_scale;
    long m_pidValueLimit;
    long m_target;
    long m_error[3];
    long m_pidValue;

    void pid() {
        if (!m_sensor->isValid()) {
            this->setMode(ControllerMode::Default);
            return;
        }

        m_error[2] = m_error[1];
        m_error[1] = m_error[0];
        m_error[0] = m_sensor->getAngle() - m_target;
        
        m_pidValue += m_kp * (m_error[0] - m_error[1]) 
                + m_ki * m_error[0] 
                + m_kd * (m_error[0] - m_error[1] - m_error[1] + m_error[2]);

        if (m_pidValue > m_pidValueLimit) {
            m_pidValue = m_pidValueLimit;
        }
        else if (m_pidValue < -m_pidValueLimit) {
            m_pidValue = -m_pidValueLimit;
        }

        m_motor->setMotion(m_pidValue / m_scale);
    }

    void resetPid() {
        m_pidValue = 0;
        m_error[0] = 0;
        m_error[1] = 0;
        m_error[2] = 0;
    }

    void homing() {
        if (m_endstop->isEnd()) {
            setMode(ControllerMode::Default);
            m_sensor->setZero();
        } else {
            m_motor->setMotion(m_motor->getPwmHoming());
        }
    }
    
};

#endif /* CONTROLLER_H_ */
