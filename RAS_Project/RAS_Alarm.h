/* RAS_Alarm.h */
#ifndef _RAS_ALARM_H_
#define _RAS_ALARM_H_

#define UPP_GNAME	"RAS-DB"
#define LOW_GNAME	"USR-ALARM"
#define ITEM_NAME	"CNT"

int ALARM_Init();
int ALARM_CountUser();
void ALARM_SetStatus();
int ALARM_Report();

#endif /* _RAS_ALARM_H_ */
