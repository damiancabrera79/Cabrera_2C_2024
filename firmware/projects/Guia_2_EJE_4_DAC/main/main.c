/*!
 * @mainpage Guía 2 - Ejercicio 4 - Segunda parte
 * @section genDesc Descripción General
 *
 * Programa para adquisición de señales mediante ADC y transmisión por UART.
 * Se utiliza FreeRTOS para gestionar las tareas y un puenteo de pines del ESPC6 para la salida UART.
 * Los datos se envían a un osciloscopio en serie para su visualización.
 *
 * @section hardConn Conexión de Hardware
 *
 * - Canal ADC conectado al CH1.
 * - UART configurada a 115200 baud.
 *
 * @section changelog Historial de Cambios
 *
 * |   Fecha      | Descripción                                    |
 * |:------------:|:-----------------------------------------------|
 * | 27/09/2024   | Creación del documento                         |
 * | 04/10/2024   | Agregaron comentarios y documentación          |
 *
 * @author
 * Damian Cabrera (cabreradamian@gmail.com)
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

#define CONFIG_PERIOD_uS_A 4329 // 0,004328 seg
#define CONFIG_PERIOD_uS_B 2000 // 2mseg - 500Hz

uint16_t valores;

uint16_t contador =0;

#define BUFFER_SIZE 231


/*==================[internal data definition]===============================*/
TaskHandle_t main_task_handle = NULL;

/*const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
*/


const char ecg[BUFFER_SIZE] = {
17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
17,17,17

} ;

/*==================[internal data definition]===============================*/

TaskHandle_t conversion_DAC_task_handle = NULL;

TaskHandle_t conversion_ADC_task_handle = NULL;


/*==================[internal functions declaration]=========================*/
static void conversionDAC(void *pParam){
	while(true){
		
		if(contador<BUFFER_SIZE){
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			AnalogOutputWrite(ecg[contador]);
			contador++;
		}
		else{
			contador = 0;
		}
	}
}

static void conversionADC(void *pParam){

	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &valores);
		// la funcion Itoa convierte de int --> ascii
		UartSendString(UART_PC,(char*) UartItoa(valores, 10));
		UartSendString(UART_PC, "\r");
	}

}

void funcionTimerA(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(conversion_DAC_task_handle, pdFALSE);

}

void funcionTimerB(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(conversion_ADC_task_handle, pdFALSE);

}

void funcionUART(void* param){
	//ver si tenemos que usarlo para leer algo desde el puerto serie.
}

/*==================[external functions definition]==========================*/


void app_main(void){
	// inicializo el struct para configurar qué puerto voy a utilizar y en qué modo
	analog_input_config_t Analog_config = {
		.input = CH1,
		.mode = ADC_SINGLE
	};

	AnalogInputInit(&Analog_config);
	AnalogOutputInit();

	// Configuro la estructura del timerA
	timer_config_t timerA = {
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_uS_A,
		.func_p = funcionTimerA,
		.param_p = NULL
	};
	// inicialización del timerA
	TimerInit(&timerA);

	// Configuro la estructura del timerB
	timer_config_t timerB = {
		.timer = TIMER_B,
		.period = CONFIG_PERIOD_uS_B,
		.func_p = funcionTimerB,
		.param_p = NULL
	};
	 /// inicialización del timer B
	TimerInit(&timerB);

	
	/// configuración de la UART
	serial_config_t Uart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = funcionUART,
		.param_p = NULL
	};

	/// inicializo la UART
	UartInit(&Uart);

	/// creación de las tareas que quiero ejecutar 
	xTaskCreate(&conversionDAC, "analogizar", 2048, NULL, 5, &conversion_DAC_task_handle);
	xTaskCreate(&conversionADC, "digitalizar", 2048, NULL, 5, &conversion_ADC_task_handle);
	
	// 
	TimerStart(timerA.timer);
	TimerStart(timerB.timer);
}

/*==================[end of file]============================================*/