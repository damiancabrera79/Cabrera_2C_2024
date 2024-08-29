/*! @mainpage Guía 1 - Ejercicio 4
 *
 * \section genDesc Descripción General
 *
 * Este ejercicio trata sobre la conversión de un número de 32 bits a formato BCD 
 * (Binary-Coded Decimal), dividiendo el número en sus dígitos decimales y almacenándolos
 * en un arreglo.
 *
 * @section changelog Changelog
 *
 * |   Fecha	  | Descripción                                    |
 * |:------------:|:-----------------------------------------------|
 * | 16/08/2024   | Creación del documento		                   |
 *
 * @author Damian Cabrera (cabreradamian@gmail.com)
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/**
 * @brief Convierte un número de 32 bits a formato BCD y lo almacena en un arreglo.
 * 
 * Esta función toma un número entero de 32 bits y lo convierte a formato BCD 
 * (Binary-Coded Decimal). Los dígitos resultantes se almacenan en el arreglo 
 * pasado como puntero. La función también se asegura de que el número de dígitos 
 * en la salida no exceda el tamaño del arreglo proporcionado.
 *
 * @param data Número entero de 32 bits que se desea convertir a BCD.
 * @param digits Cantidad de dígitos que debe contener el número convertido. 
 *               Define cuántos dígitos almacenará el número BCD.
 * @param bcd_number Puntero a un arreglo donde se almacenarán los dígitos BCD.
 *                   El arreglo debe tener espacio suficiente para almacenar 'digits' dígitos.
 * 
 * @return int8_t Retorna 0 si la conversión fue exitosa. Retorna -1 si ocurrió un error,
 *                como si el puntero es nulo o si el número tiene más dígitos que los permitidos 
 *                por 'digits'.
 *
 * @note Si el número 'data' tiene más dígitos que los permitidos por 'digits', 
 *       la función retornará un error.
 */

int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number) {

	// Verifica si el puntero del arreglo BCD es nulo
    if (bcd_number == NULL) {
        return -1;  // Error: puntero nulo
    }
    
    // Rellena el arreglo BCD de derecha a izquierda con los dígitos del número
    for (int8_t i = digits - 1; i >= 0; i--) {
        if (data > 0 || i == digits - 1) {  // Evita ceros a la izquierda
            bcd_number[i] = data % 10;      // Extrae el dígito menos significativo
            data /= 10;                     // Reduce el número, eliminando el dígito ya procesado
        } else {
            bcd_number[i] = 0;              // Rellena con ceros si ya no hay más dígitos
        }
    }
    // Si después de completar el bucle aún queda un valor en 'data', es un error
    if (data > 0) {
        return -1;  // Error: el número no cabe en el arreglo BCD
    }

    return 0;  // Éxito
}

/*==================[external functions definition]==========================*/


/**
 * @brief Función principal de la aplicación
 * 
 * En esta función se inicializan los valores, se llama a la función de conversión a BCD,
 * y luego se imprime el resultado o un mensaje de error.
 */

void app_main(void){
	
	/* Inicializaciones */

	uint32_t numero = 123;      // Número a convertir a BCD
	uint8_t digito = 3;         // Cantidad de dígitos esperados
	uint8_t arreglo[digito];    // Arreglo para almacenar los dígitos BCD
	    
    // Llamar a la función convertToBcdArray
    int8_t result = convertToBcdArray(numero, digito, arreglo);
    
    if (result == 0) {
        // Si la conversión fue exitosa, imprimir el arreglo
        printf("BCD: ");
        for (uint8_t i = 0; i < digito; i++) {
            printf("%d", arreglo[i]);
        }
        printf("\n");
    } else {
        // Si hubo un error, imprimir un mensaje de error
        printf("Error: el número no cabe en el arreglo BCD.\n");
    }
}
/*==================[end of file]==========================================*/