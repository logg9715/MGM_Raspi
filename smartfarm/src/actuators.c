#include <wiringPi.h>
#include <softPwm.h>

int tmprot() {
    pinMode(26, OUTPUT);
    softPwmCreate(26, 0, 128);
    softPwmWrite(26, 70);
    delay(500);
    softPwmWrite(26, 0);
    digitalWrite(26, 0);
    return 0;
}

int tmpspr() {
    pinMode(28, OUTPUT);
    softPwmCreate(28, 0, 128);
    softPwmWrite(28, 70);
    delay(5000);
    softPwmWrite(28, 0);
    digitalWrite(28, 0);
    return 0;
}

int tmpled() {
    pinMode(25, OUTPUT);
    static int stat = 0;
    stat = (stat == 0) ? 1 : 0;
    digitalWrite(25, stat);
    return stat;
}
