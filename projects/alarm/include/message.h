#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <status.h>
#include <lcdBuffer.h>

//displayed strings
static char msgOff[] = {"DISABLED"};
static char msgArmed[] = {" ARMED  "};
static char msgPend[] = {"PENDING "};
static char msgOn[] = {"ACTIVE  "};
static char msgLocked[] = {"LOCKED  "};
static char msgUnLocked[] = {"UNLOCKED"};
static char msgMove[] = {"[MOVING]"};
static char msgNoMove[] = {"[      ]"};
// not being lazy. It's faster than calculating it on the fly.
static char *msgCursor[] = {"-    "," -   ","  - ","   -"};

void pushStatus(message_t*, status_t*);

#endif