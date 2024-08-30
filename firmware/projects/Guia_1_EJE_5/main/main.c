/*! @mainpage Guía 1 - Ejercicio 5
 *
 * \section genDesc Descripción General
 *
 * Este ejercicio trata sobre la configuración de pines GPIO basándose en un dígito BCD 
 * (Binary-Coded Decimal). Cada bit del dígito BCD controla el estado de un pin GPIO 
 * específico, configurándolo en alto (1) o bajo (0) según el valor del bit correspondiente.
 *
 * @section changelog Changelog
 *
 * |   Fecha	  | Descripción                                   |
 * |:------------:|:----------------------------------------------|
 * | 30/08/2024   | Creación del documento		                  |
 *
 *  @author Damian Cabrera (cabreradamian@gmail.com)
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"  // Incluye el archivo de cabecera necesario para manipular los GPIO

/*==================[macros and definitions]=================================*/

/* Definir macros para mayor claridad en la dirección de los GPIO */
#define OUTPUT 1  /*!< Dirección de GPIO configurada como salida */

/*==================[internal data definition]===============================*/

 /* Definir el vector de configuraciones de GPIO */
gpioConf_t gpio_conf[4] = {
    {GPIO_20, OUTPUT},  // Configuración del GPIO_20 como salida
    {GPIO_21, OUTPUT},  // Configuración del GPIO_21 como salida
    {GPIO_22, OUTPUT},  // Configuración del GPIO_22 como salida
    {GPIO_23, OUTPUT}   // Configuración del GPIO_23 como salida
};

/*==================[internal functions declaration]=========================*/

/**
 * @brief Configura y cambia el estado de los pines GPIO según el dígito BCD.
 *
 * Esta función toma un dígito BCD y un vector de configuraciones de GPIO. Cada bit del dígito BCD
 * se usa para definir el estado del correspondiente pin GPIO: si el bit es 1, el GPIO se configura
 * en el estado alto; si el bit es 0, el GPIO se configura en el estado bajo.
 *
 * @param bcd_digito Dígito BCD que define el estado de los pines GPIO (0-15).
 * @param gpio_conf Vector de configuraciones de GPIO donde se almacenan los pines y sus direcciones.
 * 
 * @note El tamaño del vector `gpio_conf` debe ser al menos de 4 elementos, cada uno correspondiente
 *       a un bit del dígito BCD (b0 a b3).
 *
 * @return void
 */
void configurarGpios(uint8_t bcd_digito, gpioConf_t gpio_conf[4]) {
    // Asegurarse de que el vector tiene al menos 4 elementos y que no es nulo
    if (gpio_conf == NULL) {
        return;
    }

    // Iterar sobre los 4 bits del dígito BCD (de b0 a b3)
    for (int i = 0; i < 4; i++) {
        // Configurar la dirección del GPIO (0 -> IN, 1 -> OUT)
        gpioConfig(gpio_conf[i].pin, gpio_conf[i].dir);

        // Verificar el estado del bit correspondiente
        if (bcd_digito & (1 << i)) {
            // Si el bit está en 1, configurar el GPIO en alto (set)
            gpioSet(gpio_conf[i].pin);
        } else {
            // Si el bit está en 0, configurar el GPIO en bajo (clear)
            gpioClear(gpio_conf[i].pin);
        }
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación
 * 
 * En esta función se define un vector de configuraciones de GPIOs, se llama a la función
 * 'configurarGpios' para configurar el estado de los pines GPIO según un dígito BCD específico.
 * El dígito BCD se usa para definir los estados de los GPIOs, donde cada bit del dígito determina
 * si el GPIO correspondiente debe estar en alto o bajo.
 */
void app_main(void) {
    
   

    /* Configurar los GPIOs con el dígito BCD 0b1010 (10 en decimal) */
    configurarGpios(0b1010, gpio_conf);

    /* El GPIO_20 y GPIO_22 se configurarán en alto, mientras que GPIO_21 y GPIO_23 estarán en bajo */
}

/*==================[end of file]==========================================*/
