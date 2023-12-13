# MM Diana OS
This is a reversed engineered HT Diana with the aim to maintain most of it's hardware, but replacing the micro-controller and chips that power it.
The objective is to make the blaster's chip easily replacable, making it open source and add more functions to the blaster.

## Concept
The rage/cool mode switch is now used as Profile switch, setting Profile 1 or Profile 2. The fire selector remains as fire selector for the respective profile. The default value program Profile 2 to be always safe hence it acts like an ON/OFF switch.
To turn off motor, just switch it to Safe and the flywheel will stop spinning(if you are in rage mode).

## Configuration
- blasterName
Set a name for your blaster. This is also used as the blaster's WiFi SSID. Default to `Blaster_Diana`

- apWiFiPassword
Password for your blaster's WiFi password. Default to `monkeemods`

- loginUsername & loginPassword
For future use when connect to MMNetwork

- wifiSSID
Connect to a WiFi router SSID (eg your home network). You can set a few SSID, seperate them with ,
Eg: network1,network2,network3

- wifiPassword
The password for WiFi SSID.
Eg: network1password,network2password,network3password

- profile1MotorMode
The flywheel mode when in Profile 1. `rage` will spin the flywheel at `motorSpeedModeCruise` speed. `cool` will not spin the motor but you will need apply a delay(configurable in `coolModeTriggerDelay`)

- profile1FiringMode1/profile1FiringMode2/profile1FiringMode3/profile2FiringMode1/profile2FiringMode2/profile2FiringMode3

You can use `safe`,`semi`,`burst1`,`burst2`,`full`

Safe = Safety

Semi = 1 shot

Burst1 = Shots configured in `burstFireAmount1`

Burst2 = Shots configured in `burstFireAmount2`

Auto = Full auto with maximum limit set in `fullAutoMaxAmount`

- coolModeTriggerDelay
Delay after pulling the trigger to firing when in `cool` mode to ensure flywheel spin up to speed. BLHeli ESC have a 100ms delay so make sure you account this. Value is milliseconds

- rageModeTriggerDelay
Delay after pulling the trigger to firing when in `rage` mode. You most likely don't need this unless you are looking to maximise your FPS. Value is milliseconds

- motorShutoffDelay
The delay to turn off the flywheel after finish firing. Value is milliseconds

- firingRate
Darts per seconds. Setting this too high will result in solenoid not working

- burstFireAmount1
The amount of burstfire shot for `burst1`

- burstFireAmount2
The amount of burstfire shot for `burst2`

- fullAutoMaxAmount
The maximum shot for full auto. You can limit this to control your full auto or do 1-X amount of shots, based on the value you set.

- motorSpeedModeCruise
The flywheel speed in rage mode. Value is 1-100, it means 1-100%. This value might varies ESC to ESC, in my case my ESC requires a min of 21 to turn on.

- motorSpeedModeFull
The maximum flywheel speed when firing. Value is 1-100 as well. You can limit this to lower your FPS output.

- semiAutoBehavior/burstFireBehavior

There are 2 types of trigger behavior, `passive` and `reactive`.

In `passive` mode, it will ignore any other trigger pull in mid firing cycle. Example for burstfire, while it fires off all 3 burst shots, if you pull your trigger again, your blaster will fire a total of 3 shots with 2 trigger pull in total.

In `reactive` mode, it WILL NOT ignore the trigge pull in mid firing cycle. While it still firing the first set of 3 shots, if you press the trigger again, a total of 6 shots will be fired.

This applies to semi auto as well but since the speed is too fast, you will barely notice this unless you do it on purpose. Using `passive` trigger might give you the sense of the trigger malfunction so configure this based on your preference.

- frontSightEnable
Enable front sight LED. It's just 8mA so off or on won't save you much battery.

## Hardware Needed
- Raspberry Pico W
- Brushless ESC x 2 (20-30A at least) (I'm using BLHeli-S 45A, BLHeli32 should work too)
- Brushed ESC (10A at least, or you can use MOSFET) (I'm using 20A)
- BEC (unless your ESC have BEC)

## Software Dependencies
- MMBlaster, MMNetwork, MMConfig, MMStats (included)

## Installation
1. Wire the blaster as per the wiring diagram provided
2. To flash your microcontroller:
- Install Arduino IDE
- Install ESP32 Arduino library (if you using ESP32)/ Arduino-Pico (if you using Pico W)
- Download the source code and open it in Arduino IDE
- Install ESP32Servo library in ArduinoIDE(if you are using ESP32)
- Install ArduinoJson library in ArduinoIDE
- Connect your microcontroller in download/flashing mode(usually holding the B button while connecting the cable)
- Click the Upload button in ArduinoIDE
3. If you are updating to a new version, check if there are change of parameters, if yes, you have to HOLD your trigger when you connect battery to reset the config

# Note
This is designed with Pico W but ESP32 is also supported but not tested yet.
Most ESC should work as long you can fit into the blaster. You can use MOSFET for the solenoid but a small change will be needed as MOSFET don't take PWM, a HIGH should be enough to turn it on. Note that you need a MOSFET that can turn on with 3.3V(Pico voltage)

