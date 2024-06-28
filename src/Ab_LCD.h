#ifndef AB_LCD_H
#define AB_LCD_H
#include "stone.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <string.h>

extern unsigned char receive_over_flage;
extern recive_group STONER;
extern unsigned char STONE_RX_BUF[RX_LEN];

class SerialLCD
{
private:
  void set_display_data(uint8_t page);
  void Initial_setup();
  uint8_t maxJob = 0;
  uint8_t currentJobPage = 0;
  uint8_t buttonsPerPage = 0;
  uint8_t numPages = 0;
  uint8_t roundTask = 4;

  bool radioButtonChecker = false;

  unsigned long startTime = 0;
  unsigned long countdownDuration = 5000;
  uint8_t _prevPages = 0;
  bool isButtonPageWidget(const char *widgetName);
  unsigned long visibilityStartTime = 0;
  bool visibilityInProgress = false;
  unsigned long visibilityDuration = 2000;

  unsigned long loadingStartTime = 0;
  bool loadingInProgress = false;
  unsigned long loadingDuration = 4000;

  String HMI_version = "";

public:
  void page0();
  void page1();
  void page2();
  void page3();
  void page4();
  void page5();
  void page9();

  void refreshData();

  void popup_reconnecting_on();
  void popup_reconnecting_off();

  uint8_t page = 0;
  uint8_t job_step = 0;
  uint8_t currentRound = 0;
  uint8_t currentRound_offset = 0;
  uint8_t maxStep = (roundTask * 2) + 1;

  bool is4G;
  bool isSensor;
  bool isServer;
  bool isRFID;
  bool isTemp;
  bool isGPS;

  bool isLogin = false;
  bool driverLoginFailed = false;

  bool isSelectFlight = false;
  bool isSelectFlight_Ok = false;

  bool isStepAction = false;
  bool isStepAction_Ok = false;

  bool isUndoAction = false;
  bool isUndoAction_Ok = false;

  bool isCancelTask = false;
  bool isCancelTask_Ok = false;

  bool isInitalTaskReady = false;

  bool sleepInProgress = true;

  bool updateDisplay = false;

  String Datetime;
  String GSEId;
  String Flight;
  String ST;
  String Bay;
  String ET;
  String Gate;
  String Driver;
  String employeeId;
  String taskId;
  String unitName;
  // Preview FIGHTLIST

  String flight_list = "";
  String flight_type = "";
  uint8_t flight_list_size = 0;
  String selected_flight = "";
  bool recheck_flight_list = false;
  char timer_flag = 0;

  unsigned long timeOutStartTime = 0;
  bool timeOutInProgress = false;
  unsigned long timeOutDuration = 4000;

  // task page
  String task_gate;
  String task_bay;
  String task_st;
  String task_et;
  String task_flight;
  int8_t task_step = -1;
};

#endif
