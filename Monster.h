/*
 * Monster.h
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#ifndef MONSTER_H_
#define MONSTER_H_

#include <QtSerialPort>

class Monster : public QSerialPort {
    Q_OBJECT
public:
    Monster();
    virtual ~Monster();
    uint getPwm(uint index);
    uint getCurrentAdc(uint index);
    qreal getCurrentAmp(uint index);
    uint getDiag(uint index);
    uint getDirection(uint index);
    const QString getDirectionString(uint index);
    const QString getModeString(uint index);
    const QString getErrorString(uint index);
    qreal getAngleRadians(uint index);
    qreal getAngleDegrees(uint index);
    bool isEndstop(uint index);
    void setMotion(uint index, qreal value);
    void setController(uint index, int kp, int ki, int kd);
    void setTargetLinear(uint index, int angle);
    void setTargetRadians(uint index, qreal angle);
    void setTargetDegrees(uint index, qreal angle);
    void setModePid(uint index);
    void setModeHoming(uint index);
    void setPwmHoming(uint index, qreal value);
    void resetError(uint index);
private:
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

    QString m_stateLine;
    ControllerMode m_mode[2] = { ControllerMode::Default, ControllerMode::Default };
    ControllerError m_error[2] = { ControllerError::Ok, ControllerError::Ok};
    uint m_pwm[2] = { 0, 0 };
    uint m_currentSensor[2] = { 0, 0 };
    uint m_diag[2] = { 0, 0 };
    uint m_direction[2] = { 0, 0 };
    qreal m_angle[2] = { 0.0, 0.0 };
    bool m_endstop[2] = { true, true };

private slots:
    void readyReadSlot();

signals:
    void updatedState(const QString &line);
};

#endif /* MONSTER_H_ */
