//#################### semaphore #############

typedef volatile unsigned int K_SEM;
//typedef const unsigned int* K_SEMA
//#define K_SEMA K_SEM*
typedef K_SEM* K_SEMA;

K_SEM* K_semCreate(unsigned int K_sema_initial_value)
{	
	K_SEM* createdSem=(K_SEM*)malloc(sizeof(K_SEM));
	if(createdSem==NULL)
	{
		K_SYSTEM_ERROR_CODE=1;
		return NULL;
	}
	*createdSem=K_sema_initial_value;
	return createdSem;
	
}


K_SEM K_semPost(K_SEM* sema)
{
	if( (*sema)==65535 )
	{
		K_SYSTEM_ERROR_CODE=2;
		return NULL;
	}		
	(*sema)++;
	return (*sema);
}


K_SEM K_semPend(K_SEM* sema)
{
	for(;;)
	{
		while((*sema)==0)
		{
			K_taskQuit();
		
		}
		//(*sema)--;
		K_SEM temp=*sema;
		if(temp>0)
		{
			(*sema)--;
		}
		else
		{continue;}
		
		
		if(((*sema)+1)!=temp)
		{
			(*sema)++;
			continue;
		}
		else
		{break;}
	}
	return (*sema);
}
//############################################

