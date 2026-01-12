/**
 * @file button.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialization and handling of button events
 * @version 0.1
 * @date 2024-06-08
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "app.h"
#include "OneButton.h"

/** Button GPIO */
#define BUTTON_INT WB_IO5

/** Timer for VOC measurement */
SoftwareTimer button_check;

// Flag if button timer is already running
bool timer_running = false;

/** Flag if settings UI is active */
volatile bool settings_ui = false;

/** Flag if display is on or off */
bool screen_off = false;

/** Selected item in UI */
uint8_t selected_item = 0;

/** Flag for button event */
uint8_t g_oled_event = 0;

/**
 * @brief Button instance
 * 		First parameter is interrupt input for button
 * 		Second parameter defines button pushed states:
 * 			true if active low (LOW if pressed)
 * 			false if active high (HIGH if pressed)
 */
OneButton button(BUTTON_INT, true);

void ui_update(void)
{
	if (g_is_helium)
	{
		snprintf(oled_header, 127, "Helium Mapper");
	}
	else
	{
		snprintf(oled_header, 127, "%s Tracker", g_lorawan_settings.lorawan_enable ? "LPWAN" : "LoRa");
	}
	oled_clear();
	oled_write_header(oled_header);
	oled_status();
	settings_ui = false;
}

/**
 * @brief Button interrupt callback
 * 		calls button.tick() to process status
 * 		start a timer to frequently repeat button status process until event is handled
 *
 */
void checkTicks(void)
{
	// Button interrupt, call tick()
	button.tick();
	// If not already running, start the timer to frequently check the button status
	if (!timer_running)
	{
		timer_running = true;
		button_check.start();
	}
}

/**
 * @brief Single click has dual functions
 * - when outside settings, toggle to settings screen
 * - when inside settings, select menu item 1
 *
 */
void singleClick(void)
{
	if (g_display_saver)
	{
		oled_off_timer.reset();
	}
	button_check.stop();
	timer_running = false;

	// Handle in main loop
	g_oled_event = 1;
	api_wake_loop(SETTINGS);
}

/**
 * @brief Double click has dual functions
 * - when outside settings, do nothing
 * - when inside settings, select current item
 *
 */
void doubleClick(void)
{
	if (g_display_saver)
	{
		oled_off_timer.reset();
	}
	button_check.stop();
	timer_running = false;

	// Handle in main loop
	g_oled_event = 2;
	api_wake_loop(SETTINGS);
}

/**
 * @brief Multi click detection is not used (yet)
 *
 */
void multiClick(void)
{
	if (g_display_saver)
	{
		oled_off_timer.reset();
	}
	button_check.stop();
	timer_running = false;

	// Handle in main loop
	uint8_t tick_num = button.getNumberClicks();
	g_oled_event = tick_num;
	api_wake_loop(SETTINGS);

	MYLOG("BTN", "multiClick(%d) detected.", tick_num);
}

bool long_press_processed = false;

void longPressCheck(void *oneButton)
{
	// MYLOG("BTN", "LP %ld ms", ((OneButton *)oneButton)->getPressedMs());
	if ((((OneButton *)oneButton)->getPressedMs() > 1500) && !long_press_processed)
	{
		long_press_processed = true;
		MYLOG("BTN", "Display on/off detected");
		button_check.stop();
		timer_running = false;
		g_oled_event = 9;
		api_wake_loop(SETTINGS);
	}
}

void longPressStop(void *oneButton)
{
	MYLOG("BTN", "LP %ld ms", ((OneButton *)oneButton)->getPressedMs());
	long_press_processed = false;
}

/**
 * @brief Timer callback after a button push event was detected.
 * 		Needed to continue to check the button status
 *
 * @param unused
 */
void check_button(TimerHandle_t unused)
{
	button.tick();
}

void init_button(void)
{
	// Setup interrupt routine
	attachInterrupt(digitalPinToInterrupt(BUTTON_INT), checkTicks, CHANGE);

	// Setup the different callbacks for button events
	button.attachClick(singleClick);
	button.attachDoubleClick(doubleClick);
	button.attachMultiClick(multiClick);
	button.setLongPressIntervalMs(500);
	button.attachDuringLongPress(longPressCheck, &button);
	button.attachLongPressStop(longPressStop, &button);

	// Create timer for button handling
	button_check.begin(10, check_button, NULL, true);
}

/**
 * @brief Handle button events, called from app_event_handler
 * to avoid conflicts in I2C usage
 *
 */
void button_handler(void)
{
	switch (g_oled_event)
	{
	case 1: // Single Click
		if (!settings_ui)
		{
		}
		else
		{
			MYLOG("BTN", "UI screen %d", ui_screen);
			// UI top level ? ==> go back
			if (ui_screen == 0)
			{
				ui_update();
				ui_screen = 0;
				settings_ui = false;
				disable_acc(false);
				api_timer_restart(g_lorawan_settings.send_repeat_time);
			}
			// UI LoRa settings ? ==> go back
			else if (ui_screen == 1)
			{
				oled_show_ui(TOP_MENU, 255, 5);
				ui_screen = 0;
			}
			// UI Mode settings ? ==> ==> go back
			else if (ui_screen == 2)
			{
				oled_show_ui(TOP_MENU, 255, 5);
				ui_screen = 0;
			}
			// UI Precission settings ? ==> go back
			else if (ui_screen == 3)
			{
				oled_show_ui(TOP_MENU, 255, 5);
				ui_screen = 0;
			}
			// UI Display settings ? ==> go back
			else if (ui_screen == 4)
			{
				// oled_show_top_ui();
				oled_show_ui(TOP_MENU, 255, 5);
				ui_screen = 0;
			}
		}
		break;
	case 2: // Double Click
		if (settings_ui)
		{
			MYLOG("BTN", "UI screen %d", ui_screen);
			// UI top level ?
			if (ui_screen == 0)
			{
				selected_item = g_lorawan_settings.lorawan_enable ? 1 : 2;
				MYLOG("BTN", "Selected %d", selected_item);
				oled_show_ui(LORA_MENU, selected_item, 3);
				ui_screen = 1;
			}
			// UI LoRa settings ? ==> Set LoRaWAN
			else if (ui_screen == 1)
			{
				if (g_lorawan_settings.lorawan_enable)
				{
					// Do nothing, already LoRaWAN
				}
				else
				{
					MYLOG("BTN", "Switch to LoRaWAN");
					g_lorawan_settings.lorawan_enable = true;
					save_settings();
					api_reset();
				}
			}
			// UI Mode settings ? ==> Set 4-digit Cayenne LPP mode
			else if (ui_screen == 2)
			{
				if (g_gps_prec_6 || g_is_helium)
				{
					MYLOG("BTN", "Switch to 4 digit precision");
					g_gps_prec_6 = false;
					g_is_helium = false;
					save_gps_settings();
					oled_show_ui(MODE_MENU, 1, 4);
				}
			}
			// UI Precission settings ? ==> Select low precision
			else if (ui_screen == 3)
			{
				if (!g_is_helium)
				{
					MYLOG("BTN", "Switch to low precision");
					g_loc_high_prec = false;
					save_gps_settings();
					oled_show_ui(PREC_MENU, 1, 3);
				}
			}
			// UI Display settings ? ==> Display saver on
			else if (ui_screen == 4)
			{
				MYLOG("BTN", "Enable display saver");
				g_display_saver = true;
				oled_off_timer.start();
				g_display_saver = true;
				oled_show_ui(DISPLAY_MENU, 1, 3);
			}
		}
		else
		{
			if (!gnss_active)
			{
				disable_acc(true);
				api_timer_stop();
				if (screen_off)
				{
					oled_on_off(true);
				}
				settings_ui = true;
				oled_show_ui(TOP_MENU, 255, 5);
				ui_screen = 0;
			}
			else
			{
				MYLOGE("BTN", "GNSS still active");
			}
		}
		break;
	case 3: // Three Clicks
		if (settings_ui)
		{
			MYLOG("BTN", "UI screen %d", ui_screen);
			// UI top level ?
			if (ui_screen == 0)
			{
				if (g_is_helium)
				{
					selected_item = 3;
				}
				else
				{
					selected_item = g_gps_prec_6 ? 2 : 1;
				}
				MYLOG("BTN", "Selected %d", selected_item);
				oled_show_ui(MODE_MENU, selected_item, 4);
				ui_screen = 2;
			}
			// UI LoRa settings ? ==> Set LoRa P2P
			else if (ui_screen == 1)
			{
				if (!g_lorawan_settings.lorawan_enable)
				{
					// Do nothing, already LoRa P2P
				}
				else
				{
					MYLOG("BTN", "Switch to LoRa P2P");
					g_lorawan_settings.lorawan_enable = false;
					save_settings();
					api_reset();
				}
			}
			// UI Mode settings ? ==> Set 6-digit Cayenne LPP mode
			else if (ui_screen == 2)
			{
				if (!g_gps_prec_6)
				{
					MYLOG("BTN", "Switch to 6 digit precision");
					g_gps_prec_6 = true;
					g_is_helium = false;
					save_gps_settings();
					oled_show_ui(MODE_MENU, 2, 4);
				}
			}
			// UI Precission settings ? ==> Select low precision
			else if (ui_screen == 3)
			{
				if (!g_loc_high_prec)
				{
					MYLOG("BTN", "Switch to high precision");
					g_loc_high_prec = true;
					save_gps_settings();
					oled_show_ui(PREC_MENU, 2, 3);
				}
			}
			// UI Display settings ? ==> Display saver off
			else if (ui_screen == 4)
			{
				MYLOG("BTN", "Disable display saver");
				oled_off_timer.stop();
				g_display_saver = false;
				oled_show_ui(DISPLAY_MENU, 2, 3);
			}
		}
		else
		{
			api_wake_loop(FORCED);
		}
		break;
	case 0x04: // Four Clicks
		if (settings_ui)
		{
			MYLOG("BTN", "UI screen %d", ui_screen);
			// UI top level ?
			if (ui_screen == 0)
			{
				selected_item = g_loc_high_prec ? 2 : 1;
				MYLOG("BTN", "Selected %d", selected_item);
				oled_show_ui(PREC_MENU, selected_item, 3);
				ui_screen = 3;
			}
			// UI Mode settings ? ==> Set Helium mode
			else if (ui_screen == 2)
			{
				if (!g_is_helium)
				{
					MYLOG("BTN", "Switch to Helium");
					g_gps_prec_6 = false;
					g_is_helium = true;
					save_gps_settings();
					oled_show_ui(MODE_MENU, 3, 4);
				}
			}
			// UI Precission settings ? ==> no 4 clicks
			else if (ui_screen == 3)
			{
			}
			// UI Display settings ? ==> no 4 clicks
			else if (ui_screen == 4)
			{
			}
		}
		break;
	case 0x05: // Five Clicks
		if (settings_ui)
		{
			// UI top level ? ==> display saver
			if (ui_screen == 0)
			{
				oled_show_ui(DISPLAY_MENU, g_display_saver ? 1 : 2, 3);
				ui_screen = 4;
			}
		}
		break;
	case 6:				  // Six Clicks
		if (g_enable_ble) // If BLE is enabled, restart Advertising
		{
			MYLOG("BTN", "BLE On.");
			restart_advertising(15);
		}
		break;
	case 7: // Seven Clicks
		if (screen_off)
		{
			oled_on_off(true);
		}
		oled_clear();
		oled_write_header((char *)"RESET");
		oled_show();
		delay(1000);
		MYLOGE("BTN", "RST request");
		api_reset();
		break;
	case 8: // Eight Clicks
		if (screen_off)
		{
			oled_on_off(true);
		}
		oled_clear();
		oled_write_header((char *)"BOOLOADER MODE");
		oled_show();
		delay(1000);
		MYLOGE("BTN", "Bootloader request");
		NRF_POWER->GPREGRET = 0x57; // 0xA8 OTA, 0x4e Serial, 0x57 UF2
		NVIC_SystemReset();			// or sd_nvic_SystemReset();
		break;
	case 9: // Long Press
		long_press_processed = true;
		button_check.stop();
		timer_running = false;
		if (screen_off)
		{
			oled_on_off(true);
			oled_off_timer.start();
			g_display_saver = true;
			screen_off = false;
		}
		else
		{
			oled_on_off(false);
			screen_off = true;
		}
		break;
	}
}