#pragma once
#include <arduino.h>

class Debounce {
  public:
    Debounce(byte button);

    byte read(); // returns the debounced button state: LOW or HIGH.
    unsigned int count(); // Returns the number of times the button was pressed.
    void resetCount(); // Resets the button count number. 
    
  private:
    byte _button, _state, _lastState, _reading;
    unsigned int _count;
    unsigned long _delay, _last;
    boolean _wait;
};