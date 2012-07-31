#include<stdlib.h>
#include<avr/interrupt.h>

#ifndef MAX_PROCESSES
#define MAX_PROCESSES 2
#warning "MAX_PROCESSES not defined for avark.h"
#warning "MAX_PROCESSES defined as 2"
#endif


#if ( MAX_PROCESSES<=0 || MAX_PROCESSES>8 )
	#error "MAX_PROCESSES must be an integer from 1 to 8"
#endif


#define K_DISABLE() TIMSK0&=~(1<<TOIE0)
#define K_ENABLE() TIMSK0|=(1<<TOIE0)
#define K_START_CLOCK() TCCR0B|=( (1<<CS02) | (1<<CS00) )     //prescalar 1024
#define K_STOP_CLOCK() TCCR0B&=~( (1<<CS02) | (1<<CS00) )     //prescalar 1024
#define K_TIMER_PRESCALAR 1024


//############ Context Switcher ################
volatile unsigned char addressL, addressH;

volatile register unsigned char R30 asm ("r30");
volatile register unsigned char R31 asm ("r31");

volatile unsigned char current_process=MAX_PROCESSES-1;            //can take values from 0 to 7

volatile unsigned int task_stackpointer[MAX_PROCESSES];

volatile unsigned char task_firstrun=0xFF;

typedef volatile unsigned char K_STACK;

typedef void (*pointer_to_task)();
pointer_to_task task_address[MAX_PROCESSES];

volatile unsigned char K_SYSTEM_ERROR_CODE=0;

volatile unsigned char K_CONTEXT_SWITCHER_STACK[50];
//#####################################################


//###################  time ###########################
//volatile unsigned char K_TIME_adjust=0xFF;
volatile uintmax_t K_TIME=255;
//#####################################################


//################## waiting ##########################
volatile unsigned char K_IDLE_TASK_STACK[40];

volatile unsigned char K_TASK_WAITING=0;
volatile unsigned char K_ALL_WAITING_CHECKING_BYTE=0;

volatile uintmax_t K_TASK_WAKEUP_TIME[8];
volatile unsigned char K_TASK_ALARM_SET=0;


uint8_t K_TASK_MUTEX_WAITING_ATTEMPTS[MAX_PROCESSES];
uint8_t K_TASK_MUTEX_TAKEN_BY[MAX_PROCESSES];

//#####################################################



//##################### CLI ###########################
volatile char K_TASK_NAME[MAX_PROCESSES][10];

typedef void (*terminal_input_service_routine)(char*);
terminal_input_service_routine K_TISR[MAX_PROCESSES];
//#####################################################






/*################### SYSTEM ERROR CODES #############
0 : no error

#########  1 - 10 semaphore errors  ###########
1: semaphore memory not allocated
2: semaphore overflow

######### 11 - 20 mailbox errors ##############
11: no memory to create mailbox
12: mailbox buffer overflow
13: mailbox empty
14: no space in RAM to fetch mail
15: you are not the owner of the mailox. Access denied.

#######################################################
 */





//######## uart if needed here #########
void sendChar(volatile char a)
{
	while( !(UCSR0A&(1<<UDRE0)) );
	UDR0=a;
}	


void sendString(volatile char *a)
{
	int index=0;

	while(a[(int)index]!='\0')
	{
		sendChar(a[(int)index]);
		index++;
	}
	index=0;
}

void sendULInt(unsigned long int x)
{
	char integer[15];
	char place=14;
	if(x==0)
	{
		sendChar('0');
	}
	while(x!=0)
	{
		integer[place]=(x%10)+48;
		x/=10;
		place--;
	}
	place++;
	while(place<=14)
	{
		sendChar(integer[place]);
		place++;
	}
}

//########################################

void K_idle()
{
	while(1)
	{
	
	}
}


void K_schedule()
{

	/*if(task1_wakeup<K_TIME)
	{
		K_TASK_WAITING&=~(1<<1);
		//K_TASK_WAITING=0;
		sendChar('S');
	}*/
	char i;
	for(i=0;i<=7;i++)
	{
		if( (K_TASK_ALARM_SET&(1<<i)) && (K_TASK_WAKEUP_TIME[i]<K_TIME) )
		{
			K_TASK_WAITING&=~(1<<i);
			K_TASK_ALARM_SET&=~(1<<i);
		}
	}
	
	if(K_TASK_WAITING==K_ALL_WAITING_CHECKING_BYTE)
	{
		//idle task
		current_process=255;
	}
	
	else
	{
		do
		{
			if(current_process<(MAX_PROCESSES-1))
			{
				current_process=(current_process+1);
			}
			else
			{
				current_process=0;
			}
		}
		while( K_TASK_WAITING&(1<<current_process) );
	}
}



void K_start()
{	int i, j; 
	for(i=0;i<=MAX_PROCESSES-1;i++)
	{
		K_TISR[i]=NULL;
		for(j=0;j<=9;j++)
		{
			K_TASK_NAME[i][j]='\0';
		}
	}
	
	for(i=0;i<=MAX_PROCESSES-1;i++)
	{
		K_ALL_WAITING_CHECKING_BYTE|=(1<<i);
	}

	TCNT0=0;     
	//TIMSK0|=(1<<TOIE0);  
	K_ENABLE();
		
	//TCCR0B|=(1<<CS02);  
	K_START_CLOCK();

	sei();
}


void K_taskQuit()
{	
	//cli();
	K_DISABLE();
	K_TIME-=(255-TCNT0);
	K_ENABLE();
	//sei();
	TCNT0=0xFF;


	//int K_taskQuit_nop_count=(((32/1)*(F_CPU)/14745600)+1);
	//int K_taskQuit_nop_counter;
	/*for(K_taskQuit_nop_counter=0;K_taskQuit_nop_counter<=K_taskQuit_nop_count;K_taskQuit_nop_counter++)
	{
		asm volatile("nop");
	}
	*/
	int i;
	for(i=0;i<=K_TIMER_PRESCALAR;i++)
	{
		asm volatile("nop");
	}
	/*
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	   asm volatile("nop");
	 */
}









#if(MAX_PROCESSES==1)
void K_init(void (*k_task0ptr)(), int k_tos0)
{
	task_address[0]=k_task0ptr;
	task_stackpointer[0]=k_tos0;
}

#endif	


#if(MAX_PROCESSES==2)
void K_init(void (*k_task0ptr)(), int k_tos0, void (*k_task1ptr)(), int k_tos1)
{
	task_address[0]=k_task0ptr;
	task_stackpointer[0]=k_tos0;
	task_address[1]=k_task1ptr;
	task_stackpointer[1]=k_tos1;
}
#endif

#if(MAX_PROCESSES==3)
void K_init(void (*k_task0ptr)(), int k_tos0, void (*k_task1ptr)(), int k_tos1, void (*k_task2ptr)(), int k_tos2)
{
	task_address[0]=k_task0ptr;
	task_stackpointer[0]=k_tos0;
	task_address[1]=k_task1ptr;
	task_stackpointer[1]=k_tos1;
	task_address[2]=k_task2ptr;
	task_stackpointer[2]=k_tos2;
}
#endif

#if(MAX_PROCESSES==4)
void K_init(void (*k_task0ptr)(), int k_tos0, void (*k_task1ptr)(), int k_tos1, void (*k_task2ptr)(), int k_tos2, void (*k_task3ptr)(), int k_tos3)
{
	task_address[0]=k_task0ptr;
	task_stackpointer[0]=k_tos0;
	task_address[1]=k_task1ptr;
	task_stackpointer[1]=k_tos1;
	task_address[2]=k_task2ptr;
	task_stackpointer[2]=k_tos2;
	task_address[3]=k_task3ptr;
	task_stackpointer[3]=k_tos3;
}
#endif


#if(MAX_PROCESSES==5)
void K_init(void (*k_task0ptr)(), int k_tos0, void (*k_task1ptr)(), int k_tos1, void (*k_task2ptr)(), int k_tos2, void (*k_task3ptr)(), int k_tos3, , void (*k_task4ptr)(), int k_tos4)
{
	task_address[0]=k_task0ptr;
	task_stackpointer[0]=k_tos0;
	task_address[1]=k_task1ptr;
	task_stackpointer[1]=k_tos1;
	task_address[2]=k_task2ptr;
	task_stackpointer[2]=k_tos2;
	task_address[3]=k_task3ptr;
	task_stackpointer[3]=k_tos3;
	task_address[4]=k_task4ptr;
	task_stackpointer[4]=k_tos4;
}
#endif


#if(MAX_PROCESSES==6)
void K_init(void (*k_task0ptr)(), int k_tos0, void (*k_task1ptr)(), int k_tos1, void (*k_task2ptr)(), int k_tos2, void (*k_task3ptr)(), int k_tos3, void (*k_task4ptr)(), int k_tos4, void (*k_task5ptr)(), int k_tos5)
{
	task_address[0]=k_task0ptr;
	task_stackpointer[0]=k_tos0;
	task_address[1]=k_task1ptr;
	task_stackpointer[1]=k_tos1;
	task_address[2]=k_task2ptr;
	task_stackpointer[2]=k_tos2;
	task_address[3]=k_task3ptr;
	task_stackpointer[3]=k_tos3;
	task_address[4]=k_task4ptr;
	task_stackpointer[4]=k_tos4;
	task_address[5]=k_task5ptr;
	task_stackpointer[5]=k_tos5;
}
#endif


#if(MAX_PROCESSES==7)
void K_init(void (*k_task0ptr)(), int k_tos0, void (*k_task1ptr)(), int k_tos1, void (*k_task2ptr)(), int k_tos2, void (*k_task3ptr)(), int k_tos3, void (*k_task4ptr)(), int k_tos4, void (*k_task5ptr)(), int k_tos5, void (*k_task6ptr)(), int k_tos6)
{
	task_address[0]=k_task0ptr;
	task_stackpointer[0]=k_tos0;
	task_address[1]=k_task1ptr;
	task_stackpointer[1]=k_tos1;
	task_address[2]=k_task2ptr;
	task_stackpointer[2]=k_tos2;
	task_address[3]=k_task3ptr;
	task_stackpointer[3]=k_tos3;
	task_address[4]=k_task4ptr;
	task_stackpointer[4]=k_tos4;
	task_address[5]=k_task5ptr;
	task_stackpointer[5]=k_tos5;
	task_address[6]=k_task6ptr;
	task_stackpointer[6]=k_tos6;
}
#endif


#if(MAX_PROCESSES==8)
void K_init(void (*k_task0ptr)(), int k_tos0, void (*k_task1ptr)(), int k_tos1, void (*k_task2ptr)(), int k_tos2, void (*k_task3ptr)(), int k_tos3, void (*k_task4ptr)(), int k_tos4, void (*k_task5ptr)(), int k_tos5, void (*k_task6ptr)(), int k_tos6, void (*k_task7ptr)(), int k_tos7)
{
	task_address[0]=k_task0ptr;
	task_stackpointer[0]=k_tos0;
	task_address[1]=k_task1ptr;
	task_stackpointer[1]=k_tos1;
	task_address[2]=k_task2ptr;
	task_stackpointer[2]=k_tos2;
	task_address[3]=k_task3ptr;
	task_stackpointer[3]=k_tos3;
	task_address[4]=k_task4ptr;
	task_stackpointer[4]=k_tos4;
	task_address[5]=k_task5ptr;
	task_stackpointer[5]=k_tos5;
	task_address[6]=k_task6ptr;
	task_stackpointer[6]=k_tos6;
	task_address[7]=k_task7ptr;
	task_stackpointer[7]=k_tos7;
}
#endif


ISR(TIMER0_OVF_vect, ISR_NAKED)
//	ISR(USART0_RX_vect, ISR_NAKED)
{


	cli();
//		char c=UDR0;



	asm volatile ("push r0");
	asm volatile ("push r1");
	asm volatile ("push r2");
	asm volatile ("push r3");
	asm volatile ("push r4");
	asm volatile ("push r5");
	asm volatile ("push r6");
	asm volatile ("push r7");
	asm volatile ("push r8");
	asm volatile ("push r9");
	asm volatile ("push r10");
	asm volatile ("push r11");
	asm volatile ("push r12");
	asm volatile ("push r13");
	asm volatile ("push r14");
	asm volatile ("push r15");
	asm volatile ("push r16");
	asm volatile ("push r17");
	asm volatile ("push r18");
	asm volatile ("push r19");
	asm volatile ("push r20");
	asm volatile ("push r21");
	asm volatile ("push r22");
	asm volatile ("push r23");
	asm volatile ("push r24");
	asm volatile ("push r25");
	asm volatile ("push r26");
	asm volatile ("push r27");
	asm volatile ("push r28");
	asm volatile ("push r29");
	asm volatile ("push r30");
	asm volatile ("push r31");

	asm volatile("in %0,%1" : "=&r"(R31) : "I"(_SFR_IO_ADDR(SREG)));
	asm volatile ("push r31");

	R31=K_SYSTEM_ERROR_CODE;
	asm volatile ("push r31");



	task_stackpointer[current_process]=((SPH<<8) | SPL);

	int K_CONTEXT_SWITCHER_STCAKPOINTER=&K_CONTEXT_SWITCHER_STACK[49];
	SPL=K_CONTEXT_SWITCHER_STCAKPOINTER;
	SPH=(K_CONTEXT_SWITCHER_STCAKPOINTER>>8);

	K_TIME+=255;
	//sendULInt(K_TIME);
	//sendString("\n\r");


	K_schedule();
	
	



	//#######################################################
	if(current_process==255)
	{
		unsigned int idle_stack_address=&K_IDLE_TASK_STACK[39];
		SPL=idle_stack_address;
		SPH=(idle_stack_address>>8);
		
		unsigned int idle_address=K_idle;
		addressL=idle_address;
		addressH=(idle_address>>8);
		R30=addressL;
		R31=addressH;
		asm volatile("push r30");
		asm volatile("push r31");
		asm volatile("reti");
		
	}
	if(task_firstrun&(1<<current_process))
	{
		//void (*task1_ptr)();
		//task1_ptr=task1;
		//unsigned int task1_addr=task1_ptr;
		unsigned int task_addr=task_address[current_process];

		addressL=task_addr;
		addressH=(task_addr>>8);



		SPL=task_stackpointer[current_process];
		SPH=(task_stackpointer[current_process]>>8);

		R31=addressH;
		R30=addressL;
		asm volatile("push r30");
		asm volatile("push r31");



		task_firstrun&=~(1<<current_process);

		asm volatile("reti");
	}


	//#####################################################











	//SPL=task0_stackpointer;
	//SPH=(task0_stackpointer>>8);
	SPL=task_stackpointer[current_process];
	SPH=(task_stackpointer[current_process]>>8);


	K_TIME+=TCNT0;
	/*sendULInt(K_TASK_WAKEUP_TIME[1]);
	sendString("\t");
	sendULInt(K_TIME);
	sendString("\t");
	sendULInt(K_TASK_WAITING);
	sendString("\n\r");
	TCNT0=0;
	*/

	asm volatile ("pop r31");
	K_SYSTEM_ERROR_CODE=R31;

	asm volatile ("pop r31");
	asm volatile("out %0,%1" : :"I"(_SFR_IO_ADDR(SREG)) , "r"(R31));

	asm volatile ("pop r31");
	asm volatile ("pop r30");
	asm volatile ("pop r29");
	asm volatile ("pop r28");
	asm volatile ("pop r27");
	asm volatile ("pop r26");
	asm volatile ("pop r25");
	asm volatile ("pop r24");
	asm volatile ("pop r23");
	asm volatile ("pop r22");
	asm volatile ("pop r21");
	asm volatile ("pop r20");
	asm volatile ("pop r19");
	asm volatile ("pop r18");
	asm volatile ("pop r17");
	asm volatile ("pop r16");
	asm volatile ("pop r15");
	asm volatile ("pop r14");
	asm volatile ("pop r13");
	asm volatile ("pop r12");
	asm volatile ("pop r11");
	asm volatile ("pop r10");
	asm volatile ("pop r9");
	asm volatile ("pop r8");
	asm volatile ("pop r7");
	asm volatile ("pop r6");
	asm volatile ("pop r5");
	asm volatile ("pop r4");
	asm volatile ("pop r3");
	asm volatile ("pop r2");
	asm volatile ("pop r1");
	asm volatile ("pop r0");

	
	
	//sei();

	asm volatile("reti");

}


EMPTY_INTERRUPT(BADISR_vect);

/*
   ISR(BADISR_vect)
   {
   }
 */





