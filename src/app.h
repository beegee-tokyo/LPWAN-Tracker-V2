/**
 * @file app.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief For application specific includes and definitions
 *        Will be included from app.h
 * @version 0.4
 * @date 2024-06-09
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef APP_H
#define APP_H

//**********************************************/
//** Set the application firmware version here */
//**********************************************/
// ; major version increase on API change / not backwards compatible
#ifndef SW_VERSION_1
#define SW_VERSION_1 1
#endif
// ; minor version increase on API change / backward compatible
#ifndef SW_VERSION_2
#define SW_VERSION_2 1
#endif
// ; patch version increase on bugfix, no affect on API
#ifndef SW_VERSION_3
#define SW_VERSION_3 2
#endif

#include <Arduino.h>
/** Add you required includes after Arduino.h */
#include <Wire.h>

/** Include the SX126x-API */
#include <WisBlock-API-V2.h> // Click to install library: http://librarymanager/All#WisBlock-API-V2

// Debug output set to 0 to disable app debug output
#ifndef MY_DEBUG
#define MY_DEBUG 0
#endif

#if MY_DEBUG > 0
#define MYLOG(tag, ...)                     \
	do                                      \
	{                                       \
		if (tag)                            \
			PRINTF("[%s] ", tag);           \
		PRINTF(__VA_ARGS__);                \
		PRINTF("\n");                       \
		Serial.flush();                     \
		if (g_ble_uart_is_connected)        \
		{                                   \
			g_ble_uart.printf(__VA_ARGS__); \
			g_ble_uart.printf("\n");        \
		}                                   \
	} while (0)
#define MYLOGE(tag, ...)                    \
	do                                      \
	{                                       \
		if (tag)                            \
			PRINTF("[%s] ", tag);           \
		PRINTF(__VA_ARGS__);                \
		PRINTF("\n");                       \
		Serial.flush();                     \
		if (g_ble_uart_is_connected)        \
		{                                   \
			g_ble_uart.printf(__VA_ARGS__); \
			g_ble_uart.printf("\n");        \
		}                                   \
	} while (0)

#else
#define MYLOG(...)
#define MYLOGE(...)
#endif

// Application function definitions
void setup_app(void);
bool init_app(void);
void app_event_handler(void);
void ble_data_handler(void) __attribute__((weak));
void lora_data_handler(void);

/** Application stuff */
/** Examples for application events */
#define ACC_TRIGGER 0b1000000000000000
#define N_ACC_TRIGGER 0b0111111111111111
#define GNSS_FIN 0b0100000000000000
#define N_GNSS_FIN 0b1011111111111111
#define FORCED 0b0010000000000000
#define N_FORCED 0b1101111111111111
#define OLED_OFF 0b0001000000000000
#define N_OLED_OFF 0b1110111111111111
#define SETTINGS 0b0000100000000000
#define N_SETTINGS 0b1111011111111111

// Accelerometer stuff
#include <SparkFunLIS3DH.h>
#define INT1_PIN WB_IO1 // Slot A or WB_IO3 // Slot C or WB_IO5 // Slot D or WB_IO3 // Slot C or
bool init_acc(void);
void clear_acc_int(void);
void read_acc(void);
void disable_acc(bool disable_int);
extern bool g_submit_acc;
extern bool acc_ok;

// GNSS functions
#define NO_GNSS_INIT 0
#define RAK12500_GNSS 1
#define RAK12501_GNSS 2
#include "TinyGPS++.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
bool init_gnss(void);
bool poll_gnss(void);
void gnss_task(void *pvParameters);
extern SemaphoreHandle_t g_gnss_sem;
extern TaskHandle_t gnss_task_handle;
extern volatile bool last_read_ok;
extern uint8_t gnss_option;
extern bool gnss_ok;
extern bool g_loc_high_prec;
extern volatile bool gnss_active;

// LoRaWan functions
#include <wisblock_cayenne.h>
extern WisCayenne g_data_packet;
#define LPP_ACC 64

extern uint8_t g_last_fport;

// Temperature + Humidity stuff
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
bool init_bme(void);
bool read_bme(void);
void start_bme(void);
extern bool has_env_sensor;

// GPS stuff
extern bool g_gps_prec_6;
extern bool g_is_helium;

void read_gps_settings(void);
void save_gps_settings(void);
void read_batt_settings(void);
void save_batt_settings(bool check_batt_enables);

void init_user_at(void);

extern bool battery_check_enabled;

/** Battery level uinion */
union batt_s
{
	uint16_t batt16 = 0;
	uint8_t batt8[2];
};
/** Latitude/Longitude value union */
union latLong_s
{
	uint32_t val32;
	uint8_t val8[4];
};

// OLED stuff
extern char oled_header[];
bool oled_init(void);
void oled_add_line(char *line);
void oled_show(void);
void oled_write_header(char *header_line);
void oled_clear(void);
void oled_write_line(int16_t line, int16_t y_pos, String text);
void oled_update(void);
void oled_status(void);
void oled_on_off(bool switch_on);
void oled_show_ui(uint8_t sett_scr, uint8_t highlighted, uint8_t entries);
extern bool has_oled;
extern char *ui_top[];
extern char *ui_lora[];
extern char *ui_mode[];
extern char *ui_prec[];
extern uint8_t ui_screen;
extern SoftwareTimer oled_off_timer;
extern SemaphoreHandle_t g_i2c_sem;
#define TOP_MENU 0
#define LORA_MENU 1
#define MODE_MENU 2
#define PREC_MENU 3
#define DISPLAY_MENU 4

// Button stuff
void init_button(void);
void button_handler(void);
extern volatile bool settings_ui;
extern bool screen_off;
extern bool g_display_saver;
extern uint8_t g_oled_event;
#endif
