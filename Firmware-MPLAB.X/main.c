/*
 * File:   main.c
 * Author: Samuel Marco e Kevin Shogo
 *
 * Created on 13 de Junho de 2024, 08:45
 * 
 * Microcontroladador: PIC18F4550
*/

#include <xc.h>
#include <pic18f4550.h>
#include "nxlcd.h"

#pragma config FOSC = HS             // Fosc = 20MHz; Tcy = 200ns
#pragma config CPUDIV = OSC1_PLL2    // OSC/1 com PLL off
#pragma config WDT = OFF             // Watchdog desativado
#pragma config LVP = OFF             // Desabilita grava��o em baixa
#pragma config DEBUG = ON            // Habilita debug
#pragma config MCLRE = ON            // Habilita MCLR

#define _XTAL_FREQ 20000000          // Frequ�ncia de oscila��o do cristal  
#define _QUANT_AD 10                 // Taxa de amostragem por segundo 
#define _MAX_BITS 1023               // Valor m�ximo da convers�o AD
#define _MAX_TEMP 65                 // Valor m�ximo da temperatura ambiente

unsigned char tempMin = 0;           // Valor inicial da temperatura
unsigned char tempDesejada = 0;      // Temperatura ambiente

// Vari�veis para convers�o anal�gica-digital 
unsigned long valorAD, leituraAD, resultadoAD;

// Visualiza��o dos dados no display LCD
unsigned long milhar, centena, dezena, unidade, aux, dezenaDes, unidadeDes; 

void setTemperaturaAmbiente(void) {
    valorAD = 0;
    leituraAD = 0;

    // C�lculo da m�dia da convers�o AD
    for(int index = 0; index < _QUANT_AD; index++) {
        ADCON0bits.GO_DONE = 1;
        while(ADCON0bits.GO_DONE);
            
        // Convers�o para milivolts 
        leituraAD = 256 * ADRESH + ADRESL;
        valorAD += leituraAD;
    }
        
    valorAD /= 10;
    resultadoAD = (5 * 1000 * valorAD) / _MAX_BITS;
    tempMin = resultadoAD/10;
    tempDesejada = tempMin;
    
    valorAD = 0;
    leituraAD = 0;
    resultadoAD = 0;
}

void interrupt low_priority HighPriorityISR(void) {
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;       // Limpa a flag do Timer0
        
        // Atualiza contagem do Timer0
        TMR0H = 0x67;                // Ajusta contagem para 1 segundo
        TMR0L = 0x6A;                // Adiciona CI para contagem de 1 seg

        // Envia temperatura atual
        WriteCmdXLCD(0xC8);
        milhar = resultadoAD / 1000;
        aux = resultadoAD % 1000;
        centena = aux / 100;
        aux = aux % 100;
        dezena = aux / 10;
        unidade = aux % 10;
         
        while (!TXSTAbits.TRMT);
        TXREG = 0x41;                 // (A = Temperatura Atual)
        
        while (!TXSTAbits.TRMT);
        TXREG = 0x30 + centena;
        putcXLCD(0x30 + centena);
        
        while (!TXSTAbits.TRMT);
        TXREG = 0x30 + dezena;
        putcXLCD(0x30 + dezena);
        
        while (!TXSTAbits.TRMT);
        TXREG = 0x2C;
        putcXLCD(0x2C);
        
        while (!TXSTAbits.TRMT);
        TXREG = 0x30 + unidade;
        putcXLCD(0x30 + unidade);
        
        while (!TXSTAbits.TRMT);
        TXREG = 0x43;
        putcXLCD(0x43);
        
        // Envia o separador
        while (!TXSTAbits.TRMT);
        TXREG = 0x3B;
        
        // Envia temperatura desejada
        WriteCmdXLCD(0x8B);
        dezenaDes = tempDesejada / 10;
        unidadeDes = tempDesejada % 10;
        
        while (!TXSTAbits.TRMT);
        TXREG = 0x44;                 // (D = Temperatura Desejada) 
        
        while (!TXSTAbits.TRMT);
        TXREG = 0x30 + dezenaDes;
        putcXLCD(0x30 + dezenaDes);
            
        while (!TXSTAbits.TRMT);
        TXREG = 0x30 + unidadeDes;
        putcXLCD(0x30 + unidadeDes);
        
        while (!TXSTAbits.TRMT);
        TXREG = 0x43;
        putcXLCD(0x43);
        
        while (!TXSTAbits.TRMT);
        TXREG = '\r';                  // Envia carriage return
        
        while (!TXSTAbits.TRMT);
        TXREG = '\n';                  // Envia line feed
    }
}

void interrupt high_priority LowPriorityISR(void) {
    if (INTCON3bits.INT1IF && (tempDesejada < _MAX_TEMP)) {
        __delay_ms(150);
        tempDesejada++;

        // Temperatura desejada
        WriteCmdXLCD(0x8B);
        dezenaDes = tempDesejada / 10;
        unidadeDes = tempDesejada % 10;
        putcXLCD(0x30 + dezenaDes);
        putcXLCD(0x30 + unidadeDes);

        INTCON3bits.INT1IF = 0;
    }
    
    else if (INTCON3bits.INT2IF && (tempDesejada > tempMin)) {
        __delay_ms(150);
        tempDesejada = tempDesejada - 1;

        // Temperatura desejada
        WriteCmdXLCD(0x8B);
        dezenaDes = tempDesejada / 10;
        unidadeDes = tempDesejada % 10;
        putcXLCD(0x30 + dezenaDes);
        putcXLCD(0x30 + unidadeDes);

        INTCON3bits.INT2IF = 0;
    }
}

void main(void) {
    // Configura��o dos registradores e I/O
    TRISAbits.TRISA0 = 1;            // AN0 � I/O de entrada
    TRISB = 0x06;                    // 0b00000110
    TRISCbits.RC1 = 0;               // RC1 com n�vel l�gico baixo 
    TRISCbits.RC2 = 0;               // RC2 com n�vel l�gico baixo
    TRISCbits.RC6 = 0;               // RC6 � I/O de sa�da
    TRISCbits.RC7 = 1;               // RC7 � I/O de entrada
    
    INTCON2bits.RBPU = 0;            // Habilitar os resistores de pull-up interno
 
    // Configura��o da comunica��o Serial
    TXSTAbits.BRGH = 0;              // Baixa velocidade de comunica��o
    TXSTAbits.SYNC = 0;              // Comunica��o ass�ncrona
    TXSTAbits.TXEN = 1;              // Habilita transmiss�o
    RCSTAbits.SPEN = 1;              // Habilita porta serial
    BAUDCONbits.BRG16 = 1;           // BRG possui 16 bits

    SPBRGH = 0;
    SPBRG = 64;                      // BR=9600, n=129 (cabe em 8 bits)

    // Configura��o do Timer0
    TMR0H = 0x67;                    // Contagem inicial parte de 0x676A
    TMR0L = 0x6A;                    // Para estourar 16 bits a cada 1 seg.
    T0CON = 0x96;                    // Configura��o TIMER0: 0b10010110

    // Interru��o do Timer0
    INTCONbits.TMR0IF = 0;           // Limpa flag do Timer0
    INTCON2bits.TMR0IP = 0;          // Timer0 configurado como base tempo Serial
    INTCONbits.TMR0IE = 1;           // Habilita interrup��o do Timer0

    // Configura��o da interrup��o de perif�ricos
    INTCON3bits.INT2IF = 0;
    INTCON2bits.INTEDG2 = 0;
    INTCON3bits.INT2IE = 1;
    INTCON3bits.INT2IP = 1;
    
    INTCON3bits.INT1IF = 0;
    INTCON2bits.INTEDG1 = 0;
    INTCON3bits.INT1IE = 1; 
    INTCON3bits.INT1IP = 1;

    // Chave geral das interrup��es
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    // Configura��o do conversor AD 10 bits
    ADCON1 = 0x0E;                   // 0b00001110
    ADCON0 = 0x01;                   // 0b00000001
    ADCON2 = 0x95;                   // 0b10010101
    __delay_us(10);

    // Configura��o do display LCD    
    OpenXLCD(FOUR_BIT & LINES_5X7);
    WriteCmdXLCD(0x01);              // Limpeza dos caracteres e retorno do cursor (1.52ms) 
    __delay_ms(2);
    WriteCmdXLCD(0x0C);              // Desabilita o controle do cursor
    
    WriteCmdXLCD(0x80);              // Escrita na posi��o 0, 0 (37us)
    putsXLCD("Desejada = ");

    WriteCmdXLCD(0xC0);              // Escrita na posi��o 1, 0 (37us)
    putsXLCD("Atual = ");
    
    setTemperaturaAmbiente();        // Aquisi��o da temperatura ambiente

    while(1){
        valorAD = 0;
        leituraAD = 0;

        // C�lculo da m�dia da convers�o AD
        for(int index = 0; index < _QUANT_AD; index++) {
            ADCON0bits.GO_DONE = 1;
            while(ADCON0bits.GO_DONE);
            
            // Convers�o para milivolts 
            leituraAD = 256 * ADRESH + ADRESL;
            valorAD += leituraAD;
        }
        
        valorAD /= 10;
        resultadoAD = (5 * 1000 * valorAD) / _MAX_BITS;

        // Controle ON/OFF dos atuadores
        if (((resultadoAD/10) + 1) <= tempDesejada) {
            PORTCbits.RC1 = 1;       // Ativar o aquecedor
            PORTCbits.RC2 = 0;       // Desativar o cooler
        }
        if (((resultadoAD/10) + 1) > tempDesejada) {
            PORTCbits.RC1 = 0;       // Desativar o aquecedor
            PORTCbits.RC2 = 1;       // Ativa o cooler
        }
    }
}
