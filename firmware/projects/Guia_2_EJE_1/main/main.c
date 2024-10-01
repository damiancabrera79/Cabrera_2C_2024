/*! @mainpage Guía 2 - Ejercicio 1
* @section genDesc Descripción General
*
* Este programa controla el encendido y apagado de LEDs, la lectura mediante un sensor de distancia HC-SR04,
* y la visualización de la distancia en un display LCDITSE0803 utilizando un ESP32. 
* 
* El sistema permite:
* - Encender los LEDs en secuencia según la distancia medida (si es menor a 10 cm, no se enciende ningún LED;
* si está entre 10 y 20 cm, se enciende el LED 1; si está entre 20 y 30 cm, se encienden los LEDs 1 y 2;
* si es mayor a 30 cm, se encienden los LEDs 1, 2 y 3).
* - Activar o desactivar el sistema presionando el botón 1.
* - Mostrar la distancia medida en el display con la opción de congelar la lectura al presionar el botón 2.
* 
* 
* @section hardConn Hardware Connection
*
* |    ESP32c6	  	|   HC-SR04   	|
* |:---------------:|:--------------|
* | 	GPIO_3	    |	  ECHO      |
* |		GPIO_2		|   TRIGGER     |
* |		+5V			|     +5V       |
* |		GND 		|	  GND       |
* 
* |	  ESP32c6		|	Lcditse0803 |
* |:--------------:| :--------------|	
* |    	GPIO_19		|     SEL_1     |
* |    	GPIO_18		|	  SEL_2     |
* |    	GPIO_9	    |	  SEL_3     |
* |    	GPIO_20		|	   D1		|
* |    	GPIO_21		|	   D2		|
* | 	GPIO_22		|	   D3		|
* | 	GPIO_23		|	   D4		|
* | 	+5V			|	  +5V		|
* | 	GND			|	  GND		|
*
*
* @section changelog Changelog
*
* |   Fecha	     | Descripción                                   |
* |:------------:|:----------------------------------------------|
* | 02/09/2024   | Creación del documento		                 |
* | 30/09/2024   | Verificar comentarios con printf              |
* |  			 | que no funcionaban   						 |

* @author Damian Cabrera (cabreradamian@gmail.com)
*
*/


/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/

/**
 * @def CONFIG_BLINK_PERIOD_1000
 * @brief Período de refresco del display en milisegundos.
 * 
 * Esta macro define el intervalo de tiempo en milisegundos (1000 ms) para el refresco
 * de la pantalla, lo que equivale a una actualización cada segundo.
 */
#define CONFIG_BLINK_PERIOD_1000 1000

/**
 * @def CONFIG_BLINK_PERIOD_200
 * @brief Período de comprobación de switches en milisegundos.
 * 
 * Esta macro define el intervalo de tiempo en milisegundos (200 ms) para comprobar
 * si se ha presionado algún switch o botón. La verificación ocurre cada 200 ms.
 */
#define CONFIG_BLINK_PERIOD_200 200

/** @var activar
 *  @brief Variable booleana global que registra si el sistema está activado o desactivado.
 */
bool activar = true; 

/** @var hold
 *  @brief Variable booleana global que registra si el display está congelado o en modo normal.
 */
bool hold = false;

/**
 * @var distancia
 * @brief Variable uint16_t que almacena el valor de la distancia medida por el sensor HC-SR04.
 */
uint16_t distancia;

/*==================[internal data definition]===============================*/

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

/**
 * @var switches_task_handle
 * @brief Identificador de la tarea que se encarga de detectar la pulsación de los botones.
 */
TaskHandle_t switches_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @fn static void SwitchesTask(void *pvParameter)
 * @brief Función que detecta la pulsación de los botones y cambia el estado de las variables de control del sistema.
 * - Botón 1: Activa o desactiva el sistema.
 * - Botón 2: Congela o descongela el valor mostrado en el display.
 * @param pvParameter Puntero que no es utilizado.
 */
static void SwitchesTask(void *pvParameter){
	
	uint8_t tecla;
	
	while(true){
		
		printf("Switches\n");
		tecla = SwitchesRead();

		if (tecla == SWITCH_1){
			activar = !activar;
			printf("Switch 1 presionado\n");
		}
		if (tecla == SWITCH_2){
			hold = !hold;
			printf("Switch 2 presionado\n");
		}
		
		vTaskDelay(CONFIG_BLINK_PERIOD_200 / portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void SensarTask(void *pvParameter)
 * @brief Función que mide la distancia utilizando el sensor HC-SR04 y guarda el resultado en la variable global distancia.
 * @param pvParameter Puntero que no es utilizado.
 */
static void SensarTask(void *pvParameter){
	
	while(true){
	
		if(activar == true){
			distancia = HcSr04ReadDistanceInCentimeters();
			printf("Sensando distancia en Centímetros\n");
		}
		
		vTaskDelay(CONFIG_BLINK_PERIOD_1000 / portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void LedsTask(void *pvParameter)
 * @brief Función que controla el encendido y apagado de los LEDs según la distancia medida:
 * - Si la distancia es menor o igual a 10 cm: Todos los LEDs apagados.
 * - Si la distancia es mayor a 10 cm y menor o igual a 20 cm: Enciende el LED 1.
 * - Si la distancia es mayor a 20 cm y menor o igual a 30 cm: Enciende los LEDs 1 y 2.
 * - Si la distancia es mayor a 30 cm: Enciende los LEDs 1, 2 y 3.
 * @param pvParameter Puntero que no es utilizado.
 */
static void LedsTask(void *pvParameter){
	
	while(true){
		
		printf("Leds\n");
		
		if(activar == true){

			if(distancia <= 10) {
				LedsOffAll();
				printf("Todos apagados\n");
			} else if(distancia <= 20) {
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
				printf("1 encendido, 2 y 3 apagados\n");
			} else if(distancia <= 30) {
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
				printf("1 y 2 encendidos, 3 apagado\n");
			} else {
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
				printf("Todos encendidos\n");
			}
		}
		else {
			LedsOffAll();
		}
		
		vTaskDelay(CONFIG_BLINK_PERIOD_1000 / portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void DisplayTask(void *pvParameter)
 * @brief Función que muestra la distancia medida en el display. Si el sistema está activado y no está en modo congelado,
 * actualiza el valor del display. Si el sistema está desactivado, apaga el display.
 * @param pvParameter Puntero que no es utilizado.
 */
static void DisplayTask(void *pvParameter){
	
	while(true){
	
		if(activar == true){
			
			if(hold == false){
				LcdItsE0803Write(distancia);
				printf("Distancia: %d cm\n", distancia);
			} else {
				printf("Display congelado\n");
			}
		}
		else{
			LcdItsE0803Off();
		} 
		vTaskDelay(CONFIG_BLINK_PERIOD_1000 / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/

/**
 * @fn void app_main(void)
 * @brief Función principal del programa que inicializa los periféricos (LEDs, switches, sensor de distancia y display LCD) 
 * y crea las tareas FreeRTOS que manejan los diferentes componentes del sistema.
 * 
 * En esta función se configuran los siguientes periféricos:
 * - Inicialización de LEDs con `LedsInit()`.
 * - Inicialización de los botones o switches con `SwitchesInit()`.
 * - Inicialización del sensor de distancia HC-SR04 con `HcSr04Init()`.
 * - Inicialización del display LCD ITSE0803 con `LcdItsE0803Init()`.
 * 
 * Luego, se crean cuatro tareas con FreeRTOS para ejecutar en paralelo:
 * - `SwitchesTask`: Lee los botones y activa o desactiva el sistema, o congela el valor del display.
 * - `SensarTask`: Mide la distancia usando el sensor HC-SR04.
 * - `LedsTask`: Enciende o apaga los LEDs según la distancia medida.
 * - `DisplayTask`: Muestra la distancia en el display y permite congelar la lectura.
 */
void app_main(void){

	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);  // GPIO_3: Echo | GPIO_2: Trigger
	SwitchesInit();
	LcdItsE0803Init();

	/// Creación de las tareas en FreeRTOS
	xTaskCreate(&SwitchesTask, "Switches", 2048, NULL, 5, &switches_task_handle);
	xTaskCreate(&SensarTask, "Sensar", 2048, NULL, 5, &sensar_task_handle);
	xTaskCreate(&LedsTask, "Leds", 2048, NULL, 5, &leds_task_handle);
	xTaskCreate(&DisplayTask, "Display", 2048, NULL, 5, &display_task_handle);
}
/*==================[end of file]============================================*/