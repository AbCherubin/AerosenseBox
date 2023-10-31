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
  uint8_t maxJob = 0;
  uint8_t currentJobPage = 0;
  uint8_t buttonsPerPage = 0;
  uint8_t numPages = 0;
  unsigned long startTime = 0;
  unsigned long countdownDuration = 5000;
  uint8_t _prevPages = 0;
  bool isButtonPageWidget(const char *widgetName);
  unsigned long visibilityStartTime = 0;
  bool visibilityInProgress = false;
  unsigned long visibilityDuration = 2000;

  unsigned long loadingStartTime = 0;
  bool loadingInProgress = false;
  unsigned long loadingDuration = 3000;

public:
  void page0();
  void page1();
  void page2();
  void page3();
  void page4();
  void page5();
  void popup_loading_on();
  void popup_loading_off();
  void refreshData();
  uint8_t page = 0;
  uint8_t job_step = 0;

  bool is4G;
  bool isSensor;
  bool isServer;
  bool isRFID;
  bool isTemp;
  bool isGPS;

  bool isLogin = false;
  bool driverLoginFailed = false;

  String Datetime;
  String GSEId;
  String Username;
  String Flight;
  String STD;
  String Bay;
  String ETD;
  String Gate;
  String Driver;
  // Preview FIGHTLIST

  String flight_list = "";
  uint8_t flight_list_size = 0;
  String selected_flight = "";
  bool recheck_flight_list = false;
  char timer_flag = 0;
};

#endif
