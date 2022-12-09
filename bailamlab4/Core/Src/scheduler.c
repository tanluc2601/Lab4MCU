#include "scheduler.h"

struct sTask {
	void (* pTask) (void);
		uint32_t Delay ;
		uint32_t Period ;
		uint8_t RunMe;
		uint32_t TaskID ;
};

struct sTask SCH_tasks_G[SCH_MAX_TASKS];

unsigned short Error_tick_count_G;
unsigned char Last_error_code_G;
unsigned char Error_code_G;

uint8_t count_task=0; // Count the number of tasks in the queue
struct sTask tmp;
int mark=0;


void SCH_Init ( void ) {
	unsigned char i ;
	for ( i=0; i<SCH_MAX_TASKS; i++) {
			SCH_tasks_G[i].pTask = 0x0000;
			SCH_tasks_G[i].Delay = 0;
			SCH_tasks_G[i].Period = 0;
			SCH_tasks_G[i].RunMe = 0;
	}
	Error_code_G=0;

}

unsigned char SCH_Add_Task (void(*pFunction)() , unsigned int DELAY,unsigned int PERIOD){
	if(count_task>=SCH_MAX_TASKS) {
		Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
		return 10;
	}
		 count_task++;
	int Index = 0;
	// Find a place to add
	while ((SCH_tasks_G[Index].pTask != 0)&&(Index<SCH_MAX_TASKS)&&(DELAY>=SCH_tasks_G[Index].Delay)){
		DELAY=DELAY-SCH_tasks_G[Index].Delay;
		Index ++;
		}
	 mark = Index;
	 	 // If the position to add is not at the end
		if(Index!=count_task-1){
			// Move the array from the position to be added to the end of the array 1 cell after
			for ( Index = count_task-2; Index >= mark; Index--){
				SCH_tasks_G[Index+1] = SCH_tasks_G[Index];
			}
			// Subtract the Delay of the task immediately after the newly added task
			SCH_tasks_G[mark+1].Delay = SCH_tasks_G[mark+1].Delay-DELAY;
		}
	// add the task to the position to add
	 SCH_tasks_G[mark].pTask = pFunction ;
	 SCH_tasks_G[mark].Delay = DELAY;
	 SCH_tasks_G[mark].Period = PERIOD;
	 SCH_tasks_G[mark].RunMe = 0;

	 return mark ;
 }

unsigned char SCH_Delete_Task ( const int TASK_INDEX) {
		unsigned char Return_code ;
		// Check if the task to be deleted exists or not
		if ( SCH_tasks_G[TASK_INDEX ].pTask == 0||TASK_INDEX<0||TASK_INDEX>=SCH_MAX_TASKS ) {
			Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
			Return_code = RETURN_ERROR;
		} else {
			Return_code = RETURN_NORMAL;
		}
	unsigned char Index ;
	// delete and reorder queue
	for ( Index = TASK_INDEX; Index < count_task-1; Index++){
			SCH_tasks_G[Index] = SCH_tasks_G[Index+1];
	}
	// reset the value for queue Index position
	SCH_tasks_G[Index].pTask = 0;
	SCH_tasks_G[Index].Delay = 0;
	SCH_tasks_G[Index].Period = 0;
	SCH_tasks_G[Index].RunMe = 0;
	count_task--;
	return Return_code ;
}

void SCH_Update(void){
	// Check if there is a task that needs to be excecute
	if ( SCH_tasks_G[0].pTask){
		//If the first task of the queue has delay=0 then execute
		if (SCH_tasks_G[0].Delay==0){
			SCH_tasks_G[0].RunMe += 1;
			if ( SCH_tasks_G[0].Period ) {
				SCH_tasks_G[0].Delay = SCH_tasks_G[0].Period ; // reset  Delay
				tmp=SCH_tasks_G[0];		// use tmp to save the fields of the old task
				SCH_Delete_Task(0);		// delete the excecuted task from the queue
				// add task to queue and assign old Runme to execute in Dispath
				SCH_tasks_G[SCH_Add_Task(tmp.pTask,tmp.Period,tmp.Period)].RunMe=tmp.RunMe;
			}
		}
		else{
			SCH_tasks_G[0].Delay-=1;
		}
	}
}

void SCH_Dispatch_Tasks (void){
unsigned char Index ;
for ( Index = 0; Index < SCH_MAX_TASKS; Index++) {
	if ( SCH_tasks_G[Index].RunMe > 0 ) {
		(*SCH_tasks_G[Index].pTask)() ;
		SCH_tasks_G[Index].RunMe -= 1;
	}
}
	SCH_Report_Status () ;
	SCH_Go_To_Sleep () ;
}

void SCH_Report_Status(void) {
#ifdef SCH_REPORT_ERRORS
	// Check for new errors
	if(Error_code_G != Last_error_code_G) {
	// show error
		Error_port->ODR = 255 - Error_code_G;
		Last_error_code_G = Error_code_G ;
	// set 1 timer to count the error reporting time
		if(Error_code_G != 0) {
			Error_tick_count_G = 60000;
		} else {
			Error_tick_count_G = 0 ;
}
	}else{ // Error timeout
		if( Error_tick_count_G != 0) {
			if (--Error_tick_count_G == 0){
				Error_code_G = 0 ; // Reset error code
			}
		}
}
#endif
}

void SCH_Go_To_Sleep(){
	// Suspend Tick increment to prevent wakeup by Systick interrupt
	// Otherwise the Systick interrupt will wake up the device with in 1ms (HAL tim base)
	HAL_SuspendTick();
	// enter sleepmode
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	HAL_ResumeTick();
}
