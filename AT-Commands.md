# AT-Commands

To make it easy to setup the LoRaWAN® credentials and LoRa® P2P settings, an AT command interface over USB is implemented. It includes the basic commands required to define the node.     
The AT command interface is compatible with the [RUI3 AT commands](https://docs.rakwireless.com/product-categories/software-apis-and-libraries/rui3/at-command-manual/).

This document covers the custom AT commands that were added on top of the standard AT commands.

**Credits:**    
Taylor Lee (taylor.lee@rakwireless.com)

_**REMARK 1**_
After changing LoRaWAN® parameters or LoRa P2P settings, the device must be reset by either the ATZ command or pushing the reset button.

_**REMARK 2**_
The Serial port connection is lost after the ATZ command or pushing the reset button. The connection must be re-established on the connected computer before log output can be seen or AT commands can be entered again.

_**REMARK 3**_
The Serial port is setup for 115200 baud, 8N1. It cannot be changed by AT commands.

_**REMARK 4**_
In LoRaWAN® Class C mode the received data is not shown in the AT Command interface. The data has to be handled in the user application
In P2P LoRa® mode the received data is not shown in the AT Command interface. The data has to be handled in the user application

_**REMARK 5**_
LoRa® is a registered trademark or service mark of Semtech Corporation or its affiliates. LoRaWAN® is a licensed mark.

----
## Content

### Custom commands
* [ATC Command syntax](#at-command-syntax)
* [ATC+STATUS](#atcstatus) Get Device Status
* [ATC+GNSS](#atcgnss) Set GNSS output format
* [ATC+PREC](#atcprec) Set reported GNSS location precision
* [ATC+ACC](#atcacc) Enable/disable acceleration sensor values in the payload
* [ATC+MOD](#atcmod) List all connected modules

----

## AT Command syntax
In general, the custom AT Commands are the same as standard AT commands.     
The AT command is based on ASCII characters.     

Differences for custom AT commands:     
Custom AT Commands start with the prefix `ATC` and ends with `<CR><LF>` (i.e. \r\n). For the rest of the document, the `\r\n` part is omitted for the sake of clarity.

The ATC commands have the standard format “ATC+XXX”, with XXX denoting the command.

There are four available command formats:

| **AT COMMAND FORMAT**      | **Description**                                    |
| -------------------------- | -------------------------------------------------- |
| ATC+XXX?                    | Provides a short description of the given command |
| ATC+XXX=?                   | Reading the current value on the command          |
| ATC+XXX=`<input parameter>` | Writing configuration on the command              |


The output of the commands is returned via UART.

The format of the reply is divided into two parts: returned value and the status return code.

_**`<CR>` stands for “carriage return” and `<LF>` stands for “line feed”**_


1. **`<value><CR><LF>`** is the first reply when (**AT+XXX?**) command description or (**AT+XXX=?**) reading value is executed then it will be followed by the status return code. The formats with no return value like (**AT+XXX=`<input parameter>`) writing configuration command and (**AT+XXX**) run command will just reply the status return code.


2. **`<CR><LF><STATUS><CR><LF>`** is the second part of the reply which is the status return code.

The possible status are:

| **STATUS RETURN CODE**   | **Description**                                      |
| ------------------------ | ---------------------------------------------------- |
| `OK`                     | Command executed correctly without error.            |
| `+CME ERROR:1`           | Generic error or input is not supported.             |
| `+CME ERROR:2`           | Command not allowed. |
| `+CME ERROR:5`           | The input parameter of the command is wrong.         |
| `+CME ERROR:6`           | The parameter is too long.                           |
| `+CME ERROR:8`           | Value out of range.              |

More details on each command description and examples are given in the remainder of this section. 

----

## ATC+STATUS

Description: Show device status

This command allows the user to get the current device status.

| Command                    | Input Parameter | Return Value                              | Return Code |
| -------------------------- | --------------- | ----------------------------------------- | ----------- |
| ATC+STATUS?                | -               | `AT+STATUS: Show LoRaWAN status`          | `OK`        |
| ATC+STATUS=?               | -               | *< status   >*                            | `OK`        |

**Examples**:

```
ATC+STATUS?

ATC+STATUS: Show LoRaWAN status
OK

ATC+STATUS=?
LoRaWAN status:
   Auto join disabled
   OTAA enabled
   Dev EUI 5032333338350012
   App EUI 1200353833333250
   App Key 50323333383500121200353833333250
   NWS Key 50323333383500121200353833333250
   Apps Key 50323333383500121200353833333250
   Dev Addr 83986D12
   Repeat time 120000
   ADR disabled
   Public Network
   Dutycycle disabled
   Join trials 10
   TX Power 0
   DR 3
   Class 0
   Subband 1
   Fport 2
   Unconfirmed Message
   Region 10
   Network joined

+STATUS: 
OK
```

[Back](#content)    

----

## ATC+GNSS

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

### _REMARK_
This application uses the RAK1904 acceleration sensor only for detection of movement to trigger the sending of a location packet, so the data packet does not include the accelerometer part by default. Accleration sensor data can be added with the ATC+ACC command.

To switch between the three data modes, a custom AT command is implemented.    
**`AT+GNSS`**

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

AT+GNSS: Get/Set the GNSS precision and format 0 = 4 digit, 1 = 6 digit, 2 = Helium Mapper    
OK

ATC+GNSS=?

AT+GNSS:GPS precision: 2
OK

ATC+GNSS=0

OK

ATC+GNSS=3

+CME ERROR:5
```

## ATC+PREC
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
ATC+PREC?

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

## ATC+ACC
+ACC", "Get/Set whether ACC values are included in the payload
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

AT+ACC: Get/Set whether ACC values are included in the payload    
OK

ATC+ACC=?

ATC+ACC:ACC values in payload
OK

ATC+ACC=0

OK

ATC+ACC=3

+CME ERROR:5
```

## ATC+MOD

To list all connected modules, a custom AT command is implemented.    
**`ATC+MOD`**

Description: List all connected modules, **_read only command_**

This command allows the user to see the current data format or switch to another data format.

| Command                        | Input Parameter | Return Value                      | Return Code              |
| ------------------------------ | --------------- | --------------------------------- | ------------------------ |
| ATC+MOD?                       | -               | `ATC+MOD: List all found modules` | `OK`                     |
| ATC+MOD=?                      | -               | *List of found modules*           | `OK`                     |

**Examples**:

```
ATC+MOD?

ATC+MOD: List all found modules    
OK

ATC+MOD=?
+EVT:GNSS OK
+EVT:ACC FAIL
+EVT:OLED OK
+EVT:ENV FAIL
```
----
