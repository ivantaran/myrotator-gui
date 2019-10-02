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
    qreal getPwm(uint index);
    uint getCurrentAdc(uint index);
    qreal getCurrentAmp(uint index);
    uint getDiag(uint index);
    uint getDirection(uint index);
    const QString getDirectionString(uint index);
    const QString getModeString(uint index);
    const QString getErrorString(uint index);
    qreal getAngle(uint index);
    bool isEndstop(uint index);
    qreal getPwmHoming(uint index);
    qreal getPwmMin(uint index);
    qreal getPwmMax(uint index);
    qreal getAngleMin(uint index);
    qreal getAngleMax(uint index);
    qreal getTolerance(uint index);

    void setMotion(uint index, qreal value);
    void setController(uint index, int kp, int ki, int kd);
    void setTargetLinear(uint index, int angle);
    void setTargetRadians(uint index, qreal angle);
    void setTargetDegrees(uint index, qreal angle);
    void setModePid(uint index);
    void setModeHoming(uint index);
    void setPwmHoming(uint index, qreal value);
    void setPwmMin(uint index, qreal value);
    void setPwmMax(uint index, qreal value);
    void setAngleMin(uint index, qreal value);
    void setAngleMax(uint index, qreal value);
    void setTolerance(uint index, qreal value);
    void resetError(uint index);

private:
    typedef enum {
        ModeDefault = 0,
        ModePid = 1,
        ModeHoming = 2,
        ModeSpeed = 3
    } ControllerMode;
    
    typedef enum {
        ErrorOk = 0, 
        ErrorSensor = 1, 
        ErrorHoming = 2, 
    } ControllerError;

    QString m_stateLine;
    ControllerMode m_mode[2] = { ModeDefault, ModeDefault };
    ControllerError m_error[2] = { ErrorOk, ErrorOk };
    qreal m_pwm[2] = { 0.0, 0.0 };
    uint m_currentSensor[2] = { 0, 0 };
    uint m_diag[2] = { 0, 0 };
    uint m_direction[2] = { 0, 0 };
    qreal m_angle[2] = { 0.0, 0.0 };
    bool m_endstop[2] = { true, true };

    qreal m_pwmHoming[2] = { 0.0, 0.0 };
    qreal m_pwmMin[2] = { 0.0, 0.0 };
    qreal m_pwmMax[2] = { 0.0, 0.0 };
    qreal m_angleMin[2] = { 0.0, 0.0 };
    qreal m_angleMax[2] = { 0.0, 0.0 };
    qreal m_tolerance[2] = { 0.0, 0.0 };

private slots:
    void readyReadSlot();

public slots:
    void requestConfigSlot();

signals:
    void updatedState(const QString &line);

};

#endif /* MONSTER_H_ */
