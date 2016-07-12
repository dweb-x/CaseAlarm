/*
 * This is the starting point for the EN0572 assignment 2011.
 */

#include <stdbool.h>
#include <ucos_ii.h>
#include <osutils.h>
#include <bsp.h>
#include <leds.h>
#include <buttons.h>
#include <lcd.h>
#include <accelerometer.h>
#include <potentiometer.h>
#include <status.h>
#include <lcdbuffer.h>
#include <stdbool.h>
#include <message.h>
#include <string.h>

enum delayConstants { MIN_INTERVAL = 20,  
                      MAX_INTERVAL  = 120};

static const char CODE[] = {"1234"};    //The unlock code
/*
*********************************************************************************************************
*                                            APPLICATION TASK PRIORITIES
*********************************************************************************************************
*/

enum {  APP_TASK_LCD_PRIO = 4,
        APP_TASK_BUTTONS_PRIO,
        APP_TASK_TIMER_PRIO,
        APP_TASK_SOUNDER_PRIO,
        APP_TASK_INTERVAL_PRIO,
        APP_TASK_MOTION_PRIO
};

/*
*********************************************************************************************************
*                                            APPLICATION TASK STACKS
*********************************************************************************************************
*/

#define  APP_TASK_STK_SIZE      256     // Same size for each task

static OS_STK appTaskMotionStk[APP_TASK_STK_SIZE];
static OS_STK appTaskLcdStk[APP_TASK_STK_SIZE];
static OS_STK appTaskButtonsStk[APP_TASK_STK_SIZE];
static OS_STK appTaskTimerStk[APP_TASK_STK_SIZE];
static OS_STK appTaskSounderStk[APP_TASK_STK_SIZE];
static OS_STK appTaskIntervalStk[APP_TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                            APPLICATION TASK FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static void appTaskMotion(void *pdata);
static void appTaskLcd(void *pdata);
static void appTaskButtons(void *pdata);
static void appTaskTimer(void *pdata);
static void appTaskSounder(void *pdata);
static void appTaskInterval(void *pdata);


/*
*********************************************************************************************************
*                                            GLOBAL VARIABLE DEFINITIONS
*********************************************************************************************************
*/

status_t system; //see status.h for definition

int main() {

  /* Initialise the board support package */
  bspInit();
  // Initialise the system status
  system = initStatus();
  
  /* Initialise the OS */
  OSInit();                                                   

  /* Create the tasks */
  OSTaskCreate(appTaskMotion,                               
               (void *)0,
               (OS_STK *)&appTaskMotionStk[APP_TASK_STK_SIZE - 1],
               APP_TASK_MOTION_PRIO);
  
  OSTaskCreate(appTaskLcd,                               
               (void *)0,
               (OS_STK *)&appTaskLcdStk[APP_TASK_STK_SIZE - 1],
               APP_TASK_LCD_PRIO);
  
  OSTaskCreate(appTaskButtons,                               
               (void *)0,
               (OS_STK *)&appTaskButtonsStk[APP_TASK_STK_SIZE - 1],
               APP_TASK_BUTTONS_PRIO);
  
  OSTaskCreate(appTaskTimer,                               
               (void *)0,
               (OS_STK *)&appTaskTimerStk[APP_TASK_STK_SIZE - 1],
               APP_TASK_TIMER_PRIO);
  
  OSTaskCreate(appTaskSounder,                               
               (void *)0,
               (OS_STK *)&appTaskSounderStk[APP_TASK_STK_SIZE - 1],
               APP_TASK_SOUNDER_PRIO);
  
  OSTaskCreate(appTaskInterval,                               
               (void *)0,
               (OS_STK *)&appTaskIntervalStk[APP_TASK_STK_SIZE - 1],
               APP_TASK_INTERVAL_PRIO);

  
  /* Start the OS */
  OSStart();                                                  
  
  return 0;      
}

/*
*********************************************************************************************************
*                                            APPLICATION TASK DEFINITIONS
*********************************************************************************************************
*/

/*
 *      Handles motion detection via the accelerometer.
 *      When the system is armed the current accelerometer values are locked in.
 *      The task then polls each axis of the accelerometer testing if
 *      the new value is +- the old value +- the threshold. If limits are exceded
 *      the system status is updated to moving.
 */
static void appTaskMotion(void *pdata) {
  message_t msg;
  static const uint32_t THRESHOLD = 16; //A dead zone to stop false positives
  static const uint32_t CHANNEL[3] = {1, 0, 6}; //x, y, z
  static uint32_t startValue[3]; //values at instant system is armed
  static uint32_t value[3]; //current values
  static bool isSet = false; // true if initial values are locked in.
  static int i = 0;
 while (true) {
   //run once when system is armed
   if(!isSet && *system.isArmed){    
     // lock in the starting position
     for(; i < 3; i++){ // for each channel
      startValue[i] = accelerometerRead(CHANNEL[i]); 
      isSet = true;
     }
   }
   else if(isSet) {
     for(i = 0; i < 3; i++){ // for each channel
      value[i] = accelerometerRead(CHANNEL[i]); 
      if(value[i] > startValue[i] + THRESHOLD //if value +- threshold
         || value[i] < startValue[i] - THRESHOLD){
           //it moved
            *system.isMoving =  true;
         }
     }
     
   }
   
    pushStatus(&msg, &system); 
    OSTimeDly(200);
  }
}

/*
 *      Polls and responds to button events.
 *      Handles locking/unlocking, arming and code input.
 */
static void appTaskButtons(void *pdata) {
  message_t msg;
  int codeIndex = 0; //keeps track of the cursor position
  while (true) {
    //Toggle lock if system is disarmed
    if(!*system.isArmed && isButtonPressed(BUT_2)){
      if(!*system.isLocked){
          *system.isLocked = true; 
      }
      else
        *system.isLocked = false; 
    }    
    //arm system
    if(*system.isLocked && isButtonPressed(BUT_1)){
      *system.isArmed = true;
      *system.alarmStatus =  ARMED;
    }  
    //Move the cursor
    if(isButtonPressed(JS_RIGHT)){
      if(++codeIndex >= 3)
        codeIndex = 3;
      *system.cursor = codeIndex; 
    }
    else if (isButtonPressed(JS_LEFT)){
      if(--codeIndex <= 0)
        codeIndex = 0;
      *system.cursor = codeIndex;
    } 
    //Select digit
    if((isButtonPressed(JS_UP))){
      system.code[codeIndex]++;
      if(system.code[codeIndex] > '9')
        system.code[codeIndex] = '0';    
    }
    else if(isButtonPressed(JS_DOWN)) {
      system.code[codeIndex]--;
      if(system.code[codeIndex] < '0')
        system.code[codeIndex] = '9';  
    }   
    // Check if correct
    if(isButtonPressed(JS_CENTRE))
    if(strcmp(system.code,CODE) == 0){
      //reset system
      system = initStatus();
    }
    pushStatus(&msg, &system);
    OSTimeDly(200);
  }
}
/*
 *      Timer is enabled if the system is armed and the case is moving.
 *      Counts down from the interval value raising the alarm if 0 is reached. 
 */
static void appTaskTimer(void *pdata) {
  message_t msg;
  uint32_t time;
  static bool timerEnabled = false;
  while (true) {
    //if armed and moving start the timer
    if(*system.isArmed && *system.isMoving && !timerEnabled){
      *system.alarmStatus = PENDING;
      time = *system.interval;
      timerEnabled = true; 
    }
    else if(*system.isArmed && *system.isMoving && timerEnabled) {
      if(--time <= 0){
        // if timer reaches 0 raise the alarm
        *system.alarmStatus = ACTIVE;
      }      
        *system.time = time;
    }
    else { //reset
      timerEnabled = false;
      *system.time = 0;
    }   
   pushStatus(&msg, &system);
   OSTimeDly(1000);
  }
}

/*
 *      Pulls messages off the buffer and writes them to the LCD
 */
static void appTaskLcd(void *pdate) {
  /* Start the OS ticker -- must be done in the highest priority task */
  osStartTick();
  // initialize the buffer
  lcdBufferInit();
  message_t msg;
    lcdSetTextPos(1, 1);
    lcdWrite("Alarm \t :");
    lcdSetTextPos(1, 2);
    lcdWrite("Interval:");
    lcdSetTextPos(1, 3);
    lcdWrite("Time \t :");
    lcdSetTextPos(1, 4);
    lcdWrite("Case \t :"); 
    lcdSetTextPos(1, 5);
    lcdWrite("    \t :");
    lcdSetTextPos(1, 6);
    lcdWrite("Code \t :");
  while (true) {
    // if message is available get it and print    
    lcdBufferGet(&msg);   
    lcdSetTextPos(11, 1);
    lcdWrite("%s", msg.alarmStatus);
    lcdSetTextPos(11, 2);
    lcdWrite("%3i", msg.interval);
    lcdSetTextPos(11, 3);
    lcdWrite("%3i", msg.time);
    lcdSetTextPos(11, 4);
    lcdWrite("%s", msg.caseLockMsg);
    lcdSetTextPos(11, 5);
    lcdWrite("%s", msg.caseMoveMsg);
    lcdSetTextPos(11, 6);
    lcdWrite("%s", msg.code);
    lcdSetTextPos(11, 7);
    lcdWrite("%s", msg.cursor);
    OSTimeDly(10);
  }
}

/*
 *      Reads the value of the potentiometer and converts to
 *      range 20 - 120
 */
static void appTaskInterval(void *pdata) {
    message_t msg;
    uint32_t val;
    while (true) { //only adjust if disarmed
      if(!*system.isArmed && *system.alarmStatus == OFF){
        // val range 0 -> 127
        val = potentiometerRead()/8;
        if(val <= 20)
          val = 20;
        else if (val > 120)
          val = 120;
        *system.interval = val;
      pushStatus(&msg, &system);
      }
    OSTimeDly(10);
   } 
}

/*
 *      Simulates the alarm sounder
 *      Flashes the link LED if the alarm is active(sounding)
 */
static void appTaskSounder(void *pdata) {
  // Use the link LED to simulate the sounder
  ledIdentifier_t sounder = USB_LINK_LED;
  while (true) {
    if((*system.isArmed) && (*system.alarmStatus == ACTIVE)) {
        ledToggle(sounder);
    }
    else {
        ledSetState(sounder, LED_OFF);
    }
    OSTimeDly(300);
   } 
}


