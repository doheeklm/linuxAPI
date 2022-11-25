/* RAS_Alarm.h */
#ifndef _RAS_ALARM_H_
#define _RAS_ALARM_H_

#define UPP_GNAME		"RAS-DB"
#define LOW_GNAME		"USR-ALARM"
#define ITEM_NAME		"CNT"

#define ALARM_NORMAL	10
#define ALARM_MINOR		20
#define ALARM_MAJOR		30

int		ALARM_Init();
int		ALARM_CountUser();
void	ALARM_SetStatus();
int		ALARM_Report();

#endif /* _RAS_ALARM_H_ */
