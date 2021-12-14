#include <LiquidCrystal_I2C.h>
#include <LiquidMenu.h>
#include <TaskScheduler.h>
#include "enums.h"
#include "Heater.h"
#include "Arduino.h"
#include "HeatController.h"
#include "HeaterGroup.h"
#include <JC_Button.h>
#include "Recipe.h"
#include "Brew.h"

#define HEATER_FULL_INTERVAL_UPDATE 15000
#define HEATER_WEAK_PIN 5
#define HEATER_STRONG_PIN 4

#define LONG_PRESS_DELAY 1000

void 
    makeSignal(BEEP_MODE signal),
    start_measurment(),
    end_measurment(),
    updateHeater(),
    updatePowerInfo(),
    updateBrew();


Button left(A3, 100);
Button right(A0, 100);
Button enter(A1, 100);
Button cancelBtn(A2, 100);

Heater heat_weak(HEATER_WEAK_PIN);
Heater heat_strong(HEATER_STRONG_PIN);
HeaterGroup hGroup(&heat_weak, &heat_strong);
HeatController controller(&hGroup);
Brew brew(&controller, makeSignal);

Recipe main_recipe(4);
Recipe boiling_recipe(1);

LiquidCrystal_I2C lcd(0x26,20,4);

double tempUp = 0;
double tempDown = 0;
double tempAverage = 0;

unsigned long 
  week_power = 0,
  strong_power = 0;

uint8_t
  minutes_left = 0,
  seconds_left = 0,
  set_temperature = 0,
  stages_count = 0,
  current_stage = 0;

uint16_t
  current_power = 0;
   
LiquidLine brew_line(1, 0, "- Brew");
LiquidLine boiling_line(1, 1, "- Boiling");
LiquidLine heating_line(1, 2, "- Heating");
LiquidScreen main_screen(brew_line, boiling_line, heating_line);

LiquidLine heating_set_temperature_line(1,0, "Set T: ", set_temperature, "C");
LiquidLine heating_temperature_line(1,1, "T1: ", tempUp, " T2: ", tempDown);
LiquidLine heating_heat_weak_line(1,2, "Pweak: ", week_power, " kW");
LiquidLine heating_heat_strong_line(1,3, "Pstrong: ", strong_power, " kW");
LiquidScreen heating_screen(heating_set_temperature_line, heating_temperature_line, heating_heat_weak_line, heating_heat_strong_line);

LiquidLine brew_stage_info(4,0, "Stage ", current_stage, " of ", stages_count);
LiquidLine brew_power_info(2,1, "Power: ", current_power);
LiquidLine brew_temp_info(2,2, "T: ", tempAverage, " -> ", set_temperature);
LiquidLine brew_time_info(2,3, "Pause left: ", minutes_left, ":", seconds_left);
LiquidScreen brew_screen(brew_stage_info, brew_power_info, brew_temp_info, brew_time_info);

LiquidMenu menu(lcd, main_screen, brew_screen, heating_screen, 1);

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
  }
  tTempMeasurment.setCallback(&start_measurment);
  tTempMeasurment.setInterval(INTERVAL_MEASURMENT - DELAY_CONVERATION);
  controller.update(tempAverage);
  updatePowerInfo();
}

// TEMPERATURE MEASURMENT MODULE EN

//BREW MODULE START
#define BREW_UPDATE_INTERVAL 1000

Task tBrewUpdate(BREW_UPDATE_INTERVAL, TASK_FOREVER, &updateBrew, &task_manager, true);

void updateTime(unsigned long *time_left, uint8_t *minutes, uint8_t *seconds) {
  *minutes = floor(*time_left/60);
  *seconds = (*time_left) % 60;
}

void updateBrew() {
  if (menu.get_currentScreen() == &brew_screen) {
    brew.update();
    current_stage = brew.getCurrentStage();
    unsigned long time_left = brew.getTimeLeft();
    Recipe *current_recipe = brew.getRecipe();
    set_temperature = current_recipe -> getTemperature(current_stage);
    stages_count = current_recipe -> getStageCount();
    current_power = controller.getCurrentPower();
    updateTime(&time_left, &minutes_left, &seconds_left);
    current_stage++;
    menu.update();
  } 
}

//BREW MODULE END

// BUZZER MODULE START
#define START_SIGNAL_INTERVAL 500
#define START_SIGNAL_ITERATION 1

#define ERROR_SIGNAL_INTERVAL 1000
#define ERROR_SIGNAL_ITERATION 5

#define VERBOSE_SIGNAL_INTERVAL 200
#define VERBOSE_SIGNAL_ITERATION 3

#define NEXT_STAGE_SIGNAL_INTERVAL 700
#define NEXT_STAGE_SIGNAL_ITERATION 1

#define END_SIGNAL_INTERVAL 2000
#define END_SIGNAL_ITERATION 1

#define PAUSE_START_SIGNAL_INTERVAL 400
#define PAUSE_START_ITERATION 3

#define BEEP_PIN 9

void beep();

Task tBuzzer(START_SIGNAL_INTERVAL, START_SIGNAL_ITERATION*2, &beep, &task_manager, true);

void beep() {
  if (tBuzzer.isFirstIteration()) {
    pinMode(BEEP_PIN, OUTPUT);
    digitalWrite(BEEP_PIN, !digitalRead(BEEP_PIN));
  } else {
    digitalWrite(BEEP_PIN, !digitalRead(BEEP_PIN));
  }
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
      tBuzzer.set(NEXT_STAGE_SIGNAL_INTERVAL, NEXT_STAGE_SIGNAL_ITERATION*2, &beep);
      break;
    case END:
      tBuzzer.set(END_SIGNAL_INTERVAL, END_SIGNAL_ITERATION*2, &beep);
      break;
    case PAUSE_START:
      tBuzzer.set(PAUSE_START_SIGNAL_INTERVAL, PAUSE_START_ITERATION*2, &beep);
      break;
  }
  tBuzzer.restart();
  tBuzzer.forceNextIteration();
}
// BUZZER MODULE END

void mainScreenClickHandler() {
  switch (menu.get_focusedLine()) {
    case 0:
      menu.change_screen(2);
      controller.setMaxPower(100);
      brew.setRecipe(&main_recipe);
      brew.start();
    break;

    case 1:
      menu.change_screen(2);
      controller.setMaxPower(70);
      brew.setRecipe(&boiling_recipe);
      brew.start();
    break;

    case 2:
      menu.change_screen(3);
      controller.setTemperature(50);
      set_temperature = 50;
      controller.on();
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
  brew_line.attach_function(0, &mainScreenClickHandler);
  boiling_line.attach_function(0, &mainScreenClickHandler);
  heating_line.attach_function(0, &mainScreenClickHandler);
}

#define WEEK_K 10
#define STRONG_K 20

void updatePowerInfo() {
  week_power = heat_weak.getPower() * WEEK_K;
  strong_power = heat_strong.getPower() * STRONG_K;
}

void changePowerWeek() {
  if (controller.isOn()) {
    controller.off();
  }
  if (!heat_weak.isOn()) {
    heat_weak.on();
  }
  int offset = 1;
  if (enter.pressedFor(LONG_PRESS_DELAY) || cancelBtn.pressedFor(LONG_PRESS_DELAY)) {
    offset = 5;
  }
  if (cancelBtn.isPressed()) {
    offset *= -1;
  }
  int newState = heat_weak.getPower() + offset;
  heat_weak.setPower(newState);
  updatePowerInfo();
}

void changePowerStrong() {
  if (controller.isOn()) {
    controller.off();
  }
  if (!heat_strong.isOn()) {
    heat_strong.on();
  }
  int offset = 1;
  if (enter.pressedFor(LONG_PRESS_DELAY) || cancelBtn.pressedFor(LONG_PRESS_DELAY)) {
    offset = 5;
  }
  if (cancelBtn.isPressed()) {
    offset *= -1;
  }
  int newState = heat_strong.getPower() + offset;
  heat_strong.setPower(newState);
  updatePowerInfo();
}

void changeSetTemperature() {
  if (!controller.isOn()) {
    controller.on();
  } 
  int offset = 1;
  if (enter.pressedFor(LONG_PRESS_DELAY) || cancelBtn.pressedFor(LONG_PRESS_DELAY)) {
    offset = 5;
  }
  if (cancelBtn.isPressed()) {
    offset *= -1;
  }
  int newState = controller.getTemperature() + offset;
  controller.setTemperature(newState);
  set_temperature = controller.getTemperature();
}

void heatingScreenInit() {
  heating_heat_weak_line.attach_function(1, changePowerWeek);
  heating_heat_strong_line.attach_function(1, changePowerStrong);
  heating_set_temperature_line.attach_function(1, changeSetTemperature);
}

void GUIInit() {
  lcd.init();
  lcd.backlight();
  menu.update();
  menu.set_focusPosition(Position::LEFT);
  mainScreenInit();
  heatingScreenInit();
}

void recipeInit() {
  main_recipe.setStage(0, 900, 55);
  main_recipe.setStage(1, 3000, 63);
  main_recipe.setStage(2, 1200, 72);
  main_recipe.setStage(3, 300, 78);

  boiling_recipe.setStage(0, 3000, 100);
}

void setup() {
  Serial.begin(115200);
  GUIInit();
  buttonsInit();
  recipeInit();
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

void heatingScreenUpdate() {
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

void brewScreenUpdate() {
  if (right.wasPressed()) {
    brew.nextStage();
  }
}

void GUIUpdate() {
  LiquidScreen *current_screen = menu.get_currentScreen();
  if (current_screen == &main_screen) {
    mainScreenUpdate();
  } else if (current_screen == &heating_screen) {
    heatingScreenUpdate();
  } else if (current_screen == &brew_screen) {
    brewScreenUpdate();
  }
  if (current_screen != &main_screen && cancelBtn.pressedFor(LONG_PRESS_DELAY) && !menu.is_callable(0)) {
    if (current_screen == &brew_screen) {
      brew.cancel();
    } else if (current_screen == &heating_screen) {
      controller.off();
      heat_strong.setPower(0);
      heat_strong.off();
      heat_weak.setPower(0);
      heat_weak.off();
    }
    menu.change_screen(1);
  }
}

void loop() {
  readButtons();
  task_manager.execute();
  GUIUpdate();
}

