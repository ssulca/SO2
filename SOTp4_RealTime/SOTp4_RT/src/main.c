#include <string.h>
#include <math.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Priorities at which the tasks are created. */
#define RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	PERIODIC_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define	APERIODIC_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
/* The bit of port 0 that the LPCXpresso LPC13xx LED is connected. */
#define mainLED_BIT 						( 22 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define PERIODIC_SEND_FREQUENCY_MS			( 50 / portTICK_RATE_MS )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define TEMP_QUEUE_LENGTH					( 10 )
#define USER_QUEUE_LENGTH					( 10 )

#define BUF_TEMP 6
#define portNEW_DELAY 200
/* The tasks as described in the accompanying PDF application note. */
static void prvReceiveTask( void *pvParameters );
static void prvPeriodicSendTask( void *pvParameters );
static void prvAperiodicSendTask( void *pvParameters );

/* Simple function to toggle the LED on the LPCXpresso LPC17xx board. */
static void prvToggleLED( void );
char * prvRandomString(int iSize);
void UART3_Init(void);
void UART_Send(char* datos , int size);


/* The queue used by both tasks. */
static xQueueHandle xTempQueue = NULL;
static xQueueHandle xUserQueue = NULL;

QueueHandle_t xActivatedQueue;
QueueHandle_t xQueueSet;

int main(void)
{
	/* Initialise P0_22 for the LED. */
	LPC_PINCON->PINSEL1	&= ( ~( 3 << 12 ) );
	LPC_GPIO0->FIODIR |= ( 1 << mainLED_BIT );

	/* Enable traceanalycer snapshot */
	vTraceEnable(TRC_START);
	/* Create the queue. */
	xTempQueue = xQueueCreate( TEMP_QUEUE_LENGTH, sizeof(char));
	xUserQueue = xQueueCreate( USER_QUEUE_LENGTH, sizeof(char*));

	/* Init Queue Set */
	xQueueSet = xQueueCreateSet(TEMP_QUEUE_LENGTH+USER_QUEUE_LENGTH);
	if(xQueueSet == NULL)
		return 0;

	if(xQueueAddToSet(xTempQueue,xQueueSet) == pdFAIL)
		return 0;
	if(xQueueAddToSet(xUserQueue,xQueueSet)== pdFAIL)
		return 0;

	if( xTempQueue != NULL && xUserQueue != NULL )
	{
		/* Start the two tasks as described in the accompanying application
		note. */
		xTaskCreate( prvReceiveTask,"RX", configMINIMAL_STACK_SIZE,
				NULL, RECEIVE_TASK_PRIORITY, NULL );
		xTaskCreate( prvPeriodicSendTask,"TEMP", configMINIMAL_STACK_SIZE,
				NULL, PERIODIC_TASK_PRIORITY, NULL );
		xTaskCreate( prvAperiodicSendTask,"USER", configMINIMAL_STACK_SIZE,
				NULL, APERIODIC_TASK_PRIORITY, NULL );
		/* Start the tasks running. */
		vTaskStartScheduler();
	}

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
}

/**
 * Tarea de envio periodico representando el sensor de Temperatura
 * Se comunica con el proceso central a travez de la coa xTempQueue
 * @param pvParameters *char, 
 */
static void prvPeriodicSendTask( void *pvParameters )
{
	portTickType xNextWakeTime;
	char cValueToSend = 0;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();
	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again.
		The block state is specified in ticks, the constant used converts ticks
		to ms.  While in the blocked state this task will not consume any CPU
		time. */
		vTaskDelayUntil( &xNextWakeTime, PERIODIC_SEND_FREQUENCY_MS );

		/* Send to the queue - causing the queue receive task to flash its LED.
		0 is used as the block time so the sending operation will not block -
		it shouldn't need to block as the queue should always be empty at this
		point in the code. */
		cValueToSend = (char)(rand() % 255); /* random value temp kelvin*/
		xQueueSend( xTempQueue, &cValueToSend, (TickType_t) 0 );
	}
}

/**
 * Tarea de Recepcion, recibe los datos de usuario y del sensor de Temperatura
 * y los envia por UART
 */
static void prvReceiveTask( void *pvParameters )
{
	char cReceivedValue;
	char * cpTempbuf[BUF_TEMP];
	char * buffer = "init\r\n";

	UART3_Init();
	UART_Send(buffer, strlen(buffer));

	for( ;; )
	{
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */

		xActivatedQueue = xQueueSelectFromSet(xQueueSet, portMAX_DELAY);

		if(xActivatedQueue == xTempQueue){
			xQueueReceive( xTempQueue, &cReceivedValue, portNEW_DELAY );
			itoa(cReceivedValue, cpTempbuf, 10);
			strcat(cpTempbuf, "\r\n");
			UART_Send(cpTempbuf, strlen(cpTempbuf));
		}
		else if(xActivatedQueue == xUserQueue){
			xQueueReceive( xUserQueue, &buffer, portNEW_DELAY );
			UART_Send(buffer, strlen(buffer));
			vPortFree(buffer);
		}
		/* toggle LED 22 */
		prvToggleLED();
	}
}

/**
 * Funcion Aperiodica representa el ingreso de caracteres de un usuario.
 */
static void prvAperiodicSendTask( void *pvParameters ){

		portTickType xNextWakeTime;
		char *cpBuffer = "init\r\n";

		/* Initialise xNextWakeTime - this only needs to be done once. */
		xNextWakeTime = xTaskGetTickCount();
		for( ;; )
		{
			/* Place this task in the blocked state until it is time to run again.
			The block state is specified in ticks, the constant used converts ticks
			to ms.  While in the blocked state this task will not consume any CPU
			time. */
			vTaskDelayUntil( &xNextWakeTime, rand()%portNEW_DELAY/portTICK_PERIOD_MS);

			/* Send to the queue - causing the queue receive task to flash its LED.
			0 is used as the block time so the sending operation will not block -
			it shouldn't need to block as the queue should always be empty at this
			point in the code. */
			cpBuffer = prvRandomString((rand()+xTaskGetTickCount())%57);
			xQueueSend( xUserQueue, &cpBuffer, (TickType_t) 0 );
		}
}

char * prvRandomString(int iSize){
	static char* leters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvqxyz1234567890";
	/* +3 para el agregado de \r\n */
	char* cpString = pvPortMalloc(iSize += 3);

	for(int i = 0;i < iSize-3; i++){
		cpString[i]=leters[rand()%61];
	}
	cpString[iSize-3]='\r';
	cpString[iSize-2]='\n';
	cpString[iSize-1]='\0';

	return cpString;
}

/**
 * toggle LED 22
 */
static void prvToggleLED( void )
{
	unsigned long ulLEDState;

	/* Obtain the current P0 state. */
	ulLEDState = LPC_GPIO0->FIOPIN;

	/* Turn the LED off if it was on, and on if it was off. */
	LPC_GPIO0->FIOCLR = ulLEDState & ( 1 << mainLED_BIT );
	LPC_GPIO0->FIOSET = ( ( ~ulLEDState ) & ( 1 << mainLED_BIT ) );
}

/**
 * configuracion UART
 */
void UART3_Init(void){
	LPC_SC->PCONP |= (1<<25); //*PCONP PCUART0 UART3 power/clock control bit. and desabited UART1
	LPC_SC->PCONP &= ~(3<<3);// *PCONP desabilito uart 0,1
	LPC_SC->PCLKSEL1 |= (1<<18); //*PCLKSEL1

	LPC_UART3->LCR = 0x03; // *U3LCR palabra 8 bits
	LPC_UART3->LCR |= (1<<2); // *U3LCR bit de parada
	LPC_UART3->LCR |= 0b10000000; //*U3LCR habilito el latch para configurar
	LPC_UART3->DLL = 54; //*U3LCR 0b10100001 ; // 115200
	LPC_UART3->DLM =0; //*U3LCR
	LPC_UART3->LCR &=~(1<<7);//*U3LCR desabilito el latch

	//pin 0 TXD0 pin 1 RXD0 puerto 0
	LPC_PINCON->PINSEL0 =0b1010; // *PINSEL0 configurar los pines port 0
	LPC_PINCON->PINMODE0 = 0; // *PINMODE0 pin a pull up

	/*
	 * LPC_UART3->IER = 1; // *U3IER habilito la interrupcion por Recive Data Aviable
	 * *ISER0 |= 1<<8; //activate interrup uart3
	*/
}

void UART_Send(char* datos , int size){
	for(int i =0 ; i < size ; i++){
		while((LPC_UART3->LSR & (1<<5))==0){}//*U3LSR // Wait for Previous transmission
		LPC_UART3->THR = datos[i]; //*U3THR
	}
}

/*
 *  FUNCIONES NECESARIAS PARA FreeRTOS
 */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	/* This function will get called if a task overflows its stack. */
	( void ) pxTask;
	( void ) pcTaskName;
	for( ;; );
}

void vConfigureTimerForRunTimeStats( void )
{
	const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	collecting run time statistical information - basically the percentage
	of CPU time that each task is utilising.  It is called automatically when
	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	to 1). */

	/* Power up and feed the timer. */
	LPC_SC->PCONP |= 0x02UL;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<2))) | (0x01 << 2);

	/* Reset Timer 0 */
	LPC_TIM0->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	LPC_TIM0->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	but not too fast so as to overflow all the time. */
	LPC_TIM0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

	/* Start the counter. */
	LPC_TIM0->TCR = TCR_COUNT_ENABLE;
}
