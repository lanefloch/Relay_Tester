//Made by Lane Floch 7/3/2022

#include <Arduino.h>
#include <Bounce2.h>
#include <EEPROM.h>
#include "RelayTest.h"
#include "screen.h"
#include "bitmap.h"

#define sck 13 // LCD SPI com pins
#define cs 10
#define sid 11

#define coilpin 2 // relay IO
#define NOpenPin 3
#define NClosedPin 4

#define menuButton 5 // button IO
#define confirmButton 6

#define previous_test_eeprom_adress 0  // storage for previous stress test results (100,000 write cycles / address)
#define pulses_eeprom_adress 200       // storage for pulse amounnt value
#define buzzDuration_eeprom_adress 300 // storage for buzz duration value
#define mode_eeprom_adress 400

struct testResults // holds current and previous results of stress tests.
{
  int failHz;
  int ncFails;
  int noFails;
} resultVar, lastResultVar;

enum states // holds state machine current state
{
  mainMenu,      // 0
  allTest,       // 1
  pulseTest,     // 2
  buzzTest,      // 3
  eepromResults, // 4
  modeSet,       // 5
  buzzSet,       // 6
  pulseSet,      // 7
  buzzResults,   // 8
  allResults,    // 9
  pulseResults,  // 10
  wait           // 11
} machineState;

relayTest relay(NClosedPin, NOpenPin, coilpin); // testing library
screen lcdDisplay(sck, sid, cs);                // lcd formatting library
Bounce confButton = Bounce();                   // debounce library
Bounce selButton = Bounce();

int pulseTestAmount = 0; // default values for pulse and buzz test
int buzzTestMaxDuration = 0;

unsigned long time = 0; // used for general timekeeping throughout the program

unsigned long lastBuzzTime = 0; // used to keep screen periodically updated during buzz testing

bool clrscreenFlag = 0; // used to denote screen is in use and not ready to be cleared

bool allTestFlag = 0; // used to latch into "TEST ALL" mode

int cursorPosVal = 1; // denotes position of cursor within mainmenu

int lastCursorPosVal = 0;

int lastPulseVal = 0; // used for functions which set test paramenters
int lastBuzzVal = 0;

byte modeVal = 0; //used to keep track of dpdt spst no nc relays etc
byte lastModeVal = 0;

unsigned long lastSelButtPress = 0;
unsigned long lastConfButtPress = 0;

void cursorPos(int pos)
{
  if (pos > 7)
  {
    pos = 7;
  }
  else if (pos < 1)
  {
    pos = 1;
  }

  switch (pos)
  {
  case 1:
  {
    lcdDisplay.topRightJustifiedText(F("(*) TEST ALL"), 15);
    lcdDisplay.topRightJustifiedText(F("( ) PULSE TEST"), 27);
    lcdDisplay.topRightJustifiedText(F("( ) BUZZ TEST"), 39);
    lcdDisplay.topRightJustifiedText(F("( ) PREVIOUS RESULTS"), 51);
    return;
    break;
  }
  case 2:
  {
    lcdDisplay.topRightJustifiedText(F("( ) TEST ALL"), 15);
    lcdDisplay.topRightJustifiedText(F("(*) PULSE TEST"), 27);
    lcdDisplay.topRightJustifiedText(F("( ) BUZZ TEST"), 39);
    lcdDisplay.topRightJustifiedText(F("( ) PREVIOUS RESULTS"), 51);
    return;
    break;
  }
  case 3:
  {
    lcdDisplay.topRightJustifiedText(F("( ) TEST ALL"), 15);
    lcdDisplay.topRightJustifiedText(F("( ) PULSE TEST"), 27);
    lcdDisplay.topRightJustifiedText(F("(*) BUZZ TEST"), 39);
    lcdDisplay.topRightJustifiedText(F("( ) PREVIOUS RESULTS"), 51);
    return;
    break;
  }
  case 4:
  {
    lcdDisplay.topRightJustifiedText(F("( ) TEST ALL"), 15);
    lcdDisplay.topRightJustifiedText(F("( ) PULSE TEST"), 27);
    lcdDisplay.topRightJustifiedText(F("( ) BUZZ TEST"), 39);
    lcdDisplay.topRightJustifiedText(F("(*) PREVIOUS RESULTS"), 51);
    return;
    break;
  }
  case 5:
  {
    lcdDisplay.topRightJustifiedText(F("( ) PULSE TEST"), 15);
    lcdDisplay.topRightJustifiedText(F("( ) BUZZ TEST"), 27);
    lcdDisplay.topRightJustifiedText(F("( ) PREVIOUS RESULTS"), 39);
    lcdDisplay.topRightJustifiedText(F("(*) MODE SET"), 51);
    return;
    break;
  }
  case 6:
  {
    lcdDisplay.topRightJustifiedText(F("( ) BUZZ TEST"), 15);
    lcdDisplay.topRightJustifiedText(F("( ) PREVIOUS RESULTS"), 27);
    lcdDisplay.topRightJustifiedText(F("( ) MODE SET"), 39);
    lcdDisplay.topRightJustifiedText(F("(*) B-TEST SET"), 51);
    return;
    break;
  }

  case 7:
  {
    lcdDisplay.topRightJustifiedText(F("( ) PREVIOUS RESULTS"), 15);
    lcdDisplay.topRightJustifiedText(F("( ) MODE SET"), 27);
    lcdDisplay.topRightJustifiedText(F("( ) B-TEST SET"), 39);
    lcdDisplay.topRightJustifiedText(F("(*) P-TEST SET"), 51);
    return;
    break;
  }

  default:
  {
    lcdDisplay.topRightJustifiedText(F("( ) TEST ALL"), 15);
    lcdDisplay.topRightJustifiedText(F("( ) PULSE TEST"), 27);
    lcdDisplay.topRightJustifiedText(F("( ) BUZZ TEST"), 39);
    lcdDisplay.topRightJustifiedText(F("( ) PREVIOUS RESULTS"), 51);
    return;
    break;
  }
  }
};

void setup()
{
  machineState = mainMenu;

  lcdDisplay.screenBegin();

  confButton.attach(confirmButton, INPUT_PULLUP);
  selButton.attach(menuButton, INPUT_PULLUP);
  confButton.interval(50);
  selButton.interval(50);

  pinMode(coilpin, OUTPUT);
  pinMode(NOpenPin, INPUT_PULLUP);
  pinMode(NClosedPin, INPUT_PULLUP);
  pinMode(menuButton, INPUT_PULLUP);
  pinMode(confirmButton, INPUT_PULLUP);

  lcdDisplay.clearScreen();
  lcdDisplay.bitmap(0, 0, 128, 64, loadingScreen);
  lcdDisplay.writeScreen();
  delay(1500);
  lcdDisplay.clearScreen();

  Serial.begin(9600);

  // pulseTestAmount = EEPROM.read(pulses_eeprom_adress);
  EEPROM.get(pulses_eeprom_adress, pulseTestAmount);
  Serial.print("P Reading: ");
  Serial.println(pulseTestAmount);
  buzzTestMaxDuration = EEPROM.read(buzzDuration_eeprom_adress);
  Serial.print("B Reading: ");
  Serial.println(buzzTestMaxDuration);
  EEPROM.get(mode_eeprom_adress, modeVal);
  Serial.print("M Reading: ");
  Serial.println(modeVal);
}

void loop()
{
  selButton.update();
  confButton.update();

  // Serial.println(machineState);

  switch (machineState)
  {
  case mainMenu:
  {

    if (clrscreenFlag == 0)
    {
      lcdDisplay.clearScreen();

      lcdDisplay.centerJustifiedText(F("MAIN MENU"), -26); // These three lines needed for initial screen write. Otherwise have to press button first
      cursorPos(cursorPosVal);
      lcdDisplay.writeScreen();

      clrscreenFlag = 1;
    }

    if (cursorPosVal > 7)
    {
      cursorPosVal = 1;
    }

    if (selButton.changed())
    {
      if (!selButton.read())
      {
        cursorPosVal++;
      }
    }

    if (cursorPosVal != lastCursorPosVal)
    {
      lcdDisplay.clearScreen();
      lcdDisplay.centerJustifiedText(F("MAIN MENU"), -26); // Write permanent text
      cursorPos(cursorPosVal);
      lcdDisplay.writeScreen();
      lastCursorPosVal = cursorPosVal;
    }

    if (confButton.changed())
    {
      if (!confButton.read())
      {
        machineState = states(cursorPosVal);
        clrscreenFlag = 0;
      }
    }
  }
  break;

  case allTest:
  {
    allTestFlag = 1;
    machineState = pulseTest;
  }

  case pulseTest:
  {
    relay.setMode(modeVal);
    if (clrscreenFlag == 0)
    {
      lcdDisplay.clearScreen();
      lcdDisplay.centerJustifiedText("PULSE TEST PROGRESS:\0", -26); // Write permanent text
      lcdDisplay.writeScreen();
      relay.resetNcFailCount();
      relay.resetNoFailCount();
      clrscreenFlag = 1;
    }

    if (relay.pulseTest(pulseTestAmount, 25))
    {
      if (allTestFlag)
      {
        clrscreenFlag = 0;
        machineState = wait;
        break;
      }
      else
      {
        clrscreenFlag = 0;
        machineState = pulseResults;
        break;
      }
    }

    byte progress = relay.getPulseTestProgress();
    char e[4];
    itoa(progress, e, 10);
    strcat(e, "%\0");
    lcdDisplay.centerJustifiedText(e, -4);
    lcdDisplay.barGraph(relay.getPulseTestProgress(), 16);
    lcdDisplay.writeScreen();

    break;
  }

  case pulseResults:
  {

    if (clrscreenFlag == 0)
    {
      lcdDisplay.clearScreen();

      lcdDisplay.centerJustifiedText(F("PULSE TEST RESULTS"), -26);

      resultVar.ncFails = relay.getNcFailCount();
      relay.resetNcFailCount();
      // char nc1[(int)floor(log10(abs(resultVar.ncFails))) + 1];
      // itoa(resultVar.ncFails, nc1, 10);
      // lcdDisplay.centerJustifiedText(strcat("NC Failures: \0", nc1), -8);
      String stringtest;
      if (modeVal != 1)
      {
        stringtest = "NC FAILURES: ";
        stringtest += resultVar.ncFails;
        stringtest += "/";
        stringtest += pulseTestAmount;
        lcdDisplay.centerJustifiedText(stringtest, -8);
      }

      resultVar.noFails = relay.getNoFailCount();
      relay.resetNoFailCount();
      // char no1[(int)floor(log10(abs(resultVar.noFails))) + 1];
      // itoa(resultVar.noFails, no1, 10);
      // lcdDisplay.centerJustifiedText(strcat("NO Failures: \0", no1), 8);
      if (modeVal != 2)
      {
        stringtest = "NO FAILURES: ";
        stringtest += resultVar.noFails;
        stringtest += "/";
        stringtest += pulseTestAmount;
        lcdDisplay.centerJustifiedText(stringtest, 8);
      }

      lcdDisplay.writeScreen();

      clrscreenFlag = 1;
    }
    if (selButton.changed())
    {
      if (!selButton.read())
      {
        testResults temp;
        EEPROM.get(previous_test_eeprom_adress, temp);

        int a = 0;
        if (temp.ncFails != resultVar.ncFails)
          a = 1;
        if (temp.noFails != resultVar.noFails)
          a = 1;

        if (a)
        {
          EEPROM.put(previous_test_eeprom_adress, resultVar);
          // Serial.println("Writing to EEPRONM");
        }
        machineState = mainMenu;
        clrscreenFlag = 0;
      }
    }

    break;
  }

  case wait:
  {
    delay(2000);
    machineState = buzzTest;
    break;
  }

  case buzzTest:
  {
    relay.setMode(modeVal);
    if (clrscreenFlag == 0) // clear screen and write permanent text
    {
      lcdDisplay.clearScreen();
      lcdDisplay.centerJustifiedText(F("BUZZ TEST IN PROGRESS"), -26);
      lcdDisplay.centerJustifiedText(F("Current Test HZ:"), -8);
      lcdDisplay.writeScreen();
      relay.resetBuzzFailHz();
      clrscreenFlag = 1;
    }

    if (!relay.buzzTest(buzzTestMaxDuration))
    {

      time = millis();

      if (time > lastBuzzTime + 500) // update just the current HZ display every 500 MS.
      {
        int i = relay.getBuzzTestCurrentHz();
        char buzzTestHz[(int)floor(log10(abs(i))) + 1];
        itoa(i, buzzTestHz, 10);
        lcdDisplay.clearBox(50, 30, 28, 17);
        lcdDisplay.centerJustifiedText(buzzTestHz, 8);
        lcdDisplay.writeScreen();
        lastBuzzTime = time;
      }
    }
    else
    {
      if (allTestFlag)
      {
        clrscreenFlag = 0;
        machineState = allResults;
        break;
      }
      else
      {
        clrscreenFlag = 0;
        machineState = buzzResults;
        break;
      }
    }
    break;
  }

  case buzzResults:
  {
    if (clrscreenFlag == 0)
    {
      lcdDisplay.clearScreen();
      clrscreenFlag = 1;

      lcdDisplay.centerJustifiedText(F("BUZZ TEST RESULTS"), -26);

      resultVar.failHz = relay.getBuzzFailHz();
      relay.resetBuzzFailHz();
      // char hz1[(int)floor(log10(abs(hz))) + 1];
      // itoa(hz, hz1, 10);
      // lcdDisplay.centerJustifiedText(strcat("Failure HZ: \0", hz1), 0);
      String stringtest = "FAILURE HZ: ";
      stringtest += resultVar.failHz;
      lcdDisplay.centerJustifiedText(stringtest, 0);

      lcdDisplay.writeScreen();
    }
    if (selButton.changed())
    {
      if (!selButton.read())
      {
        testResults temp;
        EEPROM.get(previous_test_eeprom_adress, temp);

        int a = 0;
        if (temp.failHz != resultVar.failHz)
          a = 1;

        if (a)
        {
          EEPROM.put(previous_test_eeprom_adress, resultVar);
          // Serial.println("Writing to EEPRONM");
        }
        machineState = mainMenu;
        clrscreenFlag = 0;
      }
    }

    break;
  }

  case allResults:
  {
    if (clrscreenFlag == 0)
    {
      lcdDisplay.clearScreen();

      lcdDisplay.centerJustifiedText(F("RESULTS:"), -26);

      resultVar.ncFails = relay.getNcFailCount();
      resultVar.noFails = relay.getNoFailCount();
      resultVar.failHz = relay.getBuzzFailHz();
      relay.resetBuzzFailHz();
      relay.resetNcFailCount();
      relay.resetNoFailCount();

      if (modeVal != 1)
      {
        if (resultVar.ncFails > 0)
        {
          lcdDisplay.topRightJustifiedText(F("RELAY NC CONTACT FAIL"), 16);
        }
        else
        {
          lcdDisplay.topRightJustifiedText(F("RELAY NC CONTACT OK"), 16);
        }
      }

      if (modeVal != 2){
        if (resultVar.noFails > 0)
        {
          lcdDisplay.topRightJustifiedText(F("RELAY NO CONTACT FAIL"), 30);
        }
        else
        {
          lcdDisplay.topRightJustifiedText(F("RELAY NO CONTACT OK"), 30);
        }
    }

    String stringtest = "FAILURE HZ: ";
    stringtest += resultVar.failHz;
    lcdDisplay.topRightJustifiedText(stringtest, 44);
    // lcdDisplay.topRightJustifiedText("Failure HZ: " + String(resultVar.failHz), 44);

    lcdDisplay.writeScreen();

    clrscreenFlag = 1;
  }

    if (selButton.changed())
    {
      if (!selButton.read())
      {
        testResults temp;
        EEPROM.get(previous_test_eeprom_adress, temp);

        int a = 0;
        if (temp.ncFails != resultVar.ncFails)
          a = 1;
        if (temp.noFails != resultVar.noFails)
          a = 1;
        if (temp.failHz != resultVar.failHz)
          a = 1;

        if (a)
        {
          EEPROM.put(previous_test_eeprom_adress, resultVar);
          // Serial.println("Writing to EEPRONM");
        }
        clrscreenFlag = 0;
        allTestFlag = 0;
        machineState = mainMenu;
      }
      break;
    }
    break;
  }

case pulseSet:
{
  if (clrscreenFlag == 0)
  {
    lcdDisplay.clearScreen();
    lcdDisplay.centerJustifiedText(F("Pulses / Pulse Test:"), -26);
    lcdDisplay.centerJustifiedText(F("(100-2000)"), -15);
    clrscreenFlag = 1;
  }
  if (pulseTestAmount != lastPulseVal)
  {
    lcdDisplay.clearBox(50, 30, 28, 17);
    lcdDisplay.centerJustifiedText(String(pulseTestAmount), 8);
    lcdDisplay.writeScreen();
  }

  if (confButton.changed())
  {
    if (!confButton.read())
    {
      lastPulseVal = pulseTestAmount;
      pulseTestAmount += 100;
      if (pulseTestAmount > 2000)
      {
        pulseTestAmount = 100;
      }
      else if (pulseTestAmount < 100)
      {
        pulseTestAmount = 100;
      }
    }
  }

  if (selButton.changed())
  {
    if (!selButton.read())
    {
      clrscreenFlag = 0;
      // EEPROM.update(pulses_eeprom_adress, pulseTestAmount); // save new value to eeprom
      int temp;
      EEPROM.get(pulses_eeprom_adress, temp);
      if (temp != pulseTestAmount)
      {
        EEPROM.put(pulses_eeprom_adress, pulseTestAmount);
        Serial.print("P Writing: ");
        Serial.println(pulseTestAmount);
      }
      machineState = mainMenu;
      break;
    }
  }
  break;
}

case buzzSet:
{
  if (clrscreenFlag == 0)
  {
    lcdDisplay.clearScreen();
    lcdDisplay.centerJustifiedText(F("BUZZ TEST DURATION:"), -26);
    lcdDisplay.centerJustifiedText(F("(25-500 Seconds)"), -15);
    clrscreenFlag = 1;
  }
  if (buzzTestMaxDuration != lastBuzzVal)
  {
    lcdDisplay.clearBox(50, 30, 28, 17);
    lcdDisplay.centerJustifiedText(String(buzzTestMaxDuration), 8);
    lcdDisplay.writeScreen();
  }

  if (confButton.changed())
  {
    if (!confButton.read())
    {
      buzzTestMaxDuration += 25;
      if (buzzTestMaxDuration > 500)
      {
        buzzTestMaxDuration = 25;
      }
      else if (buzzTestMaxDuration < 25)
      {
        buzzTestMaxDuration = 25;
      }
    }
  }

  if (selButton.changed())
  {
    if (!selButton.read())
    {
      clrscreenFlag = 0;
      // EEPROM.update(buzzDuration_eeprom_adress, buzzTestMaxDuration); // save new value to eeprom
      int temp;
      EEPROM.get(buzzDuration_eeprom_adress, temp);
      if (temp != buzzTestMaxDuration)
      {
        EEPROM.put(buzzDuration_eeprom_adress, buzzTestMaxDuration);
        Serial.print("B Writing: ");
        Serial.println(buzzTestMaxDuration);
      }
      machineState = mainMenu;
      break;
    }
  }
  break;
}

case eepromResults:
{
  if (clrscreenFlag == 0)
  {
    EEPROM.get(previous_test_eeprom_adress, lastResultVar);

    lcdDisplay.clearScreen();

    lcdDisplay.centerJustifiedText(F("LAST RESULTS:"), -26);

    if (lastResultVar.ncFails > 0)
    {
      lcdDisplay.topRightJustifiedText(F("RELAY NC CONTACT FAIL"), 16);
    }
    else
    {
      lcdDisplay.topRightJustifiedText(F("RELAY NC CONTACT OK"), 16);
    }

    if (lastResultVar.noFails > 0)
    {
      lcdDisplay.topRightJustifiedText(F("RELAY NO CONTACT FAIL"), 30);
    }
    else
    {
      lcdDisplay.topRightJustifiedText(F("RELAY NO CONTACT OK"), 30);
    }

    String stringtest = "FAILURE HZ: ";
    stringtest += lastResultVar.failHz;
    lcdDisplay.topRightJustifiedText(stringtest, 44);
    // lcdDisplay.topRightJustifiedText("Failure HZ: " + String(resultVar.failHz), 44);

    lcdDisplay.writeScreen();

    clrscreenFlag = 1;
  }

  if (selButton.changed())
  {
    if (!selButton.read())
    {
      clrscreenFlag = 0;
      allTestFlag = 0;
      machineState = mainMenu;
    }
    break;
  }
  break;
}

case modeSet:
{
  if (clrscreenFlag == 0)
  {
    lcdDisplay.clearScreen();
    lcdDisplay.centerJustifiedText(F("RELAY TYPE:"), -26);
    if (modeVal == 0)
      lcdDisplay.centerJustifiedText(F("DOUBLE THROW"), 0);
    else if (modeVal == 1)
      lcdDisplay.centerJustifiedText(F("SINGLE THROW-NO"), 0);
    else if (modeVal == 2)
      lcdDisplay.centerJustifiedText(F("SINGLE THROW-NC"), 0);
    lcdDisplay.writeScreen();
    clrscreenFlag = 1;
  }

  if (modeVal != lastModeVal)
  {
    lcdDisplay.clearScreen();
    lcdDisplay.centerJustifiedText(F("RELAY TYPE:"), -26); //may have mixed terminology in here. got tired and was writing single pole instead of single throw etc
    if (modeVal == 0)
      lcdDisplay.centerJustifiedText(F("DOUBLE THROW"), 0);
    else if (modeVal == 1)
      lcdDisplay.centerJustifiedText(F("SINGLE THROW-NO"), 0);
    else if (modeVal == 2)
      lcdDisplay.centerJustifiedText(F("SINGLE THROW-NC"), 0);
    lastModeVal = modeVal;
    lcdDisplay.writeScreen();
  }

  if (confButton.changed())
  {
    if (!confButton.read())
    {
      modeVal++;
      if (modeVal > 2)
      {
        modeVal = 0;
      }
      else if (modeVal < 0)
      {
        modeVal = 0;
      }
    }
  }

  if (selButton.changed())
  {
    if (!selButton.read())
    {
      clrscreenFlag = 0;
      int temp;
      EEPROM.get(mode_eeprom_adress, temp);
      if (temp != modeVal)
      {
        EEPROM.put(mode_eeprom_adress, modeVal);
        Serial.print("M Writing: ");
        Serial.println(modeVal);
      }
      machineState = mainMenu;
    }
  }
  break;
}

default:
  break;
}
}
