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
    uint getCurrentSensor(uint index);
    uint getDiag(uint index);
    uint getMotionState(uint index);
    qreal getAngle(uint index);
    void setMotion(uint index, int value);

private:
    QString m_stateLine;
    uint m_pwm[2] = { 0 };
    uint m_currentSensor[2] = { 0 };
    uint m_diag[2] = { 0 };
    uint m_motionState[2] = { 0 };
    qreal m_angle[2] = { 0.0 };

private slots:
    void readyReadSlot();

public slots:
    void testSlot();

signals:
    void updatedState(const QString &line);
};

#endif /* MONSTER_H_ */
