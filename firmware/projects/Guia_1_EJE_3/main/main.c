/*! @mainpage Guía 1 - Ejercicio 3
 *
 * \section genDesc General Description
 *
 * Este ejemplo implementa el control de varios LEDs mediante una estructura que define
 * el modo de operación (encender, apagar o alternar), el número de ciclos de encendido/apagado
 * y el periodo de cada ciclo en milisegundos.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 16/08/2024 | Creación del documento		                 |
 *
 * @author Damian Cabrera (cabreradamian@gmail.com)
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500 // Periodo de parpadeo en milisegundos

#define ON 1     // Modo para encender el LED
#define OFF 2    // Modo para apagar el LED
#define TOGGLE 3 // Modo para alternar el estado del LED

/*==================[internal data definition]===============================*/
// Definición de la estructura para controlar los LEDs
typedef struct
{
    uint8_t mode;       // Modo de operación del LED (ON, OFF, TOGGLE)
	uint8_t n_led;      // Número del LED a controlar
	uint8_t n_ciclos;   // Cantidad de ciclos de encendido/apagado
	uint16_t periodo;   // Tiempo en milisegundos de cada ciclo
} tipo_led;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Función para controlar el estado de los LEDs.
 *
 * Esta función controla el estado de un LED según el modo especificado
 * (ON, OFF, TOGGLE) y realiza la operación durante el número de ciclos
 * indicado con el periodo especificado.
 *
 * @param est_led Puntero a una estructura tipo_led que contiene la configuración
 *                del LED.
 *
 * @note Dependiendo del modo, el LED puede permanecer encendido, apagado,
 *       o alternar entre encendido y apagado.
 */

void funcion_leds (tipo_led *est_led) {
    switch (est_led->mode) {
        case ON: {
            // Mantener el LED encendido durante n_ciclos * periodo
            for (uint8_t i = 0; i < est_led->n_ciclos; i++) {
                switch (est_led->n_led) {
                    case 1:
                        LedOn(LED_1);
                        break;
                    case 2:
                        LedOn(LED_2);
                        break;
                    case 3:
                        LedOn(LED_3);
                        break;
                    default:
                        break;
                }
                // Espera el periodo especificado
                vTaskDelay(est_led->periodo / portTICK_PERIOD_MS);
            }

            // Apagar el LED después de los ciclos
            switch (est_led->n_led) {
                case 1:
                    LedOff(LED_1);
                    break;
                case 2:
                    LedOff(LED_2);
                    break;
                case 3:
                    LedOff(LED_3);
                    break;
                default:
                    break;
            }
            break;
        }
        case OFF: {	
            switch (est_led->n_led) {				
                case 1:
                    LedOff(LED_1);
                    break;
                case 2:
                    LedOff(LED_2);
                    break;
                case 3:
                    LedOff(LED_3);
                    break;
                default:
                    break;
            }
            break;  // Salir del switch
        }
        case TOGGLE: {
            uint8_t	n_ciclo_aux;
            for (n_ciclo_aux = 0; n_ciclo_aux < est_led->n_ciclos; n_ciclo_aux++) {
                switch (est_led->n_led) {
                    case 1:
                        LedOn(LED_1);
                        vTaskDelay(est_led->periodo / portTICK_PERIOD_MS);
                        LedOff(LED_1);
                        vTaskDelay(est_led->periodo / portTICK_PERIOD_MS);
                        break;
                    case 2:
                        LedOn(LED_2);
                        vTaskDelay(est_led->periodo / portTICK_PERIOD_MS);
                        LedOff(LED_2);
                        vTaskDelay(est_led->periodo / portTICK_PERIOD_MS);						
                        break;
                    case 3:
                        LedOn(LED_3);
                        vTaskDelay(est_led->periodo / portTICK_PERIOD_MS);
                        LedOff(LED_3);
                        vTaskDelay(est_led->periodo / portTICK_PERIOD_MS);						
                        break;
                    default:
                        break;
                }
            }
            break;  // Salir del switch
        }
        default:
            break;
    }
}


/*==================[external functions definition]==========================*/

/**
* @brief Llama a la función que controla el comportamiento del LED según la configuración.
*
* Esta función toma la configuración especificada en la estructura 'tipo_led' y
* realiza la operación correspondiente sobre el LED. El LED será encendido, apagado,    
* o alternará su estado basado en el modo de operación y los ciclos especificados.
*     
* @param &leds Puntero a la estructura 'tipo_led' que contiene la configuración del LED.
* 
* @note Asegúrate de que 'LedsInit()' haya sido llamada antes de usar 'funcion_leds()'.
*/          

void app_main(void) {
    LedsInit();  // Inicializa los LEDs para prepararlos para el control

    // Configuración del LED para parpadeo
    tipo_led leds;
    leds.n_led = 2;          // Número del LED a controlar. En este caso, el LED 2.
    leds.n_ciclos = 10;       // Número de ciclos de encendido/apagado. El LED parpadeará 10 veces.
    leds.periodo = 500;     // Periodo de encendido/apagado en milisegundos. Cada ciclo durará 1 segundo.
    //leds.mode = ON;          // Modo de operación del LED. Puede ser ON, OFF o TOGGLE.    LedsInit();
    leds.mode = TOGGLE;    // Cambiado a TOGGLE para cumplir el objetivo de encender y apagar en ciclos
    funcion_leds(&leds);
}
/*==================[end of file]==========================================*/