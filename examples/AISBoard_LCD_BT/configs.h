
//------------------------------------------------------------------------------------//
#define MQTT_CLIENT_ID "5555555" //***Important***
// 111111 2PTestLCD
// 5555555 2P101
//------------------------------------------------------------------------------------//

// Pro //

// #define MQTT_GATEWAY "itafm.aerothai.biz"
// #define MQTT_HOST_PORT 15121
// int itafmServerPort = 15111;
// #define _SERVER_TOKEN "jBzRSfACZpvCunCPzMliG5Xi1mp4pH" //pushback
// #define _SERVER_TOKEN "2lKuQGeXHDMqgx82upGrBMA0TN4vxB" //pax bus
//------------------------------------------------------------------------------------//

// DEV //

#define MQTT_GATEWAY "itafm.aerothai.biz"
#define MQTT_HOST_PORT 14883
int itafmServerPort = 14111;
// #define _SERVER_TOKEN "6qLpIYnEMk71czcVbSeDuLrTFf9uGQ" // pushback
#define _SERVER_TOKEN "3m19elAJkVjkryjUeIXkkuSRQa12jz" // pax bus
//------------------------------------------------------------------------------------//
#define MQTT_USER MQTT_CLIENT_ID
#define MQTT_PASS MQTT_CLIENT_ID
#define MQTT_TIMEOUT 10000
#define MQTT_STANDBY_TIME 60000
#define BOX_ID "\"id\":" + String(MQTT_CLIENT_ID)

// HTTP Config
char *itafmServerAddress = "itafm.aerothai.biz";

//------------------------------------------------------------------------------------//
//-------------------------Input/output parameter-------------------------------------//
//------------------------------------------------------------------------------------//

#define DAC1_pin 32 // Port DAC1 ->pin D32
#define DAC2_pin 33 // Port DAC2 ->pin D33
#define DAC3_pin 25 // Port DAC3 ->pin D25
#define DAC4_pin 26 // Port DAC4 ->pin D26
#define ADC1_pin 12 // Port ADC1 ->pin D12
#define ADC2_pin 14 // Port ADC2 ->pin D14
#define ADC3_pin 2  // Port ADC3 ->pin D2
#define ADC4_pin 25 // Port ADC4 ->pin GPIO25

#define BUZZER_PIN 5      // Speaker connect to ->pin D5
#define BUZZER_LED_PIN 27 // Speaker connect to ->pin D5
                          //------------------------------------------------------------------------------------//