
#include "stm32f10x.h"
#include "cmsis_os.h"
#include "uart.h"

osMutexId x_mutex;
osMutexDef(x_mutex);
osSemaphoreId insert_item;                    	// Semaphore ID
osSemaphoreDef(insert_item);                 		// Semaphore definition
osSemaphoreId space;                         		// Semaphore ID
osSemaphoreDef(space);                       		// Semaphore definition
osSemaphoreId check;
osSemaphoreDef (check);

const unsigned int N = 4;
unsigned char buffer[N];

unsigned int insert_pointer = 0;
unsigned int remove_pointer = 0;

void producer1 (void const *argument);
void producer2 (void const *argument);
void consumer1 (void const *argument);
void consumer2 (void const *argument);
void cashier (void const *argument);


osThreadDef(producer1, osPriorityNormal, 1, 0);
osThreadDef(producer2, osPriorityNormal, 1, 0);
osThreadDef(consumer1, osPriorityNormal, 1, 0);
osThreadDef(consumer2, osPriorityNormal, 1, 0);
osThreadDef(cashier, osPriorityNormal, 1, 0);

osThreadId T_producer1;
osThreadId T_producer2;
osThreadId T_consumer1;
osThreadId T_consumer2;
osThreadId T_cashier;

osMessageQId Q_LED;
osMessageQDef (Q_LED,0x16,unsigned char);
osEvent  result;


unsigned char item	=	0x20;
unsigned char item2	= 0x30;
unsigned char data1 = 0x00;
unsigned char data2 = 0x00;
unsigned char rr[N];

void producer1 (void const *argument) 
{
		for(;;)
{
	osSemaphoreWait(space, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	buffer[insert_pointer] = item;
	item++;
	insert_pointer = (insert_pointer + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(insert_item);
	if (item == 0x5B)
	{		
		item = 0x20;
	}
}
}

void producer2 (void const *argument) 
{
	for(;;)
{
	osSemaphoreWait(space, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	buffer[insert_pointer] = item2;
	item2++;
	insert_pointer = (insert_pointer + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(insert_item);
	if (item2 == 0x3A)
	{		
		item2 = 0x30;
	}
}
}

void consumer1 (void const *argument) 
{
	for (;;)
	{
	osSemaphoreWait(insert_item, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	data1 = buffer[remove_pointer];
	rr[remove_pointer] = data1;
	remove_pointer = (remove_pointer + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(space);
	SendChar (data1);
	if ((remove_pointer % N) == 0)
	{
		SendChar('\n');
		osSemaphoreRelease(check);
	}
	}
}

void consumer2 (void const *argument) 
{
	for (;;)
	{
	osSemaphoreWait(insert_item, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	data2 = buffer[remove_pointer];
	rr[remove_pointer] = data2;
	remove_pointer = (remove_pointer + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(space);
	SendChar (data2);
	if ((remove_pointer % N) == 0)
	{
		SendChar('\n');
		osSemaphoreRelease(check);
	}
	}
}

void cashier (void const *argument) 
{
	//cashier
	for(;;){
		result = 	osMessageGet(Q_LED,osWaitForever);				//wait for a message to arrive
		SendChar(result.value.v);
	}
}


int main (void) 
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
	USART1_Init();
	insert_item = osSemaphoreCreate(osSemaphore(insert_item), 0);
	space = osSemaphoreCreate(osSemaphore(space), N);
	x_mutex = osMutexCreate(osMutex(x_mutex));	
	
	Q_LED = osMessageCreate(osMessageQ(Q_LED),NULL);					//create the message queue
	
	T_producer1 = osThreadCreate(osThread(producer1), NULL);//producer1
	T_producer2 = osThreadCreate(osThread(producer2), NULL);//producer2
	T_consumer1 = osThreadCreate(osThread(consumer1), NULL);//consumer1
	T_consumer2 = osThreadCreate(osThread(consumer2), NULL);//consumer2
	T_cashier = osThreadCreate(osThread(cashier), NULL);//cashier
	

 
	osKernelStart ();                         // start thread execution 
}
