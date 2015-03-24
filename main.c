#include <p24fxxxx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uartcom.h"

#define POT5 //10k potentiometer connected to AN5 input
#define AINPUTS 0xffef

// Configuration Bits
#ifdef __PIC24FJ64GA004__ //Defined by MPLAB when using 24FJ64GA004 device
_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx1 & IOL1WAY_ON) 
_CONFIG2( FCKSM_CSDCMD & OSCIOFNC_OFF & POSCMOD_HS & FNOSC_PRI & I2C1SEL_SEC)
#else
_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx2) 
_CONFIG2( FCKSM_CSDCMD & OSCIOFNC_OFF & POSCMOD_HS & FNOSC_PRI)
#endif

void initADC(){
	AD1CON1 = 0x0000; //A/D Control Registers(1,2&3).Control the overall operation of the A/D module. In this case, SAMP bit = 0 ends sampling and starts converting
	AD1CON2 = 0; //AVss and AVdd are used as Vref+/-
	AD1CON3 = 0X0002; //Manual Sample, Tad = 2 Tcy
	AD1PCFG = 0; //A/D Port Configuration Register.Configures I/O pins as analog inputs or digital I/Os. In this case, all input piuns are analog.
	AD1CSSL = 0; //A/D Input Scan Select Register.Selects the channels to be included for sequential scanning.In this case, No inputs are scanned.
	AD1CON1bits.ADON = 1; //.ADON->A/D converter module is operating.bit 15->=1
}
int readADC(int Channel){
	
	int i;
	AD1CHS = Channel;
	AD1CON1bits.SAMP = 1; //Start Sampling
	for(i = 0; i<1000; i++);
	AD1CON1bits.SAMP = 0; //Start Converting
	while(!AD1CON1bits.DONE); //Waits for the convertion to end
	return ADC1BUF0; 
}

void initialUART(int baud){

	U2BRG = baud;
	U2MODE = 0x8000; //8bit no parity 1 stop bit
	U2STA = 0;
	U2STAbits.UTXEN = 1; //Enable Transmit
	//IEC0bits.U2TXIE = 1; //Enable Transmit Interrupt
	//IEC0bits.U2RXIE = 1; //Enable Receive Interrupt
}
void writeBadjoras(char *frase){ //podia ser utilizado uma funçao de uartcom.h mas esta foi feita antes

	int i,size;
	size = strlen(frase);
	for(i=0;i<size;i++){
		while(U2STAbits.UTXBF==1);
		U2TXREG = *frase;
		frase++;
	}		
}
void alarme(){
	
	int pot,channel=5;
	long int w;
	char Buff[10],Buff1[6];					//Buff - Password, Buff1 - para por utilizador
	putstringUART("Sprinklers ON !!!\r\n");
	while(1){
		pot = readADC(channel);
		if(pot > 10 && pot <500){
			PORTAbits.RA0 = 1; 						
			for( w = 0 ; w < 70000 ; w++){};   // para a freq dos LED
			PORTAbits.RA0 = 0; 
			for( w = 0 ; w < 70000 ; w++){};	// para a freq dos LED
		}
		if(pot > 500){
			PORTAbits.RA0 = 1; 
			for( w = 0 ; w < 10000 ; w++){};	// para a freq dos LED
			PORTAbits.RA0 = 0; 
			for( w = 0 ; w < 10000 ; w++){};	// para a freq dos LED
		}
		if(!PORTDbits.RD6 && !PORTDbits.RD7){		// se os dois botoes forem primidos
			putstringUART("PASSWORD: \r\n");
			getstringUART(Buff, 10);
			if(!(strcmp("admin",Buff))){
				putstringUART("\r\nAutentification accepted----> Sprinklers OFF\r\n");
				break;
			}
			else{
				putstringUART("Autentification Failed");
				break;
			}
		}	
	
	}
}
int main(void)
{
	//inicializacoes

	TRISDbits.TRISD6 = 1;
	TRISDbits.TRISD7 = 1;
	TRISAbits.TRISA0 = 0;
	TRISAbits.TRISA1 = 0;

	int a,i = 0, Channel_Poten = 5, potenciometro=0, temperatura=40;
	long int w;
	char print_string[100],code;	

	initADC();
    initialUART(12);	//Fcy= 8 MHz  Formula: ((Fcy)/(16*19200))-1
	IFS1bits.U2TXIF=0;	// interrupt flag status bit
	while ( 1 ){
		if(IFS1bits.U2TXIF==0) 
			code=getcharUART();
		IFS1bits.U2TXIF=0;
		switch(code){
			case 'p': 
					potenciometro = readADC(Channel_Poten); 
					sprintf(print_string,"Caudal: %d..\r\n",potenciometro);
					writeBadjoras(print_string);
					code=0;
					break;
			case 't':
					//temperatura=readADC(Channel_Temp);   //aqui é o codigo da placa a add
					if(temperatura>39)
						alarme();
					break;
			case 'x':
					putstringUART("    Mr. Joao Ralo        Mr. Andre Coelho        Mr.Flavio Silva\r\n");

		default: break; 
		}code=0;
		
	}
}
