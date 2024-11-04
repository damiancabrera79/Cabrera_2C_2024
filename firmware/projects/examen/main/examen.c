/*! @mainpage Alerta para ciclistas
 *
 * @section genDesc General Description
 *
 * Dispositivo de alerta para ciclistas basado en la ESP-EDU que detecta 
 * eventos peligrosos mediante un acelerómetro analógico en el casco y un 
 * sensor de ultrasonido en la parte trasera de la bicicleta. El 
 * dispositivo alerta al ciclista sobre la presencia de vehículos detrás de 
 * él mediante LEDs y una alarma sonora, y envía notificaciones a un 
 * smartphone a través de Bluetooth en caso de precaución o peligro. 
 * Además, detecta golpes o caídas mediante el acelerómetro y envía una 
 * notificación en caso de que la sumatoria de la aceleración supere los 4G.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |      ESP32c6	  	|   HC-SR04   	|
 * |:------------------:|:--------------|
 * | 	   GPIO_3	    |	  ECHO      |
 * |	   GPIO_2		|   TRIGGER     |
 * |		+5V			|     +5V       |
 * |		GND 		|	  GND       |


 * |	 ESP32c6        |   UART        |
 * |:------------------:|:--------------|
 * |		GND 		|	  GND       |
 * |	    GPIO_10     |   OUTPUT      |
 * 
 * |	 ESP32c6        |   BUZZER      |
 * |:------------------:|:--------------|
 * |	 GPIO_22        |   OUTPUT      |
 * |		GND 		|	  GND       |
 * 
 * 
 * |	 SP32c6         | ACELEROMETRO  |
 * |:------------------:|:--------------|
 * |	    CH1         |   C. A/D en X |
 * |	    CH2         |   C. A/D en Y |
 * |	    CH3         |   C. A/D en Z |
 * |		GND 		|	  GND       |

 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/11/2024 | Document creation		                         |
 *
 * @author Damian Cabrera (cabreradamian@gmail.com)
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
#include "switch.h"
#include "switch.h"
#include "hc_sr04.h"
#include "led.h"

/*==================[macros and definitions]=================================*/
/// Definición de períodos de tiempo que utilizaré
#define CONFIG_PERIOD_500_mSEG 500000
#define CONFIG_PERIOD_5_SEG 5000000
#define CONFIG_PERIOD_10_mSEG 10000

#define CONFIG_BLINK_PERIOD_500 500
#define CONFIG_BLINK_PERIOD_250 250

/** @var activar
 *  @brief Variable booleana global que registra si el sistema está activado o desactivado.
 */
bool activar = true;

/** @def BUZZER
 *  @brief Variable booleana global que registra si el buzze
*/
bool buzzer = false;

/**
 * @var distancia
 * @brief Variable uint16_t que almacena el valor de la distancia medida por el sensor HC-SR04.
 */
uint16_t distancia;

/**
 * @var valor_x
 * @brief Variable uint16_t resultado de la conversión AD del CH1
 */
uint16_t valor_x;

/**
 * @var valor_y
 * @brief Variable uint16_t resultado de la conversión AD del CH2
 */

uint16_t valor_y;	

/**
 * @var valor_z
 * @brief Variable uint16_t resultado de la conversión AD del CH3
 */

uint16_t valor_z;


/*==================[internal data definition]===============================*/

/**
 * @var sensar_task_handle
 * @brief Identificador de la tarea que se encarga de realizar la medición de distancia.
 */
TaskHandle_t sensar_distancia_task_handle = NULL;

/**
 * @var LedsTask_task_handle
 * @brief Identificador de la tarea que se encarga del encendido de los LEDS y mensajes 
 * por bluetooth
 */

TaskHandle_t LedsTask_task_handle = NULL;

/**
 * @var acelerometro_task_handle
 * @brief Identificador de tarea que se encarga del acelerometro y mensajes por bluetooth
 */

TaskHandle_t acelerometro_task_handle = NULL;


/** @struct gpioConfig_t
 *  @brief Estructura utilizada para configurar los pines GPIO
*/
typedef struct {
	gpio_t pin; ///< Pin GPIO a configurar
	io_t dir;   ///< Dirección del pin: entrada o salida
} gpioConfig_t;

/*==================[internal functions declaration]=========================*/

/**
 * @fn void FuncionTimer(void* param)
 * @brief Función invocada en la interrupción del timer A. Define lo que sucede cuando se utiliza el timer.
 * Envía notificaciones a las tareas que se deben ejecutar.
 * @param param Parámetro no utilizado.
 */
void funcionTimerA(void *param)
{
	vTaskNotifyGiveFromISR(sensar_distancia_task_handle, pdFALSE);	  /* Envía una notificación a la tarea de los LEDs */
	vTaskNotifyGiveFromISR(LedsTask_task_handle, pdFALSE);	  /* Envía una notificación a la tarea de los LEDs */
}

/**
 * @fn void FuncionTimer(void* param)
 * @brief Función invocada en la interrupción del timer B. Define lo que sucede cuando se utiliza el timer.
 * Envía notificaciones a las tareas que se deben ejecutar.
 * @param param Parámetro no utilizado.
 */
void funcionTimerB(void *param)
{
	vTaskNotifyGiveFromISR(acelerometro_task_handle, pdFALSE);	  /* Envía una notificación a la tarea de los LEDs */
}

/**
 * @fn static void SensarDistancia(void *pvParameter)
 * @brief Función que mide la distancia utilizando el sensor HC-SR04 y guarda el resultado en la variable global.
 * @param pvParameter Parámetro no utilizado.
 */
static void SensarDistancia(void *pvParameter)
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Espera notificación para ejecutar */
	if (activar == true)
	{
		distancia = HcSr04ReadDistanceInCentimeters();
	}
	
}

/**
 * @fn static void Se(void *pvParameter)
 * @brief Función que lee los valores de los CH1, CH2
 * también detecta si hay mas de 4G y envia una notificación
 * @param pvParameter Parámetro no utilizado.
 */

static void SensarAceleracion(void *pvParameter)
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Espera notificación para ejecutar */
	AnalogInputReadSingle(CH1, &valor_x);
	AnalogInputReadSingle(CH2, &valor_y);
	AnalogInputReadSingle(CH3, &valor_z);
	if ( (valor_x+valor_y+valor_z)> 4 * 0.3 * 65535 / 1.65)
	{
		UartSendString(UART_PC, "Caída detectada");
	}	
	
}

/**
 * @fn static void L(void *pvParameter)
 * @brief Función que enciende Leds de acuerdo a la distancia
 * tambien envia notificaciones por 
 * @param pvParameter Parámetro no utilizado.
 */

static void LedsTask(void *pvParameter)
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Espera notificación para ejecutar */
	
	if(activar == true){

			if(distancia > 500) {
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
				printf("LED VERDE encendido, AMARILLO y ROJO apagados\n");
			} else if(distancia <= 500 && distancia >300) {
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
				printf("LED VERDE y AMARILLO encendido, ROJO apagado\n");
				GPIOOn(GPIO_22); // Enciendo BUZZER
				vTaskDelay(CONFIG_BLINK_PERIOD_500 / portTICK_PERIOD_MS);
				GPIOOff(GPIO_22); // Apag BUZZER
				vTaskDelay(CONFIG_BLINK_PERIOD_500 / portTICK_PERIOD_MS);
				UartSendString(UART_PC, "Precaución, vehículo cerca");				
			} else {
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
				printf("LEDs VERDE AMARILLO Y ROJO ENCENDIDO\n");
				GPIOOn(GPIO_22); // Enciendo BUZZER
				vTaskDelay(CONFIG_BLINK_PERIOD_250 / portTICK_PERIOD_MS);
				GPIOOff(GPIO_22); // Apago BUZZER
				vTaskDelay(CONFIG_BLINK_PERIOD_250 / portTICK_PERIOD_MS);
				UartSendString(UART_PC, "Peligro, vehículo cerca");	
			}
	}
	else {
			LedsOffAll();
	}
	
}

/**
 * @fn void inicializarGPIO(gpioConfig_t *vectorGpio, int cantidad)
 * @brief Inicializa los pines y la dirección en los que van a trabajar (input o output)
 * @param vectorGpio Puntero a vector que contiene datos de tipo struct gpioConfig_t
 * @param cantidad Número de elementos en el vector de configuración GPIO
*/
void inicializarGPIO(gpioConfig_t *vectorGpio, int cantidad){
	for(int i=0; i<cantidad ; i++){
		GPIOInit(vectorGpio[i].pin, vectorGpio[i].dir);
	}
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
	printf("Switch 1 presionado\n");
}

/**
 * @fn void funcionUART(void *param)
 * @brief Configuración de la UART (sin recibir datos del puerto)
 * @param param Puntero a parámetros de la UART
*/
void funcionUART(void* param){
	// Como no se leen datos desde el puerto, no se requieren acciones aquí
}

/*==================[external functions definition]==========================*/
void app_main(void){

	// Inicializo los periféricos
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2); // GPIO_3: Echo | GPIO_2: Trigger
	SwitchesInit();

	gpioConfig_t vectorGpio[1] = {
		{GPIO_22, GPIO_OUTPUT},  // GPIO_22 se configura como salida.
	};

	analog_input_config_t Analog_config_x = {
		.input = CH1,       // Selecciona el canal CH3 como entrada analógica.
		.mode = ADC_SINGLE  // Configura el modo de conversión analógica como modo único.
	};
	AnalogInputInit(&Analog_config_x); // Inicializa la entrada analógica con la configuración especificada.

	analog_input_config_t Analog_config_y = {
		.input = CH2,       // Selecciona el canal CH3 como entrada analógica.
		.mode = ADC_SINGLE  // Configura el modo de conversión analógica como modo único.
	};
	AnalogInputInit(&Analog_config_y); // Inicializa la entrada analógica con la configuración especificada.

	analog_input_config_t Analog_config_z = {
		.input = CH3,       // Selecciona el canal CH3 como entrada analógica.
		.mode = ADC_SINGLE  // Configura el modo de conversión analógica como modo único.
	};
	AnalogInputInit(&Analog_config_z); // Inicializa la entrada analógica con la configuración especificada.

	inicializarGPIO(vectorGpio, 4); // Inicializa los GPIOs indicados con sus respectivas configuraciones.
	
	// Defino las interrupciones para los Swicth
	SwitchActivInt(SWITCH_1, SwitchActivar, &activar);

	// Inicialización de los switches:
	SwitchesInit(); // Configura los pines de los interruptores de encendido y apagado para ser usados en el sistema.

	// Configuración de la UART para comunicación en serie.
	serial_config_t Uart = {
		.port = GPIO_10,          // Puerto UART a utilizar (definido como UART_PC).
		.baud_rate = 9800,        // Tasa de baudios configurada a 9800.
		.func_p = funcionUART,    // Callback `funcionUART` para gestionar eventos UART.
		.param_p = NULL           // Sin parámetros adicionales para la función.
	};
	
	UartInit(&Uart); // Inicializa la UART con la configuración definida.
	
	// Configuración e inicialización del timer A con un periodo de 3 segundos.
	timer_config_t timerA = {
		.timer = TIMER_A,              // Selección del timer A.
		.period = CONFIG_PERIOD_500_mSEG, // Periodo de 0.5segundos segundos para el timer.
		.func_p = funcionTimerA,       // Callback `funcionTimerA` que se ejecuta cada 3 segundos.
		.param_p = NULL                // Sin parámetros adicionales para la función.
	};
	TimerInit(&timerA); // Inicializa el timer A.

	timer_config_t timerB = {
		.timer = TIMER_B,                // Selección del timer B.
		.period = CONFIG_PERIOD_10_mSEG, // Frecuencia 100Hz.
		.func_p = funcionTimerB,         // Callback `funcionTimerB` que se ejecuta cada 5 segundos.
		.param_p = NULL                  // Sin parámetros adicionales para la función.
	};
	TimerInit(&timerB); // Inicializa el timer B.

		
	// Creación de tareas en FreeRTOS.
	xTaskCreate(&SensarDistancia, "SensarDistancia", 2048, NULL, 4, &sensar_distancia_task_handle);
	xTaskCreate(&LedsTask, "LedsTask", 2048, NULL, 4, &LedsTask_task_handle); 
	xTaskCreate(&SensarAceleracion, "SensarAceleracion", 2048, NULL, 4, &acelerometro_task_handle); 


	// Inicia los timers A y B.
	TimerStart(timerA.timer); // Inicia el timer A con su periodo de 3 segundos.
	TimerStart(timerB.timer); // Inicia el timer A con su periodo de 3 segundos.
}
/*==================[end of file]============================================*/