#pragma once
#include <Arduino.h>

class relayTest
{
public:

    relayTest(byte ncPin, byte noPin, byte coilPin)
    {
        this->ncPin = ncPin;
        this->noPin = noPin;
        this->coilPin = coilPin;
        digitalWrite(coilPin, LOW);
        lastTime = 0;
        lastTime2 = 0;
        count = 0;
        noFailcount = 0;
        ncFailcount = 0;
        pulseTestProgress = 0;
        buzzTestCurrentHz = 0;
        buzzHz = 1;
        buzzFailHz = 0;
    }

    int pulseTest(int pulses, int speedHz); // Returns amount of failed pulses

    byte getPulseTestProgress() { return (pulseTestProgress); } // 0 -100% progress status

    int buzzTest(int durationSecs); // Returns the frequency at which the relay fails

    int getBuzzTestCurrentHz() { return (buzzTestCurrentHz); } // 0 -100% progress status

    void setNcFailCount(int val) { ncFailcount = val; }

    int getNcFailCount() { return (ncFailcount); }

    void resetNcFailCount() { ncFailcount = 0; }

    void setNoFailCount(int val) { noFailcount = val; }

    int getNoFailCount() { return (noFailcount); }

    void resetNoFailCount() { noFailcount = 0; }

    int getBuzzFailHz() { return (buzzFailHz); }

    void setbuzzFailHz(int val) { buzzFailHz = val; }

    void resetBuzzFailHz() { buzzFailHz = 0; }

    void setMode(byte a)
    {
        testMode = a;
    }

private:
    byte ncPin;
    byte noPin;
    byte coilPin;
    byte pulseTestProgress;
    byte buzzTestCurrentHz;
    byte testMode = 0;
    unsigned long time;
    unsigned long lastTime;
    unsigned long lastTime2;
    int count;
    int ncFailcount;
    int noFailcount;
    int buzzHz;
    int buzzFailHz;
};