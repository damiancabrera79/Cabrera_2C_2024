/*! @mainpage Sistema de pesaje de camiones basado en la placa ESP-EDU
 *
 * @section genDesc General Description
 *
 * Sistema de pesaje de camiones basado en la placa ESP-EDU que verifica la 
 * velocidad de ingreso de los vehículos y pesa el vehículo cuando se detiene. 
 * El sistema utiliza un sensor de distancia HC-SR04 para medir la velocidad 
 * y dos galgas para medir el peso. 
 * Los LEDs de la ESP-EDU se utilizan como señales de advertencia de velocidad. 
 * El sistema envía el peso y la velocidad máxima del vehículo a la PC del 
 * operario a través de la UART y permite controlar la apertura y cierre de 
 * una barrera mediante comandos enviados desde la PC.
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


 * |	 ESP32c6        |     UART      |
 * |:------------------:|:--------------|
 * |		GND 		|	  GND       |
 * |	 UART_PC        | INPUT/OUTPUT  |
 * 
 * |	 ESP32c6        |   BARRERA     |
 * |:------------------:|:--------------|
 * |	 GPIO_18        |   OUTPUT      |
 * |		GND 		|	  GND       |
 * 
 * 
 * |	 SP32c6         |    GALGAS		|
 * |:------------------:|:--------------|
 * |	    CH1         |   C. A/D      |
 * |	    CH2         |   C. A/D      |
 * |		GND 		|	  GND       |

 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/11/2024 | Document creation		                         |
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
#include "hc_sr04.h"
#include "led.h"
#include "ctype.h"

/*==================[macros and definitions]=================================*/
// Definición de períodos de tiempo que utilizare
#define CONFIG_PERIOD_100_mSEG 100000 // 10 muestras por segundo
#define CONFIG_PERIOD_5_mSEG 5000 //200 muestras segundo

/**
 * @var distance
 * @brief Variable uint16_t que almacena el valor de la distancia medida por el sensor HC-SR04.
 */
uint16_t distance;

/**
 * @var distance_samples
 * @brief Variable uint16_t que almacena el valor de la distancia medida por el sensor HC-SR04.
 */
uint16_t distance_samples[10];

/**
 * @var peso1
 * @brief Variable uint16_t que almacena el valor del peso galga 1
 */
uint32_t peso1[50];

/**
 * @var peso2
 * @brief Variable uint16_t que almacena el valor del peso galga 2
 */
uint32_t peso2[50];

/*==================[internal data definition]===============================*/

/**
 * @var sensar_task_handle
 * @brief Identificador de la tarea que se encarga de realizar la medición de distancia.
 */
TaskHandle_t sensar_distancia_task_handle = NULL;

/**
 * @var medir_peso_task_handle
 * @brief Identificador de tarea que se encarga de medir el peso del camion 
*/
TaskHandle_t medir_peso_task_handle = NULL;

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
}

/**
 * @fn void FuncionTimer(void* param)
 * @brief Función invocada en la interrupción del timer B. Define lo que sucede cuando se utiliza el timer.
 * Envía notificaciones a las tareas que se deben ejecutar.
 * @param param Parámetro no utilizado.
 */
void funcionTimerB(void *param)
{
	vTaskNotifyGiveFromISR(medir_peso_task_handle, pdFALSE);	  /* Envía una notificación a la tarea de los LEDs */
}

/**
 * @fn static void SensarDistancia(void *pvParameter)
 * @brief Función que mide la distancia utilizando el sensor HC-SR04 y guarda el resultado en la variable global.
 * @param pvParameter Parámetro no utilizado.
 */
static void SensarDistancia(void *pvParameter)
{
	uint16_t distancia_anterior = 0;  // Inicializamos la distancia previa
	uint16_t distancia_actual = 0;
	
	float velocidad = 0;
	const float tiempo_muestreo = 0.1; // Tiempo entre mediciones en segundos (10 Hz)

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Espera notificación para ejecutar */

		// Realiza 10 lecturas del sensor y calcula el promedio.
		uint16_t distancia_suma = 0;
		for (int i = 0; i < 10; i++) {
			distance_samples[i] = HcSr04ReadDistanceInCentimeters(); // Lee el sensor
			distancia_suma += distance_samples[i];
		}
		distancia_actual = distancia_suma / 10;  // Promedio de 10 muestras.

		if (distancia_actual < 1000)
		{
			// Calcular la velocidad (en cm/s) como diferencia de distancia entre dos momentos
			velocidad = (float)(distancia_actual - distancia_anterior) / tiempo_muestreo;

			// Actualizar distancia anterior para la próxima iteración
			distancia_anterior = distancia_actual;

			// Comprobar la velocidad y encender los LEDs correspondientes
			if (velocidad > 800) {  // 8 m/s (800 cm/s)
				LedOn(LED_3);
				LedOff(LED_1);
				LedOff(LED_2);
			} 
			else if (velocidad > 0) {  // mayor a 0 m/s y 8 m/s
				LedOn(LED_2);
				LedOff(LED_1);
				LedOff(LED_3);
			} 
			else {  // camión detenido
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
		}
	}
}


/**
 * @brief Tarea para sensar aceleración en los ejes X, Y y Z y detectar caídas.
 * @param pvParameter Puntero de parámetros para la tarea.
 */
static void MedirPeso(void *pvParameter)
{
    while (true)
    {
        // Espera una notificación para ejecutar la lectura y el cálculo de aceleración
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		// Realiza 50 lecturas del sensor1
        for (int i = 0; i < 50; i++) {
            AnalogInputReadSingle(CH1, &peso1[i]); 
        }

		// Calcula el promedio de las muestras.
        uint32_t peso1_sum = 0; 
        for (int i = 0; i < 50; i++) {
           	peso1_sum += peso1[i] * (20000/3.3);  // Suma las lectura del sensor
		}
            	
		uint32_t peso_galga_1 = peso1_sum / 50;  // Promedio de las 50 muestras.

		// Realiza 50 lecturas del sensor2
        for (int i = 0; i < 50; i++) {
            AnalogInputReadSingle(CH2, &peso2[i]); 
        }

		// Calcula el promedio de las muestras.
        uint32_t peso2_sum = 0; 
        for (int i = 0; i < 50; i++) {
           	peso2_sum += peso2[i] * (20000/3.3);  // Suma las lectura del sensor
		}
            	
		uint32_t peso_galga_2 = peso2_sum / 50;  // Promedio de las 50 muestras.

		
		// sumo ambos promedios para hallar el peso total
		uint32_t peso_total = peso_galga_1 + peso_galga_2;

	    UartSendString(UART_PC, "Peso:");
		UartSendString(UART_PC,(char*) UartItoa(peso_total, 10));
		UartSendString(UART_PC, "Velocidad máxima: 10m/s");
		UartSendString(UART_PC, "\r");
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
 * @fn void FuncionUART(void *param)
 * @brief Configuración de la UART
 * @param param Puntero a parámetros de la UART
*/
void FuncionUART(void* param){
	uint8_t uart;
	UartReadByte(UART_PC, &uart);
	uart = toupper(uart);
	
	switch (uart)
	{
		case 'O':
			GPIOOn(GPIO_18);
			break;
	
		case 'C':
			GPIOOff(GPIO_18);
			break;
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){

	// Inicializo los periféricos
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2); // GPIO_3: Echo | GPIO_2: Trigger

	gpioConfig_t vectorGpio[1] = {
		{GPIO_18, GPIO_OUTPUT},  // GPIO_18 se configura como salida (Barrera)
	};

	/// Inicializa los GPIOs indicados con sus respectivas configuraciones.
	inicializarGPIO(vectorGpio, 1); 

		
	/// Galga 1: CH1 | Galga 2: CH2
	analog_input_config_t Analog_config_x = {
		.input = CH1,       // Selecciona el canal CH1 como entrada analógica.
		.mode = ADC_SINGLE  // Configura el modo de conversión analógica como modo único.
	};
	AnalogInputInit(&Analog_config_x); // Inicializa la entrada analógica con la configuración especificada.

	analog_input_config_t Analog_config_y = {
		.input = CH2,       // Selecciona el canal CH2 como entrada analógica.
		.mode = ADC_SINGLE  // Configura el modo de conversión analógica como modo único.
	};
	AnalogInputInit(&Analog_config_y); // Inicializa la entrada analógica con la configuración especificada.

	/// Configuración de la UART para comunicación en serie.
	serial_config_t Uart = {
		.port = UART_PC,   // Puerto UART a utilizar (definido como UART_CONNECTOR).
		.baud_rate = 9600,        // Tasa de baudios configurada a 9600.
		.func_p = FuncionUART,    // Callback `funcionUART` para gestionar eventos UART.
		.param_p = NULL           // Sin parámetros adicionales para la función.
	};
	
	UartInit(&Uart); // Inicializa la UART con la configuración definida.
	
	/// Configuración e inicialización del timer A con un periodo de 3 segundos.
	timer_config_t timerA = {
		.timer = TIMER_A,              		// Selección del timer A.
		.period = CONFIG_PERIOD_100_mSEG, 	// Periodo de 100 msegundos segundos para el timer.
		.func_p = funcionTimerA,       		// Callback funcionTimerA
		.param_p = NULL                		// Sin parámetros adicionales para la función.
	};
	TimerInit(&timerA); // Inicializa el timer A.

	timer_config_t timerB = {
		.timer = TIMER_B,                // Selección del timer B.
		.period = CONFIG_PERIOD_5_mSEG,  // 200 muestras por segundo.
		.func_p = funcionTimerB,         // Callback funcionTimerB
		.param_p = NULL                  // Sin parámetros adicionales para la función.
	};
	TimerInit(&timerB); // Inicializa el timer B.

	/// Creación de tareas en FreeRTOS.
	xTaskCreate(&SensarDistancia, "SensarDistancia", 2048, NULL, 4, &sensar_distancia_task_handle);
	xTaskCreate(&MedirPeso, "MedirPeso", 2048, NULL, 4, &medir_peso_task_handle);

	/// Inicia los timers A y B.
	TimerStart(timerA.timer); // Inicia el timer A con su periodo
	TimerStart(timerB.timer); // Inicia el timer A con su periodo
}
/*==================[end of file]============================================*/