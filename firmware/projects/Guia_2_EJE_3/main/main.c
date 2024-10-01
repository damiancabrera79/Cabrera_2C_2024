/*! @mainpage Guía 2 - Ejercicio 3
 * @section genDesc Descripción General
 *
 * Este programa controla el encendido y apagado de LEDs, la lectura de un sensor de distancia HC-SR04,
 * y la visualización de la distancia en un display LCDITSE0803 utilizando un ESP32. 
 * 
 *  * El sistema utiliza:
* - Las interrupciones para el control de las teclas y para el control de tiempos Timers. 
 * 
 * El sistema permite:
 * - Encender los LEDs en secuencia según la distancia medida (si es menor a 10 cm, no se enciende ningún LED;
 * si está entre 10 y 20 cm, se enciende el LED 1; si está entre 20 y 30 cm, se encienden los LEDs 1 y 2;
 * si es mayor a 30 cm, se encienden los LEDs 1, 2 y 3).
 * - Mostrar la distancia medida en el display con la opción de congelar la lectura al presionar el switch 2
 * o la tecla H del teclado de la notebook.
 * - Activar o desactivar el sistema con el switch 1 o la tecla O del teclado de la notebook.
 * - Además envia los datos de medición por el puerto serie para visualizar por "Serial Monitor"
 *
 * @section hardConn Conexión de Hardware
 *
 * |    ESP32c6	  	|   HC-SR04   	|
 * |:---------------:|:--------------|
 * | 	GPIO_3	    |	  ECHO      |
 * |	GPIO_2		|   TRIGGER     |
 * |	+5V			|     +5V       |
 * |	GND 		|	  GND       |
 *
 * |	  ESP32c6		|	Lcditse0803 |
 * |:--------------:| :--------------|
 * |    GPIO_19		|     SEL_1     |
 * |   	GPIO_18		|	  SEL_2     |
 * |   	GPIO_9	    |	  SEL_3     |
 * |   	GPIO_20		|	   D1		|
 * |   	GPIO_21		|	   D2		|
 * | 	GPIO_22		|	   D3		|
 * | 	GPIO_23		|	   D4		|
 * | 	+5V			|	  +5V		|
 * | 	GND			|	  GND		|
 *
 * @section changelog Historial de Cambios
 *
 * |   Fecha	  | Descripción                                   |
 * |:------------:|:----------------------------------------------|
 * | 20/09/2024   | Creación del documento		                  |
 * | 30/09/2024   | Correción de errores printf                   |
 *
 * @author Damian Cabrera (cabreradamian@gmail.com)
 */

/*==================[inclusiones]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "ctype.h"

/*==================[macros y definiciones]=================================*/

// Defino el tiempo (en microsegundos) entre las interrupciones del timer
#define CONFIG_BLINK_PERIOD_uS 500000 // 0,5 segundos

/** @var activar
 *  @brief Variable booleana global que registra si el sistema está activado o desactivado.
 */
bool activar = true;

/** @var hold
 *  @brief Variable booleana global que registra si el display está congelado o en modo normal.
 */
bool hold = false;



/** @var distancia
 *  @brief Variable uint16_t que almacena el valor de la distancia medida por el sensor HC-SR04.
 */
uint16_t distancia;

/** @var centimetrosPulgadas
 *  @brief Variable global booleana para controlar el cambio de unidad de medida del 
 *         sensor de centímetros a pulgadas.
 */


/*==================[definición de datos internos]===========================*/

/**
 * @var sensar_task_handle
 * @brief Identificador de la tarea que se encarga de realizar la medición de distancia.
 */
TaskHandle_t sensar_task_handle = NULL;

/**
 * @var leds_task_handle
 * @brief Identificador de la tarea que se encarga de encender y apagar los LEDs según la distancia.
 */
TaskHandle_t leds_task_handle = NULL;

/**
 * @var display_task_handle
 * @brief Identificador de la tarea que se encarga de actualizar la información en el display.
 */
TaskHandle_t display_task_handle = NULL;


TaskHandle_t enviarDatosUART_task_handle = NULL;

/*==================[declaración de funciones internas]=========================*/

/**
 * @fn void FuncionTimer(void* param)
 * @brief Función invocada en la interrupción del timer A. Define lo que sucede cuando se utiliza el timer.
 * Envía notificaciones a las tareas que se deben ejecutar.
 * @param param Parámetro no utilizado.
 */
void FuncionTimer(void *param)
{
	vTaskNotifyGiveFromISR(leds_task_handle, pdFALSE);	  /* Envía una notificación a la tarea de los LEDs */
	vTaskNotifyGiveFromISR(sensar_task_handle, pdFALSE);  /* Envía una notificación a la tarea del sensor */
	vTaskNotifyGiveFromISR(display_task_handle, pdFALSE); /* Envía una notificación a la tarea del display */
	vTaskNotifyGiveFromISR(enviarDatosUART_task_handle, pdFALSE); /* Envía una notificación a la tarea de la UART */
}

/**
 * @fn static void SwitchActivar(void *pvParam)
 * @brief Función que invierte el estado de la variable booleana de activación.
 * @param pvParam Puntero a la variable booleana a modificar.
 */
static void SwitchActivar(void *pvParam)
{
	bool *punteroActivar = (bool *)(pvParam);
	*punteroActivar = !(*punteroActivar);
	//printf("Switch 1 presionado\n");
}

/**
 * @fn static void SwitchHold(void *pvParam)
 * @brief Función que invierte el estado de la variable booleana para congelar o liberar el display.
 * @param pvParam Puntero a la variable booleana a modificar.
 */
static void SwitchHold(void *pvParam)
{
	bool *punteroHold = (bool *)(pvParam);
	*punteroHold = !(*punteroHold);
	//printf("Switch 2 presionado\n");
}



void FuncionUART(void* param){
	uint8_t uart;
	UartReadByte(UART_PC, &uart);
	uart = toupper(uart);
	
	switch (uart)
	{
		case 'O':
			SwitchActivar(&activar);
			break;
	
		case 'H':
			SwitchHold(&hold);
			break;

    }
}

/**
 * @fn static void SensarTask(void *pvParameter)
 * @brief Función que mide la distancia utilizando el sensor HC-SR04 y guarda el resultado en la variable global.
 * @param pvParameter Parámetro no utilizado.
 */
static void SensarTask(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Espera notificación para ejecutar */
		//printf("Sensando\n");
		if (activar == true)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
	}
}

/**
 * @fn static void LedsTask(void *pvParameter)
 * @brief Función que controla el encendido y apagado de los LEDs según la distancia medida.
 * @param pvParameter Parámetro no utilizado.
 */
static void LedsTask(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Espera notificación para ejecutar */
		//printf("Leds\n");
		if (activar == true)
		{
			if (distancia <= 10)
			{
				LedsOffAll();
				//printf("Todos apagados\n");
			}
			else if (distancia <= 20)
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
				//printf("1 encendido, 2 y 3 apagados\n");
			}
			else if (distancia <= 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
				//printf("1 y 2 encendidos, 3 apagado\n");
			}
			else
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
				//printf("Todos encendidos\n");
			}
		}
		else
		{
			LedsOffAll();
		}
	}
}

/**
 * @fn static void DisplayTask(void *pvParameter)
 * @brief Función que muestra la distancia medida en el display. Si el sistema está en modo congelado, detiene la actualización.
 * @param pvParameter Parámetro no utilizado.
 */
static void DisplayTask(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Espera notificación para ejecutar */
		if (activar == true)
		{
			if (hold == false)
			{
				LcdItsE0803Write(distancia);
			}
			else
			{
				//printf("Display congelado\n");
			}
		}
		else
		{
			LcdItsE0803Off();
		}
	}
}

/**
 * @fn static void MandarDatosUART(void *pvParameter)
 * @brief función encargada de enviar datos por el puerto serie 
*/
static void EnviarDatosUART(void *pvParameter)
{
	while(true){ 
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(activar == true){
			UartSendString(UART_PC,(char*) UartItoa(distancia, 10));
			UartSendString(UART_PC, " cm\r\n");
		}		
	}
}

/*==================[definición de funciones externas]==========================*/

/**
 * @fn void app_main(void)
 * @brief Función principal del programa donde se inicializan los periféricos y se crean las tareas FreeRTOS.
 */
void app_main(void)
{
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2); // GPIO_3: Echo | GPIO_2: Trigger
	SwitchesInit();
	LcdItsE0803Init();

	SwitchActivInt(SWITCH_1, SwitchActivar, &activar);
	SwitchActivInt(SWITCH_2, SwitchHold, &hold);

	timer_config_t timer_A = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_uS,
		.func_p = FuncionTimer,
		.param_p = NULL};

	TimerInit(&timer_A);

	/// configuración de la UART
	serial_config_t Uart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = FuncionUART,
		.param_p = NULL
	};

	/// Inicializo el UART
	UartInit(&Uart);

	xTaskCreate(&SensarTask, "Sensar", 512, NULL, 5, &sensar_task_handle);
	xTaskCreate(&LedsTask, "Leds", 512, NULL, 5, &leds_task_handle);
	xTaskCreate(&DisplayTask, "Display", 512, NULL, 5, &display_task_handle);
	xTaskCreate(&EnviarDatosUART, "UART", 512, NULL, 4, &enviarDatosUART_task_handle);

	// Iniciar el timer, con la estructura TIMER_A
	TimerStart(timer_A.timer);
}

/*==================[end of file]============================================*/