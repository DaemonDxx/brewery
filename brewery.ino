#include <LiquidCrystal_I2C.h>
#include <LiquidMenu.h>
#include <TaskScheduler.h>
#include "enums.h"
#include "Heater.h"
#include "Arduino.h"
#include "HeatController.h"
#include <JC_Button.h>

#define HEATER_FULL_INTERVAL_UPDATE 15000
#define HEATER_WEAK_PIN 5
#define HEATER_STRONG_PIN 4

#define LONG_PRESS_DELAY 1000
Button left(A3, 100);
Button right(A0, 100);
Button enter(A1, 100);
Button cancelBtn(A2, 100);

Heater heat_weak(HEATER_WEAK_PIN);
Heater heat_strong(HEATER_STRONG_PIN);
HeatController controller(&heat_weak, &heat_strong);

LiquidCrystal_I2C lcd(0x26,20,4);

int pauses_delay[8] = {0, 3, 5, 6, 8, 9, 2, 5};
int pauses_temperature[8] = {0, 3, 5, 6, 8, 9, 2, 5};

double tempUp = 0;
double tempDown = 0;
double tempAverage = 0;


void start_measurment(),
     end_measurment(),
     updateHeater();

unsigned long 
  week_power = 0,
  strong_power = 0;

LiquidLine start_brew_line(1, 0, "- Start brew");
LiquidLine main_recipe_line(1, 1, "- Recipe");
LiquidLine test_line(1, 2, "- Test");
LiquidScreen main_screen(start_brew_line, main_recipe_line, test_line);
#define MAIN_SCREEN_INDEX 1

LiquidLine test_temperature_line(1,0, "T1: ", tempUp, " T2: ", tempDown);
LiquidLine test_heat_weak_line(1,1, "Power weak: ", week_power, " kW");
LiquidLine test_heat_strong_line(1,2, "Power strong: ", strong_power, " kW");
LiquidScreen test_screen(test_temperature_line, test_heat_weak_line, test_heat_strong_line);
#define TEST_SCREEN_INDEX 3

LiquidLine brew_stage_info(4,0, "Stage 1 of 5");
LiquidLine brew_temp_info(2,2, "T: 88.32C -> 88C");
LiquidLine brew_time_info(0,3, "Delay: 180:55");
LiquidScreen brew_screen(brew_stage_info, brew_temp_info, brew_time_info);

LiquidMenu menu(lcd, main_screen, test_screen, brew_screen, 1);

Scheduler task_manager;

// HEATER MODULE START

Task tHeaterUpdater(HEATER_FULL_INTERVAL_UPDATE/100, TASK_FOREVER, &updateHeater, &task_manager, true);

void updateHeater() {
   heat_weak.update();
   heat_strong.update();
}
// HEATER MODULE END

// TEMPERATURE MEASURMENT MODULE START
#include <OneWire.h>
#include <DallasTemperature.h>
#define REQUIRESALARMS
#define ONE_WIRE_BUS 11
#define DELAY_CONVERATION 800
#define INTERVAL_MEASURMENT 2000

OneWire wire(ONE_WIRE_BUS);
DallasTemperature temp_sensors(&wire);

Task tTempMeasurment(INTERVAL_MEASURMENT, TASK_FOREVER, &start_measurment, &task_manager, true);

void start_measurment() {
  if (tTempMeasurment.isFirstIteration()) {
    temp_sensors.begin();
    temp_sensors.setResolution(12);
  }
  temp_sensors.setWaitForConversion(false);
  temp_sensors.requestTemperatures();
  temp_sensors.setWaitForConversion(false);
  tTempMeasurment.setCallback(&end_measurment);
  tTempMeasurment.setInterval(DELAY_CONVERATION);
}

void end_measurment() {
  float t1 = temp_sensors.getTempCByIndex(0);
  float t2 = temp_sensors.getTempCByIndex(1);
  if (t1 == DEVICE_DISCONNECTED_C || t2 == DEVICE_DISCONNECTED_C) {
    Serial.println("Temperature Error");
  } else {
    tempUp = t1;
    tempDown = t2;
    tempAverage = (t1+t2)/2;
    Serial.print("Temprerature 1: "); Serial.println(t1);
    Serial.print("Temprerature 2: "); Serial.println(t2);
  }
  tTempMeasurment.setCallback(&start_measurment);
  tTempMeasurment.setInterval(INTERVAL_MEASURMENT - DELAY_CONVERATION);
  //controller.Update(t);
}

// TEMPERATURE MEASURMENT MODULE EN

// BUZZER MODULE START
#define START_SIGNAL_INTERVAL 200
#define START_SIGNAL_ITERATION 1

#define ERROR_SIGNAL_INTERVAL 1000
#define ERROR_SIGNAL_ITERATION 5

#define VERBOSE_SIGNAL_INTERVAL 200
#define VERBOSE_SIGNAL_ITERATION 3

#define NEXT_STAGE_SIGNAL_INTERVAL 700
#define NEXT_STAGE_SIGNAL_ITERATION 1

#define BEEP_PIN 8

void beep();

BEEP_MODE bozzer_mode = START;

Task tBuzzer(START_SIGNAL_INTERVAL, START_SIGNAL_ITERATION*2, &beep, &task_manager, true);

void beep() {
  if (tBuzzer.isFirstIteration()) {
    pinMode(BEEP_PIN, OUTPUT);
    digitalWrite(BEEP_PIN, !digitalRead(BEEP_PIN));
  }
  digitalWrite(BEEP_PIN, !digitalRead(BEEP_PIN));
}

void makeSignal(BEEP_MODE signal) {
  switch (signal) {
    case VERBOSE: 
      tBuzzer.set(VERBOSE_SIGNAL_INTERVAL, VERBOSE_SIGNAL_ITERATION*2, &beep);
      break;
    case ERROR: 
      tBuzzer.set(ERROR_SIGNAL_INTERVAL, ERROR_SIGNAL_ITERATION*2, &beep);
      break;
    case NEXT_STAGE:
      tBuzzer.set(NEXT_STAGE_SIGNAL_INTERVAL, NEXT_STAGE_SIGNAL_ITERATION, &beep);
      break;
  }
  tBuzzer.restart();
  tBuzzer.forceNextIteration();
}
// BUZZER MODULE END

// BREW MODULE START
uint8_t current_pause = 0;
uint8_t timer_minutes = 0;
uint8_t timer_second = 0;
uint8_t current_set_temp = 0;

void
    nextStage(),
    waitTemp(),
    updateTimer();

// BREW MODULE END

void mainScreenClickHandler() {
  switch (menu.get_focusedLine()) {
    case 0:

    break;

    case 1:
      menu.change_screen(2);
    break;

    case 2:
      menu.change_screen(3);
    break;

  default:
    break;
  }
}

void buttonsInit() {
  left.begin();
  right.begin();
  enter.begin();
  cancelBtn.begin();
}

void readButtons() {
  left.read();
  right.read();
  enter.read();
  cancelBtn.read();
}

void mainScreenInit() {
  start_brew_line.attach_function(0, &mainScreenClickHandler);
  main_recipe_line.attach_function(0, &mainScreenClickHandler);
  test_line.attach_function(0, &mainScreenClickHandler);
}

#define WEEK_K 10
#define STRONG_K 20

void changePowerWeek() {
  int offset = 1;
  if (enter.pressedFor(LONG_PRESS_DELAY) || cancelBtn.pressedFor(LONG_PRESS_DELAY)) {
    offset = 5;
  }
  if (cancelBtn.isPressed()) {
    offset *= -1;
  }
  int newState = heat_weak.getPower() + offset;
  heat_weak.setPower(newState);
  week_power = heat_weak.getPower() * WEEK_K;
}

void changePowerStrong() {
  int offset = 1;
  if (enter.pressedFor(LONG_PRESS_DELAY) || cancelBtn.pressedFor(LONG_PRESS_DELAY)) {
    offset = 5;
  }
  if (cancelBtn.isPressed()) {
    offset *= -1;
  }
  int newState = heat_strong.getPower() + offset;
  heat_strong.setPower(newState);
  strong_power = heat_strong.getPower() * STRONG_K;
}

void testScreenInit() {
  test_heat_weak_line.attach_function(1, changePowerWeek);
  test_heat_strong_line.attach_function(1, changePowerStrong);
}

void GUIInit() {
  lcd.init();
  lcd.backlight();
  menu.update();
  menu.set_focusPosition(Position::LEFT);
  mainScreenInit();
  testScreenInit();
}

void setup() {
  Serial.begin(115200);
  GUIInit();
  buttonsInit();
  heat_weak.on();
  heat_strong.on();
}

void mainScreenUpdate() {
  if (left.wasPressed()) {
      menu.switch_focus(false);
      menu.update();
    }
    if (right.wasPressed()) {
      menu.switch_focus(true);
      menu.update();
    }
    if (enter.wasPressed()) {
      menu.call_function(0);
    }
}

float tempAverageBefore = tempAverage;

void testScreenUpdate() {
  if (tempAverageBefore != tempAverage) {
    menu.update();
    tempAverageBefore = tempAverage;
  }
  if ((enter.wasPressed() || 
      enter.pressedFor(LONG_PRESS_DELAY) || 
      cancelBtn.pressedFor(LONG_PRESS_DELAY) || 
      cancelBtn.wasPressed()) 
      && menu.is_callable(1)) {
      menu.call_function(1);
  }
  if (left.wasPressed()) {
      menu.switch_focus(false);
      menu.update();
    }
    if (right.wasPressed()) {
      menu.switch_focus(true);
      menu.update();
    }
}

void GUIUpdate() {
  LiquidScreen *current_screen = menu.get_currentScreen();
  if (current_screen == &main_screen) {
    mainScreenUpdate();
  } else if (current_screen == &test_screen) {
    testScreenUpdate();
  }
  if (current_screen != &main_screen && cancelBtn.pressedFor(LONG_PRESS_DELAY) && !menu.is_callable(0)) {
    menu.change_screen(1);
  }
}

void loop() {
  readButtons();
  task_manager.execute();
  GUIUpdate();
}

