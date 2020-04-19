#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "MyMotor.h"
#include "As5601.h"
#include "Endstop.h"

class Controller {

public:
    typedef enum {
        StatusUnknown = 0x00,
        StatusIdle = 0x01,
        StatusMoving = 0x02,
        StatusPointing = 0x04,
        StatusError = 0x08, 
        StatusHoming = 0x10,
        StatusUnhoming = 0x20,
    } ControllerStatus;
    
    typedef enum {
        ErrorOk = 0x00, 
        ErrorSensor = 0x01, 
        ErrorJam = 0x02, 
        ErrorHoming = 0x04, 
        ErrorUnhoming = 0x08, 
    } ControllerError;

    Controller(MyMotor *motor, Endstop *endstop, As5601 *sensor) {
        m_kp = 0;
        m_ki = 0;
        m_kd = 0;
        m_target = 0;
        m_tolerance = 0;
        m_angleMin = 0;
        m_angleMax = 0;
        m_status = StatusIdle;
        m_error = ErrorOk;
        m_motor = motor;
        m_endstop = endstop;
        m_sensor = sensor;
        m_homingSuccess = false;

        resetPid();
    }

    virtual ~Controller() {

    }
    
    void begin() {
        m_motor->begin();
        m_endstop->begin();
        m_sensor->begin();
    }

    void setStatus(const ControllerStatus &status) {
        if (m_status != status) {
            if (m_error != ErrorOk) {
                Serial.println("ALclear_error");
                m_status = StatusIdle;
            } else if (status == StatusIdle) {
                m_status = StatusIdle;
            } else {
                if (m_homingSuccess || (status == StatusHoming)) {
                    m_status = status;
                }
                else {
                    m_status = StatusUnhoming;
                }

                switch (m_status) {
                case StatusUnhoming:
                    m_homingSuccess = false;
                    m_sensor->resetOffset();
                    break;
                default:
                    break;
                }
            }
            resetPid();
        }
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
    
    void setTargetDegrees(float value) {
        if (m_status != StatusPointing) {
            setStatus(StatusPointing);
        }
        m_target = (long)(-4096.0f * value / 180.0f);
    }

    void setTargetVelocity(int16_t value) {
        if (m_status != StatusMoving) {
            setStatus(StatusMoving);
            m_target = 0;
        }
        m_target = value >= 0 ? m_target + 1 : m_target - 1;
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

    void clearError() {
        m_error = ErrorOk;
    }

    inline const ControllerStatus &getStatus() {
        return m_status;
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

    inline uint8_t getError() {
        return m_error;
    }

    void execute() {
        m_sensor->requestSensorValue();
        
        if (!m_sensor->isValid()) {
            setError(ErrorSensor);
        }

        switch (m_status) {
        case StatusPointing:
            pid();
            break;
        case StatusHoming:
            homing();
            break;
        case StatusUnhoming:
            unhoming();
            break;
        case StatusMoving:
            pid();
            break;
        default:
            break;
        }
    }
    
    void movePositive() {
        m_motor->setMotion(abs(m_motor->getPwmHoming()));
        setStatus(StatusMoving);
    }

    void moveNegative() {
        m_motor->setMotion(-abs(m_motor->getPwmHoming()));
        setStatus(StatusMoving);
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

    ControllerStatus m_status;
    uint8_t m_error;

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
    bool m_homingSuccess;
    bool m_unhomingSuccess;

    void pid() {
        bool ok = false;
        m_deviation[2] = m_deviation[1];
        m_deviation[1] = m_deviation[0];
        
        if (getStatus() == StatusPointing) {
            m_deviation[0] = m_sensor->getAngle(&ok) - m_target;
        } else if (getStatus() == StatusMoving) {
            m_deviation[0] = m_sensor->getVelocity(&ok) - m_target;
        } else {
            resetPid();
        }
        
        if (abs(m_deviation[0]) < m_tolerance || !ok) {
            resetPid();
        } else {
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
        if (m_homingSuccess) {
            setStatus(StatusIdle);
        }
        else {
            if (abs(m_sensor->getAngle()) > m_angleMax) {
                setError(ErrorHoming);
            }
            else {
                if (m_endstop->isEnd()) {
                    m_sensor->setZero();
                    m_homingSuccess = true;
                    setStatus(StatusIdle);
                } else {
                    m_motor->setMotion(m_motor->getPwmHoming());
                }
            }
        }
    }

    void unhoming() {
        if (abs(m_sensor->getAngle()) > m_angleMax) {
            setError(ErrorUnhoming);
        }
        else {
            if (!m_endstop->isEnd()) {
                setStatus(StatusHoming);
            } else {
                m_motor->setMotion(-m_motor->getPwmHoming());
            }
        }
    }

    void setError(const ControllerError &error) {
        if ((m_error & error) == error) {
            return;
        }

        m_error |= error;
        
        switch (error) {
        case ErrorHoming:
            m_sensor->resetOffset();
            break;
        default:
            break;
        }

        if (m_error != ErrorOk) {
            setStatus(StatusError);
        }
    }

};

#endif /* CONTROLLER_H_ */
