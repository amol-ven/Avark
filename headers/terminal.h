#include<avr/interrupt.h>
#include<string.h>

#define INPUT_BUFFERLENGTH 50
#define INPUT_RESOLVED_COMMAND_LENGTH 10

volatile char K_command[INPUT_BUFFERLENGTH];
volatile char K_command_working[INPUT_RESOLVED_COMMAND_LENGTH];
volatile int K_command_pointer=0;
volatile unsigned char command_process;


void K_setTaskName(char* name)
{
	strcpy(K_TASK_NAME[current_process], name);
	//sendULInt((int)current_process);
}

void K_setTISR(void (*terminal_input_service_routine)(char*))
{
	K_TISR[current_process]=terminal_input_service_routine;
}
	
ISR(USART0_RX_vect)
{
	K_command[K_command_pointer]=UDR0;
	sei();
	sendChar(K_command[K_command_pointer]);
	
	if(K_command[K_command_pointer]==0x08)
	{
		sendChar(' ');
		sendChar(0x08);
		K_command_pointer-=2;
	}
	
	
	if(K_command[K_command_pointer]==0x0D)        // enter key
	{
		K_command[K_command_pointer+1]='\0';
		//count leading spaces
		int index=0, space_count=0, word_count=0;
		while(index<K_command_pointer)
		{
			if(K_command[index]==' ')
			{
				space_count++;
			}
			else
			{
				break;
			}
			index++;
		}
		
		//leading spaces counted
		while(index<K_command_pointer)
		{
			if(K_command[index]!=' ')
			{
				word_count++;
			}
			else
			{
				break;
			}
			index++;
		}
		
		//initial word length counted
		
		if(word_count<=8)
		{
			strncpy(K_command_working, K_command+space_count, word_count);
			K_command_working[word_count]='\0';
			
			
			
			//NOW K_command_working has the command string
			
			
			char command_match=0;
			for(command_process=0;command_process<=MAX_PROCESSES-1;command_process++)
			{
				if( (strcmp(K_TASK_NAME[command_process], K_command_working))==0 )
				{
					command_match=1;
					break;
				}
			}
			if(command_match==1)
			{
				int K_command_offset=space_count+word_count;
				space_count=0, word_count=0;
				while(index<K_command_pointer)
				{
					if(K_command[index]==' ')
					{
						space_count++;
					}
					else
					{
						break;
					}
					index++;
				}
			
				//leading spaces counted
				while(index<K_command_pointer)
				{
					if(K_command[index]!=' ')
					{
						word_count++;
					}
					else
					{
						break;
					}
					index++;
				}
				//switch word length counted
				
				strncpy(K_command_working, K_command+K_command_offset+space_count, word_count);
				K_command_working[word_count]='\0';
				
				if(strcmp(K_command_working, "-s")==0)
				{
					K_TASK_WAITING|=(1<<command_process);
					
					sendString("\n\rAK:_>");
				}
				else if(strcmp(K_command_working, "-k")==0)
				{
					K_TASK_WAITING|=(1<<command_process);
					task_firstrun|=(1<<command_match);
					
					sendString("\n\rAK:_>");
				}
				else if(strcmp(K_command_working, "-r")==0)
				{
					K_TASK_WAITING&=~(1<<command_process);
				
					sendString("\n\rAK:_>");
				}
				
				else
				{
					if(K_TISR[command_process]!=NULL)
					{
						sendString("\n\r");
						K_DISABLE();
						(*K_TISR[command_process])(K_command+K_command_offset+space_count);
						K_ENABLE();
						sendString("\n\rAK:->");
					}
					else
					{
						sendString("\n\r'");
						sendString(K_command_working);
						sendString("' No such switch");
						sendString("\n\rAK:_>");
					}
				}
			}
			else 
			{
			
				
				sendString("\n\r'");
				sendString(K_command_working);
				sendString("' No such command");
				sendString("\n\rAK:_>");
				
			}
			
		}
		else
		{
				sendString("\n\r");
				sendString("No such command");
				sendString("\n\rAK:_>");
		}	
			
			
		
		
		K_command_pointer=INPUT_BUFFERLENGTH-2;
	}
	K_command_pointer++;
	if(K_command_pointer>(INPUT_BUFFERLENGTH-2))
	{
		K_command_pointer=0;
	}
	
	
}
