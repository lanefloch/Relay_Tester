#include <Arduino.h>
#include <digitalWriteFast.h>
#include "RelayTest.h"

int relayTest::pulseTest(int pulses, int speedHz) // Returns 1 when done
{
    unsigned long time = millis();

    if (count < pulses)
    {

        if (time > lastTime + (1000 / speedHz))
        {
            if (digitalRead(coilPin) == LOW)
            {
                if (digitalRead(ncPin) == 1)
                {
                    if (testMode != 1)
                        ncFailcount++;
                };
                if (digitalRead(noPin) == 0)
                {
                    if (testMode != 2)
                        noFailcount++;
                }; // backwards due to INPUT_PULLUP

                digitalWrite(coilPin, HIGH);
            }
            else
            {
                if (digitalRead(ncPin) == 0)
                {
                    if (testMode != 1)
                        ncFailcount++;
                };
                if (digitalRead(noPin) == 1)
                {
                    if (testMode != 2)
                        noFailcount++;
                };

                digitalWrite(coilPin, LOW);
                count++;
            }
            lastTime = millis();
            pulseTestProgress = ((float)count / (float)pulses) * 100;
            return (0);
        }
    }
    else if (count >= pulses)
    {
        count = 0;
        pulseTestProgress = 0;
        digitalWrite(coilPin, LOW);
        return (1);
    }
}

int relayTest::buzzTest(int durationSecs) // Returns 1 when complete
{
    time = millis();

    if (time > lastTime + (1000 / buzzHz))
    {
        if (digitalReadFast(coilPin) == LOW)
        {
            if (digitalReadFast(ncPin) == 1 && (testMode == 2 || testMode == 0))
            {
                int a = buzzHz;
                buzzHz = 1;
                buzzFailHz = a;
                digitalWrite(coilPin, LOW);
                return (1);
            };
            if (digitalReadFast(noPin) == 0 && (testMode == 1 || testMode == 0))
            {
                int a = buzzHz;
                buzzHz = 1;
                buzzFailHz = a;
                digitalWrite(coilPin, LOW);
                return (1);
            }; // backwards due to INPUT_PULLUP

            digitalWriteFast(coilPin, HIGH);
        }
        else
        {
            if (digitalReadFast(ncPin) == 0 && (testMode == 2 || testMode == 0))
            {
                int a = buzzHz;
                buzzHz = 1;
                buzzFailHz = a;
                digitalWrite(coilPin, LOW);
                return (1);
            };
            if (digitalReadFast(noPin) == 1 && (testMode == 1 || testMode == 0))
            {
                int a = buzzHz;
                buzzHz = 1;
                buzzFailHz = a;
                digitalWrite(coilPin, LOW);
                return (1);
            };

            digitalWriteFast(coilPin, LOW);
        }

        buzzTestCurrentHz = buzzHz;

        if (time > lastTime2 + ((1000 * durationSecs) / 200))
        {
            buzzHz++;
            lastTime2 = time;
        }
        lastTime = time;
        return (0);
    }
}
