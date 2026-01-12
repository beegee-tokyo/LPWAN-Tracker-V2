# WisBlock Tracker Solution V2
| <img src="./assets/RAK-Whirls.png" alt="Modules" width="150"> | <img src="./assets/rakstar.jpg" alt="RAKstar" width="100"> |    
| :-: | :-: |     
This is the source code for the WisBlock Tracker Solution with RAK12500 or RAK12501 GNSS module, and optional RAK1904 acceleration sensor and/or RAK1906 environment sensor

## _REMARK 1_
Recommended WisBlock modules
- [WisBlock Starter Kit](https://store.rakwireless.com/collections/kits-bundles/products/wisblock-starter-kit)
- alternative [WisMesh RAK4631 Starter Kit](https://store.rakwireless.com/collections/kits-bundles/products/wisblock-starter-kit) (with optional RAK12501 and RAK1910 modules) flashed with this firmware.
- alternative [WisMesh Board ONE](https://store.rakwireless.com/products/wismesh-board-one-meshtastic-node) (with optional RAK12501 and OLED modules) flashed with this firmware.
- alternative [WisMesh Base Board](https://store.rakwireless.com/products/wismesh-baseboard-rak19026) (with GNSS and OLED modules) flashed with this firmware.
- [RAK12500](https://store.rakwireless.com/collections/wisblock-sensor/products/wisblock-gnss-location-module-rak12500)
- alternative [RAK12501](https://store.rakwireless.com/products/wisblock-rak12501-gnss-module) WisBlock Sensor GNSS module
- [RAK1904](https://store.rakwireless.com/collections/wisblock-sensor/products/rak1904-lis3dh-3-axis-acceleration-sensor)
- optional [RAK1906](https://store.rakwireless.com/collections/wisblock-sensor/products/rak1906-bme680-environment-sensor)
- [Unify Enclosure with solar panel](https://store.rakwireless.com/products/unify-enclosure-ip65-100x75x38-solar) (For Wisblock Starter Kit and WisMesh RAK4631 Starter Kit)
- alternative [3D printed enclosure for WisMesh Board ONE](https://makerworld.com/en/models/1631878-rakwireless-wismesh-board-one#profileId-1723640)
- alternative [3D printed enclosure for WisMesh Pocket V2](https://makerworld.com/en/models/1678035-rakwireless-wismesh-pocket-mine#profileId-1777368)

## _REMARK 2_
This example is using the [WisBlock API](https://github.com/beegee-tokyo/WisBlock-API) which helps to create low power consumption application and taking the load to handle communications from your shoulder.

----

# Hardware used in PoC
- [RAK4631](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Overview/) WisBlock Core module
- [RAK19007](https://docs.rakwireless.com/product-categories/wisblock/rak19007/overview/) WisBlock Base board
- [RAK12501](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12501/Overview/) WisBlock Sensor GNSS module
- alternative [RAK12500](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12500/Overview/) WisBlock Sensor GNSS module
- optional [RAK1904](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1904/Overview/) WisBlock Sensor acceleration module
- optional [RAK1906](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1906/Overview/) WisBlock Sensor environment module

Optional and alternative modules are tested as well.

## Power consumption
The application does switch off the GPS module and the MCU and LoRa transceiver go into sleep mode between measurement cycles to save power. I could measure a sleep current of 40uA of the whole system. 

----

# Software used
- [PlatformIO](https://platformio.org/install)
- [Adafruit nRF52 BSP](https://docs.platformio.org/en/latest/boards/nordicnrf52/adafruit_feather_nrf52832.html)
- [Patch to use RAK4631 with PlatformIO](https://github.com/RAKWireless/WisBlock/blob/master/PlatformIO/RAK4630/README.md)
- [SX126x-Arduino LoRaWAN library](https://github.com/beegee-tokyo/SX126x-Arduino)
- [SparkFun u-blox GNSS Arduino Library](https://platformio.org/lib/show/11715/SparkFun%20u-blox%20GNSS%20Arduino%20Library)
- [TinyGPSPlus](https://registry.platformio.org/libraries/mikalhart/TinyGPSPlus)
- [Adafruit BME680 Library](https://platformio.org/lib/show/1922/Adafruit%20BME680%20Library)
- [WisBlock API](https://github.com/beegee-tokyo/WisBlock-API)
- [CayenneLPP](https://registry.platformio.org/libraries/sabas1080/CayenneLPP)

## _REMARK_
The libraries are all listed in the **`platformio.ini`** and are automatically installed when the project is compiled.

----

# Setting up LoRaWAN credentials
The LoRaWAN settings can be defined in two different ways. 
- Over USB with [AT Commands](./AT-Commands.md)
- Hardcoded in the sources (_**ABSOLUTELY NOT RECOMMENDED**_)

## 1) Setup over USB port
Using the AT command interface the WisBlock can be setup over the USB port.

The AT command interface is compatible with the [RUI3 AT commands](https://docs.rakwireless.com/product-categories/software-apis-and-libraries/rui3/at-command-manual/).     
The AT command interface is exended with custom AT commands to setup the GNSS behaviour. A detailed manual for the custom AT commands are in [AT-Commands.md](./AT-Commands.md)

Here is an example for the typical AT commands required to get the device ready (EUI's and Keys are examples):
```log
// Setup AppEUI
AT+APPEUI=70b3d57ed00201e1
// Setup DevEUI
AT+DEVEUI=ac1f09fffe03efdc
// Setup AppKey
AT+APPKEY=2b84e0b09b68e5cb42176fe753dcee79
// Set automatic send frequency in seconds
AT+SENDFREQ=60
// Set data rate
AT+DR=3
// Set LoRaWAN region (here US915)
AT+BAND=8
// Reset node to save the new parameters
ATZ
// After reboot, start join request
AT+JOIN=1,0,8,10
```

## _REMARK_
The AT command format used here is _**NOT**_ compatible with the RAK5205/RAK7205 AT commands.

## 2) Hardcoded LoRaWAN settings
`void api_read_credentials(void);`    
`void api_set_credentials(void);`    
If LoRaWAN credentials need to be hardcoded (e.g. the region, the send repeat time, ...) this can be done in `setup_app()`.
First the saved credentials must be read from flash with `api_read_credentials();`, then credentials can be changed. After changing the credentials must be saved with `api_set_credentials()`.
As the WisBlock API checks if any changes need to be saved, the changed values will be only saved on the first boot after flashing the application.     
Example:    
```c++
// Read credentials from Flash
api_read_credentials();
// Make changes to the credentials
g_lorawan_settings.send_repeat_time = 240000;                   // Default is 2 minutes
g_lorawan_settings.subband_channels = 2;                        // Default is subband 1
g_lorawan_settings.app_port = 4;                                // Default is 2
g_lorawan_settings.confirmed_msg_enabled = LMH_CONFIRMED_MSG;   // Default is UNCONFIRMED
g_lorawan_settings.lora_region = LORAMAC_REGION_EU868;          // Default is AS923-3
// Save hard coded LoRaWAN settings
api_set_credentials();
```

_**REMARK 1**_    
Hard coded credentials must be set in `void setup_app(void)`!

_**REMARK 2**_    
Keep in mind that parameters that are changed from with this method can be changed over AT command or BLE _**BUT WILL BE RESET AFTER A REBOOT**_!

----

# Packet data format
Three different data packet formats are available:
1) Standard Cayenne LPP format as it is used by MyDevice. This format has a 4 digit precision for the GNSS location.     
2) Extended Cayenne LPP format. This format has a 6 digit precision for the GNSS location and gives a better precision of the location. A special data decoder is required for this data format.
Above two data formats include as well the battery level and (if available) the data from the BME680 environment sensor.    

The different LPP channels are assigned like this:   

| Data               | Channel # | Channel ID | Length   | Comment                                             |
| ------------------ | --------- | ---------- | -------- | --------------------------------------------------- |
| GNSS data          | 1         | 136        | 9 bytes  | 4 digit precision                                   |
| GNSS data          | 1         | 137        | 11 bytes | 6 digit precision                                   |
| Battery value      | 2         | 2          | 2 bytes  | in Volt                                             |
| Humidity           | 3         | 104        | 1 bytes  | in %RH                                              |
| Temperature        | 4         | 103        | 2 bytes  | in °C                                               |
| Barmetric Pressure | 5         | 115        | 2 bytes  | in hPa (mBar)                                       |
| Gas resistance     | 6         | 2          | 2 bytes  | in kOhm, can be used to calculate air quality index |
| Accelerometer      | 64        | 113        | 6 bytes  | 0.001 G Signed MSB per axis                         |


3) Only location data formatted for the [Helium Mapper application](https://news.rakwireless.com/make-a-helium-mapper-with-the-wisblock/)    
This data packet contains only raw data without any data markers.    
**`4 byte latitude, 4 byte longitude, 2 byte altitude, 2 byte precision, 2 byte battery voltage`**

## _REMARK_
This application uses the RAK1904 acceleration sensor only for detection of movement to trigger the sending of a location packet, so the data packet does not include the accelerometer part by default. Accleration sensor data can be added with the ATC+ACC command.

----

# Custom AT commands 
## Change data format
To switch between the three data modes, a custom AT command is implemented.    
**`ATC+GNSS`**

Description: Switch between data packet formats

This command allows the user to see the current data format or switch to another data format.
Allowed values     
0 = 4 digit     
1 = 6 digit     
2 = Helium Mapper

| Command                      | Input Parameter | Return Value                                                                                  | Return Code              |
| ---------------------------- | --------------- | --------------------------------------------------------------------------------------------- | ------------------------ |
| ATC+GNSS?                    | -               | `ATC+GNSS: Get/Set the GNSS precision and format 0 = 4 digit, 1 = 6 digit, 2 = Helium Mapper` | `OK`                     |
| ATC+GNSS=?                   | -               | *0, 1 or 2*                                                                                   | `OK`                     |
| ATC+GNSS=`<Input Parameter>` | *0, 1 or 2*     | -                                                                                             | `OK` or `AT_PARAM_ERROR` |

**Examples**:

```
ATC+GNSS?

ATC+GNSS: Get/Set the GNSS precision and format 0 = 4 digit, 1 = 6 digit, 2 = Helium Mapper    
OK

ATC+GNSS=?

ATC+GNSS:GPS precision: 2
OK

ATC+GNSS=0

OK

ATC+GNSS=3

+CME ERROR:5
```

## Set location precision
To change the reported GNSS acquisition precision, a custom AT command is implemented.    

**`ATC+PREC`**

Description: Switch between location precision 

This command allows the user to set the reported location precision.     
Allowed values:     
0 = only fix type 3D      
1 = fix type 3D and >= 6 satellites     

| Command                        | Input Parameter | Return Value                                                                                                 | Return Code              |
| ------------------------------ | --------------- | ------------------------------------------------------------------------------------------------------------ | ------------------------ |
| ATC+PREC?                      | -               | `ATC+PREC: Get/Set the GNSS acquisition precision 0 = only fix type 3D, 1 = fix type 3D and >= 6 satellites` | `OK`                     |
| ATC+PREC=?                     | -               | *0 or 1*                                                                                                     | `OK`                     |
| ATC+PREC=`<Input Parameter>`   | *0 or 1*        |                                                                                                              | `OK` or `AT_PARAM_ERROR` |

**Examples**:

```
AT+PREC?

ATC+PREC: Get/Set the GNSS acquisition precision 0 = only fix type 3D, 1 = fix type 3D and >= 6 satellites    
OK

ATC+PREC=?

ATC+PREC:Acquistion requirements high
OK

ATC+PREC=0

OK

ATC+PREC=3

+CME ERROR:5
```

## Enable/Disable acceleration values in the payload

To enable or disable acceleration values in the payload, a custom AT command is implemented.    
**`ATC+ACC`**

Description: Enable or disable acceleration sensor values in the payload
Allowed values:     
0 = no accleration sensor values in the payload      
1 = accleration sensor values in the payload      

| Command                     | Input Parameter | Return Value                                                      | Return Code              |
| --------------------------- | --------------- | ----------------------------------------------------------------- | ------------------------ |
| ATC+ACC?                    | -               | `ATC+ACC: Get/Set whether ACC values are included in the payload` | `OK`                     |
| ATC+ACC=?                   | -               | *"ACC values in payload" or "ACC values not in payload"*          | `OK`                     |
| ATC+ACC=`<Input Parameter>` | *0 or 1*        | -                                                                 | `OK` or `AT_PARAM_ERROR` |

**Examples**:

```
ATC+ACC?

ATC+ACC: Get/Set whether ACC values are included in the payload    
OK

ATC+ACC=?

ATC+ACC:ACC values in payload
OK

ATC+ACC=0

OK

ATC+ACC=3

+CME ERROR:5
```

## List all found I2C modules

To list all connected modules, a custom AT command is implemented.    
**`ATC+MOD`**

Description: List all connected modules, **_read only command_**

This command allows the user to see the current data format or switch to another data format.

| Command   | Input Parameter | Return Value                      | Return Code |
| --------- | --------------- | --------------------------------- | ----------- |
| ATC+MOD?  | -               | `ATC+MOD: List all found modules` | `OK`        |
| ATC+MOD=? | -               | *List of found modules*           | `OK`        |

**Examples**:

```
AT+MOD?

AT+MOD: List all found modules    
OK

AT+MOD=?
+EVT:GNSS OK
+EVT:ACC FAIL
+EVT:OLED OK
+EVT:ENV FAIL
```
----

# Compiled output
The compiled files are located in the [./Generated](./Generated) folder. Each successful compiled version is named as      
**`WisBlock_GNSS_Vx.y.z_YYYYMMddhhmmss`**    
x.y.z is the version number. The version number is setup in the [./platformio.ini](./platformio.ini) file.    
YYYYMMddhhmmss is the timestamp of the compilation.

The generated **`.zip`** file can be used as well to update the device over BLE using either [WisBlock Toolbox](https://play.google.com/store/apps/details?id=tk.giesecke.wisblock_toolbox) or [Nordic nRF Toolbox](https://play.google.com/store/apps/details?id=no.nordicsemi.android.nrftoolbox) or [nRF Connect](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)

----

# Debug options 
Debug output can be controlled by defines in the **`platformio.ini`**    
_**LIB_DEBUG**_ controls debug output of the SX126x-Arduino LoRaWAN library
 - 0 -> No debug outpuy
 - 1 -> Library debug output (not recommended, can have influence on timing)    

_**MY_DEBUG**_ controls debug output of the application itself
 - 0 -> No debug outpuy
 - 1 -> Application debug output

_**CFG_DEBUG**_ controls the debug output of the nRF52 BSP. It is recommended to keep it off

## Example for no debug output and maximum power savings:

```ini
[env:wiscore_rak4631]
platform = nordicnrf52
board = wiscore_rak4631
framework = arduino
build_flags = 
    ; -DCFG_DEBUG=2
	-DSW_VERSION_1=1 ; major version increase on API change / not backwards compatible
	-DSW_VERSION_2=0 ; minor version increase on API change / backward compatible
	-DSW_VERSION_3=0 ; patch version increase on bugfix, no affect on API
	-DLIB_DEBUG=0    ; 0 Disable LoRaWAN debug output
	-DMY_DEBUG=0     ; 0 Disable application debug output
	-DNO_BLE_LED=1   ; 1 Disable blue LED as BLE notificator
lib_deps = 
	beegee-tokyo/SX126x-Arduino
	sparkfun/SparkFun u-blox GNSS Arduino Library@2.0.13
	adafruit/Adafruit BME680 Library
	beegee-tokyo/WisBlock-API
extra_scripts = pre:rename.py
```