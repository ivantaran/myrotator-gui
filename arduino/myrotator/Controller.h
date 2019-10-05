#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "MyMotor.h"
#include "As5601.h"
#include "Endstop.h"

class Controller {

public:
    typedef enum {
        ModeDefault = 0,
        ModePid = 1,
        ModeHoming = 2,
        ModeAngleSpeed = 3
    } ControllerMode;
    
    typedef enum {
        ErrorOk = 0, 
        ErrorSensor = 1, 
        ErrorHoming = 2, 
    } ControllerError;

    Controller(MyMotor *motor, Endstop *endstop, As5601 *sensor) {
        m_kp = 0;
        m_ki = 0;
        m_kd = 0;
        m_target = 0;
        m_tolerance = 0;
        m_angleMin = 0;
        m_angleMax = 0;
        m_mode = ModeDefault;
        m_error = ErrorOk;
        m_motor = motor;
        m_endstop = endstop;
        m_sensor = sensor;

        resetPid();
    }

    virtual ~Controller() {

    }
    
    void begin() {
        m_motor->begin();
        m_endstop->begin();
        m_sensor->begin();
    }

    void setMode(const ControllerMode &mode) {
        if (mode == ModeDefault || m_error == ErrorOk) {
            m_mode = mode;

            switch (m_mode) {
            case ModeHoming:
                m_sensor->resetOffset();
                break;
            default:
                break;
            }
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

    void setTolerance(long value) {
        m_tolerance = value;
    }

    void setAngleMin(long value) {
        m_angleMin = value;
    }

    void setAngleMax(long value) {
        m_angleMax = value;
    }

    void resetError() {
        m_error = ErrorOk;
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

    inline long getAngleMin() {
        return m_angleMin;
    }

    inline long getAngleMax() {
        return m_angleMax;
    }

    inline const ControllerError &getError() {
        return m_error;
    }

    void execute() {
        m_sensor->requestSensorValue();
        
        if (!m_sensor->isValid()) {
            setError(ErrorSensor);
        }

        switch (m_mode) {
        case ModePid:
            pid();
            break;
        case ModeHoming:
            homing();
            break;
        case ModeAngleSpeed:
            break;
        case ModeDefault:
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
            setError(ErrorHoming);
        }
        else {
            if (m_endstop->isEnd()) {
                setMode(ModeDefault);
                m_sensor->setZero();
            } else {
                m_motor->setMotion(m_motor->getPwmHoming());
            }
        }
    }

    void setError(const ControllerError &error) {
        m_error = error;
        
        switch (m_error) {
        case ErrorHoming:
            m_sensor->resetOffset();
            break;
        default:
            break;
        }

        if (m_error != ErrorOk) {
            setMode(ModeDefault);
        }
    }

};

#endif /* CONTROLLER_H_ */
