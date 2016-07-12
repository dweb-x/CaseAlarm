#ifndef __STATUS_H
#define __STATUS_H
#include <stdbool.h>
#include <stdint.h>
 
typedef enum {
        OFF,
        ARMED,
        PENDING,
        ACTIVE
} alarmState;
 
/*
 * these vars can only be accessed through the struct status_t 
 * Any shared data here is atomic. The system is designed to only transition
 * forward latching into any new state. the only way to transition backward
 * is when the system is fully reset. Because of this I felt no syncronization
 * was needed. You can't miss a change if the change is permenant.
 */
static bool isArmed; //shared
static bool isLocked;
static bool isMoving;//shared
static alarmState alarmStatus;//shared
static uint32_t interval;//shared
static uint32_t time;
static char code[5] = {"0000"};
static uint8_t cursor;
/*
 * Struct of pointers to the system state. 
 */
typedef struct systemStatus {
        bool *isArmed;
        bool *isLocked;
        bool *isMoving;
        alarmState *alarmStatus;
        uint32_t *interval;
        uint32_t *time;
        char *code;
        uint8_t *cursor;
} volatile status_t;
 
status_t initStatus(void);
 
#endif