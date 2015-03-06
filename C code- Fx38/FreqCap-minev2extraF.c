// FreqF38x.c: Measure the frequency of a signal on pin T0.
//
// By:  Jesus Calvino-Fraga (c) 2008-2014
//
// The next line clears the "C51 command line options:" field when compiling with CrossIDE
//  ~C51~  

#include <C8051f38x.h>

#include <stdio.h>
#include <string.h>
#define SYSCLK      48000000L  // SYSCLK frequency in Hz
#define BAUDRATE      115200L  // Baud rate of UART in bps

#define SEG_A P1_6
#define SEG_B P1_7
#define SEG_C P2_0
#define DP    P2_1
#define SEG_D P2_2
#define SEG_E P2_3
#define SEG_G P2_4
#define SEG_F P2_5

#define CA1   P3_0
#define CA2   P2_7
#define CA3   P2_6

unsigned char overflow_count;

////////////////////////////////////////////////
//   		Multiplexer Code				////
////////////////////////////////////////////////

code unsigned char seven_seg[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
                                   0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E };
                                   
volatile unsigned char ISR_state=0;
volatile unsigned char disp3, disp2, disp1;
unsigned char dp_pos=0;

char _c51_external_startup (void)
{
	PCA0MD&=(~0x40) ;    // DISABLE WDT: clear Watchdog Enable bit
	VDM0CN=0x80; // enable VDD monitor
	RSTSRC=0x02|0x04; // Enable reset on missing clock detector and VDD

	// CLKSEL&=0b_1111_1000; // Not needed because CLKSEL==0 after reset
	#if (SYSCLK == 12000000L)
		//CLKSEL|=0b_0000_0000;  // SYSCLK derived from the Internal High-Frequency Oscillator / 4 
	#elif (SYSCLK == 24000000L)
		CLKSEL|=0b_0000_0010; // SYSCLK derived from the Internal High-Frequency Oscillator / 2.
	#elif (SYSCLK == 48000000L)
		CLKSEL|=0b_0000_0011; // SYSCLK derived from the Internal High-Frequency Oscillator / 1.
	#else
		#error SYSCLK must be either 12000000L, 24000000L, or 48000000L
	#endif
	OSCICN |= 0x03; // Configure internal oscillator for its maximum frequency
	
	// Configure the pins connected to the 7-segment display as outputs
	P1MDOUT|=0b_1100_0000; // SEG_A, SEG_B
	P2MDOUT|=0b_1111_1111; // SEG_C, DP, SEG_D, SEG_E, SEG_F, SEG_G, CA2, CA3
	P3MDOUT|=0b_0000_0001; // CA1
	XBR0=0x00;                     
	XBR1=0x40; // Enable crossbar and weak pull-ups
	
	return 0;
}

// Uses Timer3 to delay <us> micro-seconds. 
void Timer3us(unsigned char us)
{
	unsigned char i;               // usec counter
	
	// The input for Timer 3 is selected as SYSCLK by setting T3ML (bit 6) of CKCON:
	CKCON|=0b_0100_0000;
	
	TMR3RL = (-(SYSCLK)/1000000L); // Set Timer3 to overflow in 1us.
	TMR3 = TMR3RL;                 // Initialize Timer3 for first overflow
	
	TMR3CN = 0x04;                 // Sart Timer3 and clear overflow flag
	for (i = 0; i < us; i++)       // Count <us> overflows
	{
		while (!(TMR3CN & 0x80));  // Wait for overflow
		TMR3CN &= ~(0x80);         // Clear overflow indicator
	}
	TMR3CN = 0 ;                   // Stop Timer3 and clear overflow flag
}

void Timer2_ISR (void) interrupt 5
{
	unsigned char todisp;
	TF2H = 0; // Clear Timer2 interrupt flag
	
	CA3=1;
	CA2=1;
	CA1=1;

	switch(ISR_state)
	{
		case 0:
			todisp=disp3;
		break;
		case 2:
			todisp=disp2;
		break;
		case 4:
			todisp=disp1;
		break;
		default:
			todisp=0xff; // blank
		break;
	}

	ACC=todisp;
	SEG_A=ACC_0;
	SEG_B=ACC_1;
	SEG_C=ACC_2;
	SEG_D=ACC_3;
	SEG_E=ACC_4;
	SEG_F=ACC_5;
	SEG_G=ACC_6;
//	DP=ACC_7;

	switch(ISR_state)
	{
		case 0:
			CA3=0;
		break;
		case 2:
			CA2=0;
		break;
		case 4:
			CA1=0;
		break;
	}

	ISR_state++;
	if(ISR_state>5) ISR_state=0;
}

void Timer2_Init (void)
{
	TMR2CN=0x00;   // Stop Timer2; Clear TF2;
	CKCON&=~0xF0;  // Timer2 clocked based on TMR2CN_T2XCLK;
	TMR2RL=0xf800; // Initialize reload value
	TMR2=0xffff;   // Set to reload immediately
	ET2=1;         // Enable Timer2 interrupts
	TR2=1;         // Start Timer2
}

void waitms (unsigned int ms)
{
	unsigned int j;
	for(j=ms; j!=0; j--)
	{
		Timer3us(249);
		Timer3us(249);
		Timer3us(249);
		Timer3us(250);
	}
}

void Send_7Seg (unsigned int x)
{
	if ((x/100) == 0){
	disp3=0b_1111_1111;
	}
	else{	
	disp3=seven_seg[x/100];
	}
	
	if (((x/10)%10)==0){
	disp2=0b_1111_1111;
	}
	else{
	disp2=seven_seg[(x/10)%10];
	}
	disp1=seven_seg[x%10];
	
//	if ((++dp_pos)>2) dp_pos=0;
	
//	switch(dp_pos)
//	{
//		case 0:
//			
//			disp1&=0b_0111_1111; // Turn on the decimal point of the first digit
//			break;
//		case 1:
//			disp2&=0b_0111_1111; // Turn on the decimal point of the second digit
//			break;
//		case 2:
//			
//			disp3&=0b_0111_1111; // Turn on the decimal point of the third digit
//			break;
//	}
}

// End of Multiplexer Code

void PORT_Init (void)
{
	P0MDOUT |= 0x10; // Enable UART TX as push-pull output
	XBR0=0b_0000_0001; // Enable UART on P0.4(TX) and P0.5(RX)                    
	XBR1=0b_0101_0000; // Enable crossbar.  Enable T0 input.
	XBR2=0b_0000_0000;
}

void SYSCLK_Init (void)
{
	// CLKSEL&=0b_1111_1000; // Not needed because CLKSEL==0 after reset
#if (SYSCLK == 12000000L)
	//CLKSEL|=0b_0000_0000;  // SYSCLK derived from the Internal High-Frequency Oscillator / 4 
#elif (SYSCLK == 24000000L)
	CLKSEL|=0b_0000_0010; // SYSCLK derived from the Internal High-Frequency Oscillator / 2.
#elif (SYSCLK == 48000000L)
	CLKSEL|=0b_0000_0011; // SYSCLK derived from the Internal High-Frequency Oscillator / 1.
#else
	#error SYSCLK must be either 12000000L, 24000000L, or 48000000L
#endif
	OSCICN |= 0x03;   // Configure internal oscillator for its maximum frequency
	RSTSRC  = 0x04;   // Enable missing clock detector
}
 
void UART0_Init (void)
{
	SCON0 = 0x10;
   
#if (SYSCLK/BAUDRATE/2L/256L < 1)
	TH1 = 0x10000-((SYSCLK/BAUDRATE)/2L);
	CKCON &= ~0x0B;                  // T1M = 1; SCA1:0 = xx
	CKCON |=  0x08;
#elif (SYSCLK/BAUDRATE/2L/256L < 4)
	TH1 = 0x10000-(SYSCLK/BAUDRATE/2L/4L);
	CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 01                  
	CKCON |=  0x01;
#elif (SYSCLK/BAUDRATE/2L/256L < 12)
	TH1 = 0x10000-(SYSCLK/BAUDRATE/2L/12L);
	CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 00
#else
	TH1 = 0x10000-(SYSCLK/BAUDRATE/2/48);
	CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 10
	CKCON |=  0x02;
#endif
	TL1 = TH1;      // Init Timer1
	TMOD &= ~0xf0;  // TMOD: timer 1 in 8-bit autoreload
	TMOD |=  0x20;                       
	TR1 = 1; // START Timer1
	TI = 1;  // Indicate TX0 ready
}


void TIMER0_Init(void)
{
	TMOD&=0b_1111_0000; // Set the bits of Timer/Counter 0 to zero
	TMOD|=0b_0000_0101; // Timer/Counter 0 used as a 16-bit counter
	TR0=0; // Stop Timer/Counter 0
}

#define R1 1.64e3
#define R2 1.64e3
 
void main (void) 
{
	unsigned long F;
	float C;
	int input;
	
	PCA0MD &= ~0x40; // WDTE = 0 (clear watchdog timer enable)
	PORT_Init();     // Initialize Port I/O
	SYSCLK_Init ();  // Initialize Oscillator
	UART0_Init();    // Initialize UART0
	TIMER0_Init();

	printf("\x1b[2J"); // Clear screen using ANSI escape sequence.
	printf ("Frequency measurement using Timer/Counter 0.\n"
	        "File: %s\n"
	        "Compiled: %s, %s\n\n",
	        __FILE__, __DATE__, __TIME__);
	        
	printf("HELLO! I AM ALIVE!\n I live to measure capacitance :D \n");
	printf("Can I measure something for you? 1= Yes or 0= no\n");
	
	scanf("%d",&input);
	if (input == 0){
	printf("Too bad, I'm going to do it anyway");
	}
	else{
	printf("OMG I LOVE YOU. You've made me the happiest processor in the world");
	}
	waitms(1000);
	
	
	
	

	Timer2_Init();
	EA=1; // Enable interrupts
	
	
	while(1)
	{
		TL0=0;
		TH0=0;
		overflow_count=0;
		TF0=0;
		TR0=1; // Start Timer/Counter 0
		waitms(1000);
		TR0=0; // Stop Timer/Counter 0
		F=overflow_count*0x10000L+TH0*0x100L+TL0;
		
		C=(1.44e9)/((R1+2*R2)*F);
		printf("\rf=%luHz, C=%6.2fnf", F, C);
		printf("\x1b[0K"); // ANSI: Clear from cursor to end of line.
		Send_7Seg(C);
	}
}



	
	
	



 