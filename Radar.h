#ifndef RADAR_H
#define RADAR_H

#include <QWidget>
#include <QPainter>
#include <QtMath>

class Radar : public QWidget {
    Q_OBJECT
    
public:
    Radar();
    virtual ~Radar();
    void setSensor(qreal azm, qreal elv, bool visible);    
protected:    
    virtual void paintEvent(QPaintEvent*);
    
private:
    
    class RadarItem{
    public:
        RadarItem() {    
            azm     = 0.0;//(qreal)(qrand() % 628) * 0.01;
            elv     = 0.0;//-M_PI_2;//(qreal)(qrand() % 157) * 0.01;
            visible = true;
        }
        qreal azm;
        qreal elv;
        bool visible;
    };
    
    RadarItem m_sensor;
    void drawSensor(QPainter *p);
};

#endif	/* RADAR_H */
