#ifndef ENDSTOP_H_
#define ENDSTOP_H_


class Endstop {
public:
    Endstop(uint8_t pin) {
        m_pin = pin;
    }

    virtual ~Endstop() {

    }
    
    void begin() {
        pinMode(m_pin, INPUT_PULLUP);
    }

    bool isEnd() {
        return (digitalRead(m_pin) == HIGH);
    }

private:
    uint8_t m_pin;

};

#endif /* ENDSTOP_H_ */
