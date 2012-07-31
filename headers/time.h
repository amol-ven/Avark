//############ software timer ################

#define K_getTime() K_TIME

#define K_taskSuspend() K_TASK_WAITING|=(1<<current_process)

void K_taskWait(uintmax_t waitingtime)
{
	K_TASK_ALARM_SET|=(1<<current_process);
	K_TASK_WAKEUP_TIME[current_process]=K_getTime()+waitingtime;
	K_taskSuspend();
}
//############################################
