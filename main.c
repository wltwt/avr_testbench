/*
 * File:   main.c
 * Author: Olav Telneset
 *
 * 
 * Large portions referenced from:
 * Alexandru Niculae, Getting Started with USART, Microchip Technology Inc.
 *
 * Created on January 28, 2025, 2:17 PM 
 */


#define F_CPU 4000000UL
#define NUM_PORTS 2
#define NUM_USARTS 3
#define MAX_COMMAND_LEN 64

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>





static inline uint16_t USART_BAUD_RATE(uint32_t baud)
{
    return (uint16_t)((F_CPU * 64.0) / (16.0 * baud) + 0.5);
}


static inline void configure_gpio(volatile uint8_t* dir_reg)
{
    *dir_reg |= PIN0_bm;
    *dir_reg &= ~PIN1_bm;
}


static inline void configure_usart(USART_t *usart, uint16_t baud_val)
{
    usart->BAUD = baud_val;
    usart->CTRLB |= (USART_TXEN_bm | USART_RXEN_bm);
}


static inline void usart_sendChar(USART_t *usart, char c)
{
    while (!(usart->STATUS & USART_DREIF_bm)) 
    {
        
    }
    usart->TXDATAL = c;
}


static inline void usart_sendString(USART_t *usart, char *str)
{
    while (*str)
    {
        usart_sendChar(usart, *str++);
    }
}

static USART_t *stdout_usart;

static int USART_printChar(char c, FILE *stream)
{
    usart_sendChar(stdout_usart, c);
    return 0;
}

static FILE USART_stream = FDEV_SETUP_STREAM(USART_printChar, NULL, _FDEV_SETUP_WRITE);


uint8_t USART_read(USART_t *usart)  
{ 
    while (!(usart->STATUS & USART_RXCIF_bm))
    {
        ;
    }
    return usart->RXDATAL;
}


int main(void)
{
    PORT_t *PORTS[] = {&PORTB, &PORTC};
    USART_t *USARTS[] = {&USART3, &USART1, &USART0};
    
    PORTA.DIR |= PIN4_bm;
    PORTA.DIR &= ~PIN5_bm;

    for (uint8_t i = 0; i < NUM_PORTS; i++) {
        configure_gpio(&(PORTS[i]->DIR));
    }
    
    for (uint8_t i = 0; i < NUM_USARTS; i++) {
        configure_usart(USARTS[i], USART_BAUD_RATE(9600));
    }
    
    stdout_usart = &USART3;
    stdout = &USART_stream;
    
    PORTMUX.USARTROUTEA =
            (0x1 << PORTMUX_USART0_gp)
            | (0x0 << PORTMUX_USART1_gp)
            | (0x0 << PORTMUX_USART3_gp);
    
    char command[MAX_COMMAND_LEN];
    char c;
    
    uint8_t index = 0;
    

    while (1)
    {
        stdout_usart = &USART0;
        stdout = &USART_stream;
    
        printf("Test\n");

        while (1) {

            c = USART_read(&USART1);

            if(c != '\n' && c != '\r') {
                command[index++] = c;
                if(index > MAX_COMMAND_LEN)
                {
                    index = 0;
                }
            }

            if(c == '\n')
            {
                command[index] = '\0';
                index = 0;

                stdout_usart = &USART3;
                stdout = &USART_stream;

                printf("%s\n", command);
                
                break;
            }
   
        }
        _delay_ms(3000);
        
    }
}