#include <message.h>

/*
 *      Constructs an LCD message from a snapshot of the system status
 *      and pushes the message onto the LCD buffer. 
 *      Handles conversion of system data into strings
 *      for display.
 */
void pushStatus(message_t *msg, status_t *status){

  // Set the alarm status string
  switch(*status->alarmStatus) {
    case OFF:
      msg->alarmStatus = msgOff;
      break;
    case ARMED:
      msg->alarmStatus = msgArmed;
      break;
    case PENDING:
      msg->alarmStatus = msgPend;
      break;
    case ACTIVE:
      msg->alarmStatus = msgOn;
      break;
  }
  
  // output the interval
  msg->interval = *status->interval;
  
  // output time
  msg->time = *status->time;
  // output case status
  msg->caseLockMsg = *status->isLocked ? msgLocked : msgUnLocked;
  msg->caseMoveMsg = *status->isMoving ? msgMove : msgNoMove;
  // output code
  msg->code = status->code;
  // output cursor position
  switch(*status->cursor) {
    case 0:
      msg->cursor = msgCursor[0];
      break;
    case 1:
      msg->cursor = msgCursor[1];
      break;
    case 2:
      msg->cursor = msgCursor[2];
      break;
    case 3:
      msg->cursor = msgCursor[3];
      break;
  }
  //put message on lcd buffer
  lcdBufferPut(msg);
}
        

        