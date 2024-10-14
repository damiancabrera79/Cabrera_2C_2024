/*!
 * @mainpage Guía 2 - Ejercicio 4 - Primera parte
 * @section genDesc Descripción General
 *
 * Este programa dispara la conversión A/D a través de una interrupción de timer
 * cada 2 ms, lo que genera una frecuencia de muestreo de 500 Hz.
 * 
 * Los datos de la conversión son transmitidos por UART en formato ASCII y visualizados
 * mediante un graficador serie de código abierto llamado "Serial Oscilloscope".
 * 
 * El formato de envío de datos es "11\r". Por ejemplo, si hubiera más de un canal,
 * sería "11,22,33\r", donde 11, 22 y 33 corresponden a los canales 1, 2 y 3 respectivamente.
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

/**
 * @brief Período del Timer A en microsegundos.
 */
#define CONFIG_PERIOD_uS_A 2000 ///< 2ms período del Timer

/**
 * @brief Variable global para almacenar el valor leído del ADC.
 */
uint16_t valores; ///< Valor leído del ADC

/*==================[internal data definition]===============================*/

/**
 * @brief Handle de la tarea encargada de la conversión ADC.
 */
TaskHandle_t conversion_ADC_task_handle = NULL; ///< Handle de la tarea ADC

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función de la tarea encargada de la conversión del ADC.
 * 
 * Esta función ejecuta la conversión ADC periódicamente cuando recibe una notificación
 * de la interrupción del Timer A. Convierte los datos a formato ASCII y los envía por UART.
 * 
 * @param pParam Puntero a los parámetros de la tarea (no se utiliza).
 */
static void conversionADC(void *pParam) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // Lee un valor analógico (ADC) del canal CH1 y lo guarda en la variable global valores.
		AnalogInputReadSingle(CH1, &valores);
        // La función Itoa (Integer to ASCII) convierte el valor numérico (entero) obtenido de 
		// la lectura analógica en su representación de texto (ASCII).
		// Esta función envía la cadena de caracteres resultante (la conversión ASCII) a través 
		// del puerto UART que está conectado a la PC
        UartSendString(UART_PC, (char*) UartItoa(valores, 10));
		// Esta línea envía el carácter de retorno de carro (\r) al final de la cadena de caracteres
		UartSendString(UART_PC, "\r");
    }
}

/**
 * @brief Función que se ejecuta en la interrupción del Timer A.
 * 
 * Esta función es llamada por la interrupción del timer y notifica a la tarea ADC
 * que realice una nueva conversión.
 * 
 * @param pParam Puntero a los parámetros de la función (no se utiliza).
 */
void funcionTimerA(void *pParam) {
    vTaskNotifyGiveFromISR(conversion_ADC_task_handle, pdFALSE); ///< Notificación a la tarea ADC
}

/**
 * @brief Función de la UART (aún no utilizada).
 * 
 * En esta función se podría manejar la lectura de datos desde la UART en caso de ser necesario.
 * 
 * @param param Puntero a los parámetros de la función (no se utiliza).
 */
void funcionUART(void *param) {
    // Pendiente de implementación para leer desde el puerto serie si es necesario
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación.
 * 
 * Esta función inicializa el ADC, el Timer, la UART y crea la tarea de conversión ADC.
 * El timer dispara la conversión ADC cada 2 ms, y los valores son enviados por UART.
 */
void app_main(void) {
    // Configuración del canal ADC en modo de conversión única
    analog_input_config_t Analog_config = {
        .input = CH1,
        .mode = ADC_SINGLE
    };

    AnalogInputInit(&Analog_config); ///< Inicializa el canal ADC
    AnalogOutputInit(); ///< Inicializa la salida analógica

    // Configuración del Timer A
    timer_config_t timerA = {
        .timer = TIMER_A,
        .period = CONFIG_PERIOD_uS_A,
        .func_p = funcionTimerA,
        .param_p = NULL
    };

    TimerInit(&timerA); ///< Inicialización del Timer A

    // Configuración de la UART
    serial_config_t Uart = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = funcionUART,
        .param_p = NULL
    };

    UartInit(&Uart); ///< Inicializo la UART

    // Creación de la tarea de conversión ADC
    xTaskCreate(&conversionADC, "digitalizar", 512, NULL, 5, &conversion_ADC_task_handle);

    TimerStart(timerA.timer); ///< Inicio del Timer A
}

/*==================[end of file]============================================*/
