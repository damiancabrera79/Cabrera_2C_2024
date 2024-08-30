/*! @mainpage Guía 1 - Ejercicio 5
 *
 * @section genDesc Descripción General
 *
 * Este ejercicio trata sobre la configuración de pines GPIO basada en un dígito BCD 
 * (Decimal Codificado en Binario). Cada bit del dígito BCD controla el estado de un pin GPIO 
 * específico, configurándolo en alto (1) o bajo (0) según el valor del bit correspondiente.
 *
 * @section changelog Changelog
 *
 * |   Fecha	  | Descripción                                   |
 * |:------------:|:----------------------------------------------|
 * | 30/08/2024   | Creación del documento		                  |
 *
 * @author Damian Cabrera (cabreradamian@gmail.com)
 */

/*==================[inclusions]=============================================*/
/**
 * @file main.c
 * @brief Archivo principal que incluye las bibliotecas necesarias.
 *
 * Este archivo contiene las funciones para la configuración y control de los GPIOs 
 * basados en un dígito BCD. Utiliza las librerías FreeRTOS y gpio_mcu.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"  /*!< Incluye el archivo de cabecera necesario para los GPIOs */

/*==================[macros and definitions]=================================*/
/**
 * @def OUTPUT
 * @brief Define la dirección del GPIO como salida.
 */
#define OUTPUT 1  /*!< Dirección del GPIO configurada como salida */

/*==================[internal data definition]===============================*/

/**
 * @struct gpioConf_t
 * @brief Estructura que representa la configuración de un pin GPIO.
 * 
 * Esta estructura agrupa el número del pin GPIO y su dirección (entrada o salida).
 * 
 * @var gpioConf_t::pin
 * Número del pin GPIO que se configurará.
 * @var gpioConf_t::dir
 * Dirección del GPIO: '0' para entrada (IN) y '1' para salida (OUT).
 */
typedef struct
{
	gpio_t pin;			/*!< Número del pin GPIO */
	io_t dir;			/*!< Dirección del GPIO: '0' IN; '1' OUT */
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Configura y cambia el estado de los pines GPIO según el dígito BCD.
 *
 * Esta función toma un dígito BCD y un vector de configuraciones de GPIOs. Cada bit del dígito BCD
 * se usa para definir el estado del correspondiente pin GPIO: si el bit es 1, el GPIO se configura
 * en el estado alto; si el bit es 0, el GPIO se configura en el estado bajo.
 *
 * @param bcd_digito Dígito BCD que define el estado de los pines GPIO
 * @param gpio_conf Vector de configuraciones de GPIOs donde se almacenan los pines y sus direcciones.
 * 
 * @note El tamaño del vector `gpio_conf` debe ser al menos de 4 elementos, uno para cada bit del dígito BCD (b0 a b3).
 *
 * @return void
 */
void configurarGpios(uint8_t bcd_digito, gpioConf_t gpio_conf[4]) {
    // Asegurarse de que el vector tiene al menos 4 elementos y que no es nulo
    if (gpio_conf == NULL) {
        return;
    }

    // Iterar sobre los 4 bits del dígito BCD (de b0 a b3)
    for (uint8_t i = 0; i < 4; i++) {
        // Configurar la dirección del GPIO (0 -> IN, 1 -> OUT)
        GPIOInit(gpio_conf[i].pin, gpio_conf[i].dir);

        // Verificar el estado del bit correspondiente
        if (bcd_digito & (1 << i)) {
            // Si el bit está en 1, configurar el GPIO en alto
            GPIOOn(gpio_conf[i].pin);
        } else {
            // Si el bit está en 0, configurar el GPIO en bajo
            GPIOOff(gpio_conf[i].pin);
        }
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación.
 * 
 * En esta función se define un vector de configuraciones de GPIOs y se llama a la función
 * 'configurarGpios' para configurar el estado de los pines GPIO según un dígito BCD específico.
 * El dígito BCD se usa para definir los estados de los GPIOs, donde cada bit del dígito determina
 * si el GPIO correspondiente debe estar en alto o bajo.
 *
 * @return void
 */
void app_main(void) {
    /* Definir el vector de configuraciones de GPIOs */
    gpioConf_t gpio_conf[4] = {
        {GPIO_20, OUTPUT},  // Configuración del GPIO_20 como salida
        {GPIO_21, OUTPUT},  // Configuración del GPIO_21 como salida
        {GPIO_22, OUTPUT},  // Configuración del GPIO_22 como salida
        {GPIO_23, OUTPUT}   // Configuración del GPIO_23 como salida
    };

    // Definir el dígito BCD para configurar los pines GPIO
    uint8_t digitoBCD = 9; // Ejemplo de valor para el dígito BCD
    configurarGpios(digitoBCD, gpio_conf);
}

/*==================[end of file]==========================================*/