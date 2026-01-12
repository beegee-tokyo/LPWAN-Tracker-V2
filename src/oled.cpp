/**
 * @file RAK1921_oled.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialization and usage of RAK19211.3" OLED
 * @version 0.4
 * @date 2024-06-09
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "app.h"
// #include <nRF_SH1106Wire.h>
#include <nRF_SSD1306Wire.h>

void disp_show(void);

/** Width of the display in pixel */
#define OLED_WIDTH 128
/** Height of the display in pixel */
#define OLED_HEIGHT 64
/** Height of the status bar in pixel */
#define STATUS_BAR_HEIGHT 11
/** Height of a single line */
#define LINE_HEIGHT 10

/** Number of message lines */
#define NUM_OF_LINES (OLED_HEIGHT - STATUS_BAR_HEIGHT) / LINE_HEIGHT

/** Line buffer for messages */
char disp_buffer[NUM_OF_LINES + 1][32] = {0};

/** Current line used */
uint8_t current_line = 0;

/** Display class using Wire */
// SH1106Wire oled_display(0x3c, PIN_WIRE_SDA, PIN_WIRE_SCL, GEOMETRY_128_64, &Wire);
SSD1306Wire oled_display(0x3c, PIN_WIRE_SDA, PIN_WIRE_SCL, GEOMETRY_128_64, &Wire);

/** Timer for display off */
SoftwareTimer oled_off_timer;

void oled_off_cb(TimerHandle_t unused);

bool g_display_saver = false;

/** UI entries for top level */
char *ui_top[] = {(char *)"BACK", (char *)"LoRaWAN/LoRa", (char *)"Packet Mode", (char *)"Location Precision", (char *)"Display"};
/** UI entries for LoRa selection */
char *ui_lora[] = {(char *)"BACK", (char *)"LoRaWAN", (char *)"LoRa P2P"};
/** UI entries for packet mode */
char *ui_mode[] = {(char *)"BACK", (char *)"CayenneLPP 4 digit", (char *)"CayenneLPP 6 digit", (char *)"Helium"};
/** UI entries for precision settings */
char *ui_prec[] = {(char *)"BACK", (char *)"any fix", (char *)"6 Sat, ACCU < 2.5"};
/** UI entries for display power saver */
char *ui_disp[] = {(char *)"BACK", (char *)"Display saver on", (char *)"Display saver off"};
/** UI screen selected 0 = top, 1 = LoRa, 2 = Mode, 3 = ACCU, 4 = Display saver */
uint8_t ui_screen = 0;

/**
 * @brief Initialize the display
 *
 * @return true always
 * @return false never
 */
bool oled_init(void)
{
	Wire.begin();

	delay(500); // Give display reset some time
	oled_display.init();
	oled_display.displayOff();
	oled_display.clear();
	oled_display.displayOn();
#if _RAK19026_ == 1
	MYLOG("DISP", "Rotate display");
	oled_display.flipScreenVertically();
#endif
	oled_display.setContrast(128);
	oled_display.setFont(ArialMT_Plain_10);
	oled_display.display();

	// Set delayed sending to 1/2 of programmed send interval or 30 seconds
	oled_off_timer.begin(120000, oled_off_cb, NULL, false);
	if (g_display_saver)
	{
		MYLOG("DISP", "Enable display off timer");
		oled_off_timer.start();
	}
	else
	{
		MYLOG("DISP", "Disable display off timer");
		oled_off_timer.stop();
	}
	return true;
}

/**
 * @brief Write the top line of the display
 */
void oled_write_header(char *header_line)
{
	oled_display.setFont(ArialMT_Plain_10);

	// clear the status bar
	oled_display.setColor(BLACK);
	oled_display.fillRect(0, 0, OLED_WIDTH, STATUS_BAR_HEIGHT + 1);

	oled_display.setColor(WHITE);
	oled_display.setTextAlignment(TEXT_ALIGN_LEFT);

	oled_display.drawString(0, 0, header_line);

	uint8_t usbStatus = NRF_POWER->USBREGSTATUS;
	uint16_t len = 0;
	char oled_line[64];
	switch (usbStatus)
	{
	case 0:
	{
		MYLOG("OLED", "Writing battery");
		float bat = read_batt();
		bat = 0.0;
		for (int idx = 0; idx < 10; idx++)
		{
			bat += read_batt();
		}
		bat = bat / 10000.0;

		len = sprintf(oled_line, "%.2fV", bat);
		oled_display.drawString(125 - (oled_display.getStringWidth(oled_line, len)), 0, oled_line);
		break;
	}
	case 3: // VBUS voltage above valid threshold and USBREG output settling time elapsed (same information as USBPWRRDY event)
	{
		len = sprintf(oled_line, "%s", "USB");
		MYLOG("OLED", "Writing USB: %s, len = %d", oled_line, len);
		oled_display.drawString(125 - (oled_display.getStringWidth(oled_line, len)), 0, oled_line);
		break;
	}
	default:
		break;
	}

	// char battery[20];
	// float bat = read_batt();
	// bat = 0.0;
	// for (int idx = 0; idx < 10; idx++)
	// {
	// 	bat += read_batt();
	// }
	// bat = bat / 10 / 1000;
	// uint16_t len = sprintf(battery, "%.2fV", bat);
	// oled_display.drawString(125 - (oled_display.getStringWidth(battery, len)), 0, battery);

	// draw divider line
	oled_display.drawLine(0, 11, 125, 11);
	oled_display.display();
}

/**
 * @brief Add a line to the display buffer
 *
 * @param line Pointer to char array with the new line
 */
void oled_add_line(char *line)
{
	if (current_line == NUM_OF_LINES)
	{
		// Display is full, shift text one line up
		for (int idx = 0; idx < NUM_OF_LINES; idx++)
		{
			memcpy(disp_buffer[idx], disp_buffer[idx + 1], 32);
		}
		current_line--;
	}
	snprintf(disp_buffer[current_line], 32, "%s", line);

	if (current_line != NUM_OF_LINES)
	{
		current_line++;
	}

	oled_show();
}

/**
 * @brief Update display messages
 *
 */
void oled_show(void)
{
	oled_display.setColor(BLACK);
	oled_display.fillRect(0, STATUS_BAR_HEIGHT + 1, OLED_WIDTH, OLED_HEIGHT);

	oled_display.setFont(ArialMT_Plain_10);
	oled_display.setColor(WHITE);
	oled_display.setTextAlignment(TEXT_ALIGN_LEFT);
	for (int line = 0; line < current_line; line++)
	{
		oled_display.drawString(0, (line * LINE_HEIGHT) + STATUS_BAR_HEIGHT + 1, disp_buffer[line]);
	}
	oled_display.display();
}

/**
 * @brief Clear the display
 *
 */
void oled_clear(void)
{
	oled_display.setColor(BLACK);
	oled_display.fillRect(0, STATUS_BAR_HEIGHT + 1, OLED_WIDTH, OLED_HEIGHT);
	current_line = 0;
}

/**
 * @brief Write a line at given position
 *
 * @param x_pos horizontal offset
 * @param y_pos vertical offset
 * @param text String text
 */
void oled_write_line(int16_t x_pos, int16_t y_pos, String text)
{
	if (screen_off)
	{
		return;
	}
	oled_display.setFont(ArialMT_Plain_10);
	oled_display.drawString(x_pos, y_pos, text);
}

/**
 * @brief Trigger OLED power down
 *
 * @param unused
 */
void oled_off_cb(TimerHandle_t unused)
{
	api_wake_loop(OLED_OFF);
}

/**
 * @brief Show device status in OLED
 *
 */
void oled_status(void)
{
	oled_clear();
	if (g_gps_prec_6 || g_is_helium)
	{
		oled_add_line((char *)"GNSS 6 digit prec");
	}
	else
	{
		oled_add_line((char *)"GNSS 4 digit prec");
	}
	if (g_loc_high_prec)
	{
		oled_add_line((char *)"GNSS fix + 6 sat prec");
	}
	if (g_is_helium)
	{
		oled_add_line((char *)"Helium Mapper");
	}
	else
	{
		if (g_lorawan_settings.lorawan_enable)
		{
			oled_add_line((char *)"LoRaWAN Tracker");
		}
		else
		{
			oled_add_line((char *)"LoRa Tracker");
		}
	}
	if (g_lpwan_has_joined)
	{
		oled_add_line((char *)"Connected");
	}
	else
	{
		oled_add_line((char *)"Connecting ....");
	}
	oled_show();
}

/**
 * @brief Switch OLED on/off
 *
 * @param switch_on TRUE ==> ON, FALSE ==> OFF
 */
void oled_on_off(bool switch_on)
{
	if (switch_on)
	{
		screen_off = false;
		oled_display.displayOn();
		oled_write_header(oled_header);
		oled_show();
	}
	else
	{
		screen_off = true;
		oled_display.displayOff();
	}
}

/** Temporary buffer for display text */
char ui_buff[128];

/**
 * @brief UI display handler
 *
 * @param sett_scr which menu to show
 * @param highlighted which item to highlight
 * @param entries how many items in the menu
 */
void oled_show_ui(uint8_t sett_scr, uint8_t highlighted, uint8_t entries)
{
	char **use_menu;

	switch (sett_scr)
	{
	case 0:
		use_menu = ui_top;
		break;
	case 1:
		use_menu = ui_lora;
		break;
	case 2:
		use_menu = ui_mode;
		break;
	case 3:
		use_menu = ui_prec;
		break;
	case 4:
		use_menu = ui_disp;
		break;
	}

	oled_clear();
	if (g_is_helium)
	{
		snprintf(oled_header, 127, "Helium Mapper");
	}
	else
	{
		snprintf(oled_header, 127, "%s Tracker", g_lorawan_settings.lorawan_enable ? "LPWAN" : "LoRa P2P");
	}
	oled_write_header(oled_header);
	for (uint8_t idx = 0; idx < entries; idx++)
	{
		if (highlighted == idx)
		{
			sprintf(ui_buff, "(X) %s", use_menu[idx]);
		}
		else
		{
			sprintf(ui_buff, "(%d) %s", idx + 1, use_menu[idx]);
		}
		oled_add_line(ui_buff);
	}
	oled_show();
}
