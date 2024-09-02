/*! @mainpage Guía 1 - Ejercicio 6
 *
 * @section genDesc Descripción General
 *
 * Este ejercicio trata sobre la configuración de pines GPIO basada en un dígito BCD 
 * (Decimal Codificado en Binario). Cada bit del dígito BCD controla el estado de un pin GPIO 
 * específico, configurándolo en alto (1) o bajo (0) según el valor del bit correspondiente.
 * 
 * @section hardConn Hardware Connection
 *
 * |	  ESP32c6	|	  LCD       |
 * |:--------------:| :-------------|	
 * |    GPIO_19		|     SEL_1     |
 * |    GPIO_18		|	  SEL_2     |
 * |    GPIO_9	    |	  SEL_3     |
 * |    GPIO_20		|	   D1		|
 * |    GPIO_21		|	   D2		|
 * | 	GPIO_22		|	   D3		|
 * | 	GPIO_23		|	   D4		|
 * | 	+5V			|	  +5V		|
 * | 	GND			|	  GND		|
 *
 * @section changelog Changelog
 *
 * |   Fecha	  | Descripción                                   |
 * |:------------:|:----------------------------------------------|
 * | 30/08/2024   | Creación del documento		                  |
 * | 02/09/2024   | Funcionando		                              |
 *
 * @author Damian Cabrera (cabreradamian@gmail.com)
 */

/*==================[inclusions]=============================================*/
/**
 * @file main.c
 * @brief Archivo principal que incluye las bibliotecas necesarias.
 *
 * Este archivo contiene las funciones para la configuración y control de los GPIOs 
 * basados en un dígito BCD. Utiliza la librería gpio_mcu.
 */

#include <stdio.h>
#include <stdint.h>
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
 * @fn void inicializarGPIO(gpioConfig_t *gpioVector, int numPins)
 * @brief Inicializa los pines GPIO configurando su dirección (entrada o salida).
 * 
 * Esta función recorre el vector de configuración de los GPIO y llama a 'GPIOInit'
 * para cada pin especificado, estableciendo su dirección (INPUT o OUTPUT) según corresponda.
 * 
 * @param gpioVector Puntero al vector que contiene las configuraciones de los pines GPIO.
 *                   Cada elemento del vector es una estructura 'gpioConfig_t' que contiene:
 *                   - 'pin': El número de pin GPIO.
 *                   - 'dir': La dirección del pin (INPUT o OUTPUT).
 * @param numPins Número de pines GPIO (elementos) en el vector que deben ser inicializados.
 * 
 * @return void
 */

void inicializarGPIO(gpioConf_t *gpioVector, int numPins){
	for(int i = 0; i < numPins ; i++){
		GPIOInit(gpioVector[i].pin, gpioVector[i].dir);
	}
}

////////////////////

/**
 * @fn void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcdNumber)
 * @brief Convierte un número entero en su representación BCD (Decimal Codificado en Binario) y lo almacena en un array.
 * 
 * Esta función toma un número entero y lo convierte en su formato BCD, extrayendo cada dígito y almacenándolo 
 * en un array. Rellena con ceros los dígitos que no estén presentes en caso de que el número tenga menos dígitos 
 * que el tamaño especificado.
 * 
 * @param data Número entero a convertir en BCD.
 * @param digits Número de dígitos que debe tener el array BCD.
 * @param bcdNumber Puntero al array donde se almacenarán los dígitos BCD.
 * 
 * @return void
 */

void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcdNumber){
    for (int8_t i = digits - 1; i >= 0; i--) {
        if (data > 0 || i == digits - 1) {  // Evita ceros a la izquierda, pero siempre procesa al menos un dígito
            bcdNumber[i] = data % 10;       // Extrae el dígito menos significativo
            data /= 10;                     // Elimina el dígito ya procesado, reduciendo el número
        } else {
            bcdNumber[i] = 0;               // Rellena con ceros si ya no hay más dígitos
        }
    }
}

/////////////////

/**
 * @fn void configurarBcdAGpio(uint8_t digitBCD, gpioConfig_t *vectorGPIO)
 * @brief Configura el estado de los pines GPIO en función del dígito BCD proporcionado.
 * 
 * Esta función toma un dígito BCD (Decimal Codificado en Binario) y lo convierte en una señal binaria,
 * configurando el estado de 4 pines GPIO correspondientes. Los bits del dígito BCD determinan si cada 
 * pin GPIO debe estar en estado alto o bajo.
 * 
 * @param digitBCD Dato en formato BCD (4 bits) que representa un número decimal.
 * @param vectorGPIO Puntero al vector que contiene la configuración de los pines GPIO. Cada elemento
 *                   de este vector debe tener el número de pin a controlar.
 * 
 * @return void
 */

void configurarBcdAGpio(uint8_t digitBCD, gpioConf_t *vectorGPIO){
	
    for(int i = 0; i < 4; i++){
        // Verifica si el bit 'i' del número BCD es 0
        if((digitBCD & (1 << i)) == 0){     
            GPIOOff(vectorGPIO[i].pin);     // Si el bit es 0, apaga el GPIO correspondiente
        }
        else{
            GPIOOn(vectorGPIO[i].pin);      // Si el bit es 1, enciende el GPIO correspondiente
        }
    }
}
//////////////

/**
 * @fn void mostrarValor(uint32_t data, uint8_t digits, gpioConfig_t *vectorGPIO, gpioConfig_t *vectorLCD)
 * @brief Muestra un valor numérico en un display o similar, usando pines GPIO y un vector de pines LCD.
 * 
 * Esta función convierte un número entero en formato BCD (Decimal Codificado en Binario), luego configura los pines 
 * GPIO correspondientes para mostrar cada dígito en el display. Finalmente, utiliza los pines del LCD para activar 
 * cada dígito individualmente.
 * 
 * @param data El número entero que se desea mostrar.
 * @param digits El número de dígitos que debe mostrarse (tamaño del array `vectorBCD`).
 * @param vectorGPIO Puntero al vector de pines GPIO, utilizado para configurar el estado de los pines en función 
 *                   del valor BCD de cada dígito.
 * @param vectorLCD Puntero al vector de pines LCD, utilizado para controlar el encendido y apagado de los dígitos 
 *                  en el display.
 * 
 * @return void
 */

void mostrarValor(uint32_t data, uint8_t digits, gpioConf_t *vectorGPIO, gpioConf_t *vectorLCD)
{
    uint8_t vectorBCD[digits];   // Array que almacenará la representación BCD del número `data`
    
    // Convierte el valor entero a un array de dígitos BCD
    convertToBcdArray(data, digits, vectorBCD);

    // Itera sobre cada dígito BCD
    for(int i = 0; i < digits; i++){
        
        // Configura los pines GPIO según el valor BCD del dígito actual
        configurarBcdAGpio(vectorBCD[i], vectorGPIO);
        
        // Enciende el pin LCD correspondiente para mostrar el dígito actual
        GPIOOn(vectorLCD[i].pin);
        
        // Apaga el pin LCD para pasar al siguiente dígito
        GPIOOff(vectorLCD[i].pin);
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

    uint32_t dato = 921;
	uint8_t cantidadDigitos = 3;

    /* Definir el vector de configuraciones de GPIOs */
    gpioConf_t GPIO_conf[4] = {
        {GPIO_20, OUTPUT},  // Configuración del GPIO_20 como salida
        {GPIO_21, OUTPUT},  // Configuración del GPIO_21 como salida
        {GPIO_22, OUTPUT},  // Configuración del GPIO_22 como salida
        {GPIO_23, OUTPUT}   // Configuración del GPIO_23 como salida
    };

    inicializarGPIO(GPIO_conf, 4);

    gpioConf_t LCD_conf[3] = {
        {GPIO_19, OUTPUT},  // Configuración del GPIO_19 como salida
        {GPIO_18, OUTPUT},  // Configuración del GPIO_18 como salida
        {GPIO_9, OUTPUT},  // Configuración del GPIO_9 como salida
    };

    inicializarGPIO(LCD_conf, 3);

    mostrarValor(dato, cantidadDigitos, GPIO_conf, LCD_conf);
}

/*==================[end of file]==========================================*/