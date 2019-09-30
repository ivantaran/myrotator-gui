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
    
    typedef enum {
        Ok = 0, 
        ErrorSensor = 1, 
        ErrorHoming = 2, 
    } ControllerError;

    Controller(const ControllerType &type) {
        m_kp = 0;
        m_ki = 0;
        m_kd = 0;
        m_target = 0;
        m_tolerance = 0;
        m_angleMin = 0;
        m_angleMax = 4095;
        m_mode = ControllerMode::Default;
        m_error = ControllerError::Ok;

        resetPid();

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
        if (m_error == ControllerError::Ok) {
            m_mode = mode;
        }
        resetPid();
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
    
    void setTarget(long value) {
        m_target = value;
    }

    void setPidTolerance(long value) {
        m_tolerance = value;
    }

    void resetError() {
        m_error = ControllerError::Ok;
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

    inline long getTolerance() {
        return m_tolerance;
    }
    
    inline const ControllerError &getError() {
        return m_error;
    }

    void execute() {
        m_sensor->requestSensorValue();
        
        if (!m_sensor->isValid()) {
            setError(ControllerError::ErrorSensor);
        }

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

    ControllerMode m_mode;
    ControllerError m_error;

    long m_kp;
    long m_ki;
    long m_kd;
    long m_scale;
    long m_pidValueLimit;
    long m_tolerance;
    long m_target;
    long m_deviation[3];
    long m_pidValue;
    long m_angleMin;
    long m_angleMax;

    void pid() {
        m_deviation[2] = m_deviation[1];
        m_deviation[1] = m_deviation[0];
        m_deviation[0] = m_sensor->getAngle() - m_target;
        
        if (abs(m_deviation[0]) < m_tolerance) {
            resetPid();
        }
        else {
            m_pidValue += m_kp * (m_deviation[0] - m_deviation[1]) 
                    + m_ki * m_deviation[0] 
                    + m_kd * (m_deviation[0] - m_deviation[1] - m_deviation[1] + m_deviation[2]);

            if (m_pidValue > m_pidValueLimit) {
                m_pidValue = m_pidValueLimit;
            }
            else if (m_pidValue < -m_pidValueLimit) {
                m_pidValue = -m_pidValueLimit;
            }
            m_motor->setMotion(m_pidValue / m_scale);
        }
    }

    void setScale(long value) {
        m_scale = value;
        m_pidValueLimit = (long)m_motor->getPwmMax() * m_scale;
    }
    
    void resetPid() {
        m_pidValue = 0;
        m_deviation[0] = 0;
        m_deviation[1] = 0;
        m_deviation[2] = 0;
        setScale(1000);
        m_motor->setMotion(0);
    }

    void homing() {
        if (abs(m_sensor->getAngle()) > m_angleMax) {
            setError(ControllerError::ErrorHoming);
        }
        else {
            if (m_endstop->isEnd()) {
                setMode(ControllerMode::Default);
                m_sensor->setZero();
            } else {
                m_motor->setMotion(m_motor->getPwmHoming());
            }
        }
    }

    void setError(const ControllerError &error) {
        m_error = error;
        
        switch (m_error) {
        case ControllerError::ErrorHoming:
            m_sensor->resetOffset();
            break;
        default:
            break;
        }

        if (m_error != ControllerError::Ok) {
            setMode(ControllerMode::Default);
        }
    }

};

#endif /* CONTROLLER_H_ */
