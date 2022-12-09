#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include "main.h"

#define SCH_MAX_TASKS 	10
#define NO_TASK_ID 		0

#define ERROR_SCH_TOO_MANY_TASKS 10
#define ERROR_SCH_CANNOT_DELETE_TASK 2
#define RETURN_ERROR 0
#define RETURN_NORMAL 1

void SCH_Init (void);
void SCH_Update(void);
unsigned char SCH_Add_Task (void(*pFunction)() , unsigned int DELAY,unsigned int PERIOD);
void SCH_Dispatch_Tasks (void);
unsigned char SCH_Delete_Task ( const int );
void SCH_Go_To_Sleep();



#endif /* INC_SCHEDULER_H_ */
