/**
 * @mainpage Guía 1 - Ejercicio 2
 *
 * @section genDesc Descripción General
 *
 * Este ejercicio trata sobre el control de LEDs en función del estado de los interruptores. 
 * Se utiliza FreeRTOS para gestionar las tareas, y el programa enciende y apaga LEDs 
 * en función de los interruptores que están activos.
 *
 * @section changelog Changelog
 *
 * | Fecha    | Descripción                            |
 * |----------|----------------------------------------|
 * | 9/08/2024 | Creación del documento                  |
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
/**
 * @brief Período de parpadeo de los LEDs en milisegundos.
 */
#define CONFIG_BLINK_PERIOD 1000

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]========================*/
/**
 * @brief Función principal del programa.
 *
 * Esta función inicializa los LEDs y los interruptores, y entra en un bucle infinito en 
 * el que lee el estado de los interruptores y controla los LEDs en consecuencia. 
 * Los LEDs parpadean con un período definido si los interruptores están activos.
 */

void app_main(void) {
    uint8_t teclas; // Variable para almacenar el estado de los interruptores
    
    LedsInit(); // Inicializa los LEDs
    SwitchesInit(); // Inicializa los interruptores
    
    while (1) {
        teclas = SwitchesRead(); // Lee el estado de los interruptores
        
        switch (teclas) {
            case SWITCH_1:
                LedOn(LED_1); // Enciende el LED 1
                vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); // Espera por el período definido
                LedOff(LED_1); // Apaga el LED 1
                vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); // Espera por el período definido
                break;
            
            case SWITCH_2:
                LedOn(LED_2); // Enciende el LED 2
                vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); // Espera por el período definido
                LedOff(LED_2); // Apaga el LED 2
                vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); // Espera por el período definido
                break;
            
            case (SWITCH_1 | SWITCH_2):
                LedOn(LED_3); // Enciende el LED 3
                vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); // Espera por el período definido
                LedOff(LED_3); // Apaga el LED 3
                vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); // Espera por el período definido
                break;
            
            default:
                // Código a ejecutar por defecto (si es necesario)
                break;
        }
    }
}

/*==================[end of file]==========================================*/