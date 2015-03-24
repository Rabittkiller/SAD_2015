#include <p24fxxxx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void initUART(int baud){

	U2BRG = baud;
	U2MODE = 0x8000; //8bit no parity 1 stop bit
	U2STA = 0;
	U2STAbits.UTXEN = 1; //Enable Transmit
	//IEC0bits.U2TXIE = 1; //Enable Transmit Interrupt
	//IEC0bits.U2RXIE = 1; //Enable Receive Interrupt
}
void writeBadjoras(char *frase){

	int i,size;
	size = strlen(frase);
	for(i=0;i<size;i++){
		while(U2STAbits.UTXBF==1);
		U2TXREG = *frase;
		frase++;
	}		
}

int main(void)
{
	//inicializacoes

	TRISDbits.TRISD6 = 1;
	TRISDbits.TRISD7 = 1;
	TRISAbits.TRISA0 = 0;
	TRISAbits.TRISA1 = 0;

	int a,i = 0, Channel_Temp = 5, Sensor_Temp=0;
	char print_string[100];
	//main	

	initADC();
    initUART(12);
	while ( 1 ){
		Sensor_Temp = readADC(Channel_Temp); 
		sprintf(print_string,"temp: %d\n",Sensor_Temp);
		write(print_string);
		if(Sensor_Temp > 100){
			PORTAbits.RA0=1;
		}else{
			PORTAbits.RA0=0;
		}
		if(Sensor_Temp > 500){
			PORTAbits.RA1=1;
		}else{
			PORTAbits.RA1=0;
		}
		
		if ( !PORTDbits.RD6){
			PORTAbits.RA0 = 1;

			for( i = 0 ; i < 20000 ; i++){};
			PORTAbits.RA0 = 0;

			for( i = 0 ; i < 20000 ; i++){};
		}
		
		if ( !PORTDbits.RD7){

			PORTAbits.RA1 = 1;
			for( i = 0 ; i < 20000 ; i++){};

			PORTAbits.RA1 = 0;
			for( i = 0 ; i < 20000 ; i++){};
		}
	}
}
