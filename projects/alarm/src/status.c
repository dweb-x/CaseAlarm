#include "status.h"
/*
 *      Initializes the system
 *      also called to reset
 */
status_t initStatus(){
        isArmed = false;
        isLocked = false;
        isMoving = false;
        alarmStatus = OFF;
        interval = 60;
        time = 0;
        cursor = 0;
       
        status_t system;
        system.isArmed = &isArmed;
        system.isLocked = &isLocked;
        system.isMoving = &isMoving;
        system.alarmStatus = &alarmStatus;
        system.interval = &interval;
        system.time = &time;
        system.code = code;
        system.cursor = &cursor;
        return system;
}