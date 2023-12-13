/*
  Monkee Mods Diana OS
  Created by sasaug
*/

#include "MMNetwork.h"
#include "MMConfig.h"
#include "MMStats.h"
#include "MMBlaster.h"
#include "DigitalPin.h"
#ifdef ESP32
#include <ESP32Servo.h>
#else     
#include <Servo.h>
#endif
#define NAME "Blaster_Diana"
#define OS_VERSION "0.1.0"
#define BLASTER_TYPE "flywheel"

#ifdef ESP32
  #define MOTOR1ESC_PIN D9
  #define MOTOR2ESC_PIN D10
  #define SOLENOIDESC_PIN D8
  #define TRIGGERSWITCH_PIN D4 
  #define PROFILESWITCH_PIN D5
  #define FSSWITCH1_PIN D1 //pos 1
  #define FSSWITCH2_PIN D2 //pos 3
  #define FSSWITCH3_PIN D3 //pos 2
  #define FRONTSIGHT_PIN D0
#else
  #define MOTOR1ESC_PIN 18
  #define MOTOR2ESC_PIN 19
  #define SOLENOIDESC_PIN 20
  #define TRIGGERSWITCH_PIN 6
  #define PROFILESWITCH_PIN 7
  #define FSSWITCH1_PIN 8 //pos 1
  #define FSSWITCH2_PIN 9 //pos 3
  #define FSSWITCH3_PIN 10 //pos 2
  #define FRONTSIGHT_PIN 5
#endif

//CONSTANT VARIABLES
const String NETMSG_SERVER_TRIGGER_PRESS = "server;trigger_press";
const String NETMSG_SERVER_REV_MOTOR_CRUISE = "server;rev_motor_cruise";
const String NETMSG_SERVER_REV_MOTOR_FULL = "server;rev_motor_full";

//DYNAMIC VARIABLES
DigitalPin triggerSwitchPin;
DigitalPin profileSwitchPin;
DigitalPin fs1SwitchPin;
DigitalPin fs2SwitchPin;
DigitalPin fs3SwitchPin;
DigitalPin motor1ESCPin;
DigitalPin motor2ESCPin;
DigitalPin solenoidESCPin;

int SPEEDMODE_FULL = 100; //mapped to 180 
int SPEEDMODE_CRUISE = 22; //mapped to 39.6
int SPEEDMODE_NEUTRAL = 33; //mapped to 59.4 
int SPEEDMODE_STOP = 0; //mapped to 0 angle

Servo solenoidESC;
Servo motor1ESC;
Servo motor2ESC;

MMBlaster blaster = MMBlaster(FLYWHEEL);
MMNetwork network = MMNetwork(NAME);
MMConfig config = MMConfig(NAME, OS_VERSION, BLASTER_TYPE, network);
MMStats stats = MMStats();

bool emulateTriggerPress = false;

//Game Mode stuff
const String NETMSG_SERVER_GAMEMODE_ENABLE = "server;gamemode_enable";
const String NETMSG_SERVER_GAMEMODE_DISABLE = "server;gamemode_disable";
const String NETMSG_SERVER_GAMEMODE_MOTOR_CRUISE_SPEED = "server;gamemode_motor_cruise_speed;";
const String NETMSG_SERVER_GAMEMODE_MOTOR_FULL_SPEED = "server;gamemode_motor_full_speed;";
const String NETMSG_SERVER_GAMEMODE_TRIGGER_ENABLE = "server;gamemode_trigger_enable";
const String NETMSG_SERVER_GAMEMODE_TRIGGER_DISABLE = "server;gamemode_trigger_disable";
const String NETMSG_SERVER_GAMEMODE_FIRING_MODE = "server;gamemode_firing_mode;";
const String NETMSG_SERVER_GAMEMODE_MOTOR_MODE = "server;gamemode_motor_mode;";
const String NETMSG_SERVER_GAMEMODE_BURSTFIRE_AMOUNT = "server;gamemode_burstfire_amount;";

bool isGameMode = false;
bool gameModeTriggerEnabled = false;
String gameModeFiringMode = "semi";
String gameModeMotorMode = "rage";
int gameModeBurstfireAmount = 3;

#ifdef ESP32
  TaskHandle_t taskLoop1;
#endif

// Initialise and setting up the board on main thread
void setup() {
  //initialised the pin mode
  motor1ESCPin.setup(MOTOR1ESC_PIN, OUTPUT);
  motor2ESCPin.setup(MOTOR2ESC_PIN, OUTPUT);
  solenoidESCPin.setup(SOLENOIDESC_PIN, OUTPUT);

  triggerSwitchPin.setup(TRIGGERSWITCH_PIN, INPUT_PULLUP);
  profileSwitchPin.setup(PROFILESWITCH_PIN, INPUT_PULLUP);
  fs1SwitchPin.setup(FSSWITCH1_PIN, INPUT_PULLUP);
  fs2SwitchPin.setup(FSSWITCH2_PIN, INPUT_PULLUP);
  fs3SwitchPin.setup(FSSWITCH3_PIN, INPUT_PULLUP);

  motor1ESC.attach(MOTOR1ESC_PIN);
  motor2ESC.attach(MOTOR2ESC_PIN);
  solenoidESC.attach(SOLENOIDESC_PIN);
  armMotor();

  //initialise the serial
  Serial.begin(9600); 
}

// Initialise networking stuff on second thread
void setup1() {
  //setup stats
  stats.setup();

  //initialise config first
  config.init();

  //reset config when trigger is down on boot
  if(triggerSwitchPin.read()){
    config.clearConfig();
  }

  if(!config.haveConfig()){
    //base config
    config.setConfigString(CONFIG_KEY_BLASTERNAME, "Diana", false, "Your Blaster Name");
    config.setConfigString(CONFIG_KEY_APWIFIPASSWORD, "monkeemods", false, "AP WiFi Password, secure your blaster's WiFi config page");
    config.setConfigString(CONFIG_KEY_LOGINUSERNAME, "", false, "Username for MMNetwork");
    config.setConfigString(CONFIG_KEY_LOGINPASSWORD, "", false, "Password for MMNetwork");
    config.setConfigString(CONFIG_KEY_WIFISSID, "", true, "WiFi SSID (multiple value supported, split with comma)");
    config.setConfigString(CONFIG_KEY_WIFIPASSWORD, "", true, "WiFi Password (multiple value supported, split with comma)");

    //blaster specific config
    config.setConfigString("profile1MotorMode", "rage", false, "rage,cool", "Profile 1 Flywheel Mode");
    config.setConfigString("profile1FiringMode1", "semi", false, "safe,semi,burst1,burst2,full", "Profile 1 Firing Mode 1");
    config.setConfigString("profile1FiringMode2", "burst1", false, "safe,semi,burst1,burst2,full", "Profile 1 Firing Mode 2");
    config.setConfigString("profile1FiringMode3", "full", false, "safe,semi,burst1,burst2,full", "Profile 1 Firing Mode 3");
    config.setConfigString("profile2MotorMode", "cool", false, "rage,cool", "Profile 2 Flywheel Mode");
    config.setConfigString("profile2FiringMode1", "safe", false, "safe,semi,burst1,burst2,full", "Profile 2 Firing Mode 1");
    config.setConfigString("profile2FiringMode2", "safe", false, "safe,semi,burst1,burst2,full", "Profile 2 Firing Mode 2");
    config.setConfigString("profile2FiringMode3", "safe", false, "safe,semi,burst1,burst2,full", "Profile 2 Firing Mode 3");
  
    config.setConfigInt("coolModeTriggerDelay", 300, "Motor spin up delay before firing when in cool mode");
    config.setConfigInt("rageModeTriggerDelay", 0, "Motor spin up delay before firing when in rage mode");
    config.setConfigInt("motorShutoffDelay", 200, "Motor shutdown delay after done firing");
    config.setConfigInt("firingRate", 5, "Darts per seconds, setting this too high will make pusher not working properly");
    config.setConfigInt("burstFireAmount1", 3, "Amount per burst fire shot");
    config.setConfigInt("burstFireAmount2", 5, "Amount per burst fire shot");
    config.setConfigInt("fullAutoMaxAmount", 30, "You can limit shots per full auto trigger pull");
    
    config.setConfigInt("motorSpeedModeCruise", 21, "Cruise/Hot/Rage mode motor speed");
    config.setConfigInt("motorSpeedModeFull", 100, "Motor speed at full (when shots firing)");
    config.setConfigString("semiAutoBehavior", "reactive", false, "passive,reactive", "Semi auto trigger behavior");
    config.setConfigString("burstFireBehavior", "reactive", false, "passive,reactive", "Burst fire trigger behavior");
    config.setConfigBool("frontSightEnable", true, "Enable front sight LED");
    config.saveConfig();
  }

  //setup config since settings is finalised now
  config.setup();

  //register callback
  network.setServerConnectedCallback(onServerConnected);
  network.setServerMessageCallback(onServerMessage);
  network.setup();

  //setup variables
  pinMode(FRONTSIGHT_PIN, OUTPUT);
  SPEEDMODE_CRUISE = config.getConfigInt("motorSpeedModeCruise");
  SPEEDMODE_FULL = config.getConfigInt("motorSpeedModeFull");
  digitalWrite(FRONTSIGHT_PIN, config.getConfigBool("frontSightEnable")? HIGH: LOW);

  //setup blaster
  refreshBlasterMode();
  blaster.setMotorShutoffDelay(config.getConfigInt("motorShutoffDelay"));
  blaster.setFiringRate(config.getConfigInt("firingRate"));
  blaster.setBurst1Amount(config.getConfigInt("burstFireAmount1"));
  blaster.setBurst2Amount(config.getConfigInt("burstFireAmount2"));
  blaster.setFullAutoMaxAmount(config.getConfigInt("fullAutoMaxAmount"));
  blaster.setSemiAutoTriggerBehavior(config.getConfigString("semiAutoBehavior") == "passive" ? PASSIVE: REACTIVE);
  blaster.setBurstfireTriggerBehavior(config.getConfigString("burstFireBehavior") == "passive" ? PASSIVE: REACTIVE);
  blaster.setPusherCallback(onPusherUpdate);
  blaster.setMotorCallback(onMotorUpdate);

  #ifdef ESP32
    xTaskCreatePinnedToCore(
      esp32Loop1,          
      "esp32Loop1",  
      10000,           
      NULL,           
      0,              
      &taskLoop1,        
      1);
  #endif
}

#ifdef ESP32
  void esp32Loop1(void * parameter){
    while(true){
      loop1();
    }
  } 
#endif

// Board's main loop
void loop() {
  //override the loop method if it's game mode
  if(isGameMode){
    loopGameMode();
    return;
  }

  //update firing mode if profile or fireselector switch change
  if(profileSwitchPin.isInputChanged() || fs1SwitchPin.isInputChanged() || fs2SwitchPin.isInputChanged() || fs3SwitchPin.isInputChanged()){
    refreshBlasterMode();
  }

  if(triggerSwitchPin.isInputChanged() || emulateTriggerPress){
    if(triggerSwitchPin.read() || emulateTriggerPress){
      blaster.triggerPress();
    }else{
      blaster.triggerRelease();
    }
  }

  blaster.loop();

  emulateTriggerPress = false;
  delay(1);                   
}

void refreshBlasterMode(){
  int profile = profileSwitchPin.read() ? 2: 1; 
  String key = "profile%1FiringMode%2";
  key.replace("%1", String(profile));
  if(fs1SwitchPin.read()){
    key.replace("%2", "1");
  }else if(fs3SwitchPin.read()){
    key.replace("%2", "2");
  }else{
    key.replace("%2", "3");
  }
  String firingMode = config.getConfigString(key);
  if(firingMode == "safe"){
    blaster.setFiringMode(SAFE);
  }else if(firingMode == "semi"){
    blaster.setFiringMode(SEMI);
  }else if(firingMode == "burst1"){
    blaster.setFiringMode(BURST1);
  }else if(firingMode == "burst2"){
    blaster.setFiringMode(BURST2);
  }else if(firingMode == "full"){
    blaster.setFiringMode(FULLAUTO);
  }

  String key2 = "profile%1MotorMode";
  key2.replace("%1", String(profile));
  String motorMode = config.getConfigString(key2);
  if(motorMode == "cool"){
    blaster.setMotorMode(COOL);
  }else{
    blaster.setMotorMode(RAGE);
  }    
}

//Game mode loop
void loopGameMode(){
  if(triggerSwitchPin.isInputChanged()){
    if(triggerSwitchPin.read()){
      blaster.triggerPress();
    }else{
      blaster.triggerRelease();
    }
  }

  blaster.loop();
  delay(10);    
}

// Board's second loop for networking purposes
void loop1(){
  network.loop();
  config.loop();
  stats.loop();
  delay(1);
}

void setMotorSpeed(int speedMode) {
  motor1ESC.write(speedToAngle(speedMode));
  motor2ESC.write(speedToAngle(speedMode));
}

void setSolenoidSpeed(int speedMode) {
  if(speedMode == SPEEDMODE_FULL){
    solenoidESC.writeMicroseconds(2000);
  }else{
    solenoidESC.writeMicroseconds(1000);
  }
}

void onServerConnected(){
}

void onServerMessage(String msg){
  config.onServerMessage(msg);
  if(msg == NETMSG_SERVER_GAMEMODE_ENABLE){
    isGameMode = true;
  }else if(msg == NETMSG_SERVER_GAMEMODE_DISABLE){
    isGameMode = false;
    delay(3000);
    #ifdef ESP32
      ESP.restart();
    #else
      rp2040.reboot();
    #endif
  }else if(msg == NETMSG_SERVER_TRIGGER_PRESS){
    emulateTriggerPress = true;
  }else if(msg == NETMSG_SERVER_REV_MOTOR_CRUISE){
    setMotorSpeed(SPEEDMODE_CRUISE);
    delay(3000);
    setMotorSpeed(SPEEDMODE_STOP);
  }else if(msg == NETMSG_SERVER_REV_MOTOR_FULL){
    setMotorSpeed(SPEEDMODE_FULL);
    delay(3000);
    setMotorSpeed(SPEEDMODE_STOP);
  }else if(msg.indexOf(NETMSG_SERVER_GAMEMODE_MOTOR_CRUISE_SPEED) > -1 && isGameMode){
    msg.replace(NETMSG_SERVER_GAMEMODE_MOTOR_CRUISE_SPEED, "");
    SPEEDMODE_CRUISE = msg.toInt();
  }else if(msg.indexOf(NETMSG_SERVER_GAMEMODE_MOTOR_FULL_SPEED) > -1 && isGameMode){
    msg.replace(NETMSG_SERVER_GAMEMODE_MOTOR_FULL_SPEED, "");
    SPEEDMODE_FULL = msg.toInt();
  }else if(msg == NETMSG_SERVER_GAMEMODE_TRIGGER_ENABLE && isGameMode){
    gameModeTriggerEnabled = true;
  }else if(msg == NETMSG_SERVER_GAMEMODE_TRIGGER_DISABLE && isGameMode){
    gameModeTriggerEnabled = false;
  }else if(msg.indexOf(NETMSG_SERVER_GAMEMODE_FIRING_MODE) > -1 && isGameMode){
    msg.replace(NETMSG_SERVER_GAMEMODE_FIRING_MODE, "");
    gameModeFiringMode = msg;
  }else if(msg.indexOf(NETMSG_SERVER_GAMEMODE_MOTOR_MODE) > -1 && isGameMode){
    msg.replace(NETMSG_SERVER_GAMEMODE_MOTOR_MODE, "");
    gameModeMotorMode = msg;
  }else if(msg.indexOf(NETMSG_SERVER_GAMEMODE_BURSTFIRE_AMOUNT) > -1 && isGameMode){
    msg.replace(NETMSG_SERVER_GAMEMODE_BURSTFIRE_AMOUNT, "");
    gameModeBurstfireAmount = msg.toInt();
  }
}

void onPusherUpdate(MotorSpeed speed, bool isFired){
  if(speed == STOP){
    setSolenoidSpeed(SPEEDMODE_STOP);
  }else{
    setSolenoidSpeed(SPEEDMODE_FULL);
  }
}

void onMotorUpdate(MotorSpeed speed){
  if(speed == STOP){
    setMotorSpeed(SPEEDMODE_STOP);
  }else if(speed == CRUISE){
    setMotorSpeed(SPEEDMODE_CRUISE);
  }else{
    setMotorSpeed(SPEEDMODE_FULL);
  }
}

void armMotor(){
  //arm speed controller, modify as necessary for your ESC
  motor1ESC.write(speedToAngle(SPEEDMODE_STOP));
  motor2ESC.write(speedToAngle(SPEEDMODE_STOP));
  setSolenoidSpeed(SPEEDMODE_STOP);
  delay(1000);
  motor1ESC.write(speedToAngle(SPEEDMODE_NEUTRAL));
  motor2ESC.write(speedToAngle(SPEEDMODE_NEUTRAL));
  setSolenoidSpeed(SPEEDMODE_FULL);
  delay(1000);
  motor1ESC.write(speedToAngle(SPEEDMODE_STOP));
  motor2ESC.write(speedToAngle(SPEEDMODE_STOP));
  setSolenoidSpeed(SPEEDMODE_STOP);
  Serial.println("Armed motor");
  delay(1000);
}

int speedToAngle(int speed){
  // speed is from 0 to 100 where 0 is off and 100 is max speed
  // the following maps speed values of 0-100 to angles from 0-180
  return map(speed, 0, 100, 0, 180);
}

//Stats related stuff
void statsAddShotsFired(){
  stats.setStatsInt("shotsFired", stats.getStatsInt("shotsFired")+1);
}

void statsAddSemiFired(){
  stats.setStatsInt("shotsFiredSemi", stats.getStatsInt("shotsFiredSemi")+1);
}

void statsAddBurstFired(){
  stats.setStatsInt("shotsFiredBurst", stats.getStatsInt("shotsFiredBurst")+1);
}

void statsAddFullAutoFired(){
  stats.setStatsInt("shotsFiredFullAuto", stats.getStatsInt("shotsFiredFullAuto")+1);
}



