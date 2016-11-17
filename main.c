/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2015
***********************************************************************
	 	   			 		  			 		  		
 Team ID: 19

 Project Name: Musical Piano

 Team Members:

   
   - Software Leader: Sid Parida     

   - Interface Leader: Mark Brooks 

   - Packaging/Documentation Leader: Kylie Broton   


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to build a musical piano with recording playback and simultaneous two key playback capabilities.


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1. Free play mode including note display

 2. Recording of song

 3. Playback of song

 4. Pitch and Tempo Adjust while playback

 5. Simultaneous free play of up to 2 notes

***********************************************************************

  Date code started: 11/21

  Update history (add an entry every time a significant change is made):

  Date: 11/27  Name: Free Play Mode   Update: 2

  Date: 11/28  Name: Recording and Playback Mode  Update: 3

  Date: 12/2  Name: Displays and LEDs   Update: 4


***********************************************************************
*/



#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All funtions after main should be initialized here */
char inchar(void);
void outchar(char);
void fdisp(void);
void shiftout(char);
void lcdwait(void);
void send_byte(char);
void send_i(char);
void chgline(char);
void print_c(char);
void pmsglcd(char[]);
void getButtons(void);
void displayButtons(void);
void recordNotes(int, int);
int getSpeaker0Note();
int getSpeaker1Note();
void assignNote(int, char *, int); 
void fancyDisplay();
void assignLEDS();

#define BN1 162
#define C0 153
#define D0b 144
#define D0 136
#define E0b 128
#define E0 121
#define F0 114
#define G0b 108
#define G0 102
#define A0b 96
#define A0 91
#define B0b 86
#define B0 81
#define C1 76
#define D1b 72
#define D1 68
#define E1b 64
#define E1 60
#define F1 57
#define G1b 54
#define G1 51
#define A1b 48
#define A1 45
#define B1b 43
#define B1 40
#define C2 38
#define D2b 36
#define D2 34
#define E2b 32
#define E2 30
#define F2 28
#define G2b 27
#define G2 25
#define A2b 24
#define A2 23
#define B2b 21
#define B2 20
#define C3 19
#define REST 1
#define SONG_LENGTH 53
#define WN 16
#define HWN 12
#define HN 8
#define QHN 6
#define QN 4
#define EQN 3
#define EN 2
#define SN 1
#define TEMPO 5000
#define SONG_LENGTH 53
#define MAX_SONG_LENGTH 200
#define MAX_SONG_NUMBER 2
#define RECORD_PERIOD 1250 


/*  Variable declarations */ 	   

// Song Spaces
int SONG1_B1[MAX_SONG_LENGTH];
int SONG1_B2[MAX_SONG_LENGTH];
int SONG2_B1[MAX_SONG_LENGTH];
int SONG2_B2[MAX_SONG_LENGTH];
			 		  			 		       

// PushButtons
char modepb = 0;
char prevmodepb = 0;
char startpb = 0;
char prevstartpb = 0;
char optionpb = 0;
char prevoptionpb = 0;

// Different system variables 
int pitch = 5;            
int mode = 0; // 0 - Free Play Mode, 1 - Record Mode, 2 - Playback Mode, 3 - TeachMode
int tempo = 0;
int temp_tempo = 0;
int start = 0;
int option = 0; 


// Music related 
int freq_mod0 = 0;
int freq_mod1 = 0;
int count0 = 0;
int count1 = 0;
int bg1Notes[5] = {A1b, A1, B1b, B1, C2};
int bg2Notes[8] = {C1, D1b, D1, E1b, E1, F1, G1b, G1};

// For use with Modes 
int recordButtons = 0;
int changeNoteFlag = 0;
int currentSongIndex = 0;
int tempoChangeFlag = 0;

// Button Groups
int bg1 = 0;
int bg2 = 0;

int i = 0;
int tim_flag = 0;
long tim_count = 0;
int rec_count = 0;
int playCount = 0;



/* LCD COMMUNICATION BIT MASKS */
#define RS 0x04		// RS pin mask (PTT[2])
#define RW 0x08		// R/W pin mask (PTT[3])
#define LCDCLK 0x10	// LCD EN/CLK pin mask (PTT[4])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 0x80	// LCD line 1 cursor position
#define LINE2 0xC0	// LCD line 2 cursor position
#define SCALE 100
/*	 	   		
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL


/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

         
/* Add additional port pin initializations here */
//PTT = 0;
DDRT = 0xFF;
DDRM = 0xFF;

/* Initialize the SPI to 6 Mbs */
SPICR1= 0x50;
SPICR2 = 0x00;
SPIBR = 0x10;
	 	   			 		  			 		  		
/* Initialize digital I/O port pins */
DDRAD = 0;
ATDDIEN = 252;

/* Initialize the LCD
     - pull LCDCLK high (idle)
     - pull R/W' low (write state)
     - turn on LCD (LCDON instruction)
     - enable two-line mode (TWOLINE instruction)
     - clear LCD (LCDCLR instruction)
     - wait for 2ms so that the LCD can wake up     
*/
PTT_PTT4 = 1;
PTT_PTT3 = 0;

send_i(LCDON);
send_i(TWOLINE);
send_i(LCDCLR);
lcdwait(); 

/* Initialize RTI for 2.048 ms interrupt rate */	
RTICTL = 0x1F;
CRGINT = CRGINT | 0x80;

/* Initialize TIM Ch 7 (TC7) for periodic interrupts every 1.000 ms
     - enable timer subsystem
     - set channel 7 for output compare
     - set appropriate pre-scale factor and enable counter reset after OC7
     - set up channel 7 to generate 1 ms interrupt rate
     - initially disable TIM Ch 7 interrupts      
*/
TSCR1 = 0x80;
TSCR2 = 0x0C;
TIOS = 0xFF;
TIE = 0x80;
TC7 = 150; 

//Shift Registre Controls
// CEBar = 5, StrobeBar = 6, CLK = 7
PTT_PTT5 = 1;
PTT_PTT6 = 1;
PTT_PTT7 = 1;


// LEDs
PTM_PTM0 = 0; // Green LED
PTM_PTM1 = 0; // Yellow LED
PTM_PTM3 = 0; // Red LED 


//initialize ATD (8 bit, unsigned, nominal sample time, seq length = 2)

ATDCTL2 = 0x80; //power up ATD
ATDCTL3 = 0x10; //set conversion sequence length to TWO
ATDCTL4 = 0x85; //select resolution and sample time

//initialize PWM Ch 0 (left aligned, positive polarity, max 8-bit period)

MODRR = 0x03; //PT0 used as PWM Ch 0 output
PWME	= 0x03; //enable PWM Ch 0
PWMPOL = 0x01; //set active high polarity
PWMCTL = 0x00; //no concatenate (8-bit)
PWMCAE	 = 0x00; //left-aligned output mode
PWMPER0 = 0xFF; //set maximum 8-bit period
PWMPER1 = 0xFF; //set maximum 8-bit period
PWMDTY0 = 0x00; //initially clear DUTY register
PWMDTY1 = 0x00; //initially clear DUTY register
PWMCLK = 0x00; //select Clock A for Ch 0
PWMPRCLK =  0x00;  //set Clock A = 12 MHz (prescaler = 2) rate	  


// Song Location Initializations
for(i = 0; i < MAX_SONG_LENGTH; i++) {
  SONG1_B1[i] = (i%2) ? 0: 0;
}
for(i = 0; i < MAX_SONG_LENGTH; i++) {
  SONG1_B2[i] = (i%2) ? 0: 0;
}

for(i = 0; i < MAX_SONG_LENGTH; i++) {
  SONG2_B1[i] = (i%2) ? A1: G1;
}
for(i = 0; i < MAX_SONG_LENGTH; i++) {
  SONG2_B2[i] = (i%2) ? C1: C1;
}	      

}

/*	 		  			 		  		
***********************************************************************
 Main
***********************************************************************
*/

void main(void) {
  	DisableInterrupts;
	initializations(); 		  			 		  		
	EnableInterrupts;

send_i(LCDCLR);
lcdwait();
 


for(;;) {


  /* write your code here */
   if(modepb)
	{
		modepb = 0;
		mode++;
		mode = mode % 3;

		startpb = 0;
		start = 0;
		optionpb = 0;
		option = 0;
		currentSongIndex = 0;
		tempoChangeFlag = 0;

	}
	
	if(startpb)
	{
		startpb = 0;
		start++;
		start = start % 2;
	}
	
	if(optionpb)
	{
		optionpb = 0;
		if(start != 1) 
		{
	    option++;
		  option = option % MAX_SONG_NUMBER;
		  currentSongIndex = 0;
		}
	}


   
   getButtons();
   //displayButtons();
   fancyDisplay();
   assignLEDS();
   
   
      	//ATD Conversion, Values available in   ATDDR0H and ATDDR1H
   ATDCTL5 = 0x10;
   while(ATDSTAT0 != 0x80) {}
   pitch = ATDDR0H / 26;
   tempo = ATDDR1H/26;
   if(tempo != temp_tempo) 
   {
        tempoChangeFlag = 1;
        temp_tempo = tempo;
   }
  
  } /* loop forever */
   
}  /* do not leave main */



/*
***********************************************************************
 RTI interrupt service routine: RTI_ISR

  Initialized for 2.048 ms interrupt rate

  Samples state of pushbuttons (PAD7 = left, PAD6 = right)

  If change in state from "high" to "low" detected, set pushbutton flag
     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
     Recall that pushbuttons are momentary contact closures to ground
***********************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flag
  	CRGFLG = CRGFLG | 0x80; 

	// Update Buttons
	if(PORTAD0_PTAD3 == 0 & prevmodepb == 1)   modepb = 1;
	prevmodepb = PORTAD0_PTAD3;

	if(PORTAD0_PTAD4 == 0 & prevstartpb == 1)   startpb = 1;
	prevstartpb = PORTAD0_PTAD4;

	if(PORTAD0_PTAD5 == 0 & prevoptionpb == 1)   optionpb = 1;
	prevoptionpb = PORTAD0_PTAD5;



}


int getSpeaker0Note()
{
	if(option == 0) return SONG1_B1[currentSongIndex];
	else if(option == 1) return SONG2_B1[currentSongIndex];


	else return 0;
}

int getSpeaker1Note()
{
	if(option == 0) return SONG1_B2[currentSongIndex];
	else if(option == 1) return SONG2_B2[currentSongIndex];


	else return 0;
}



/*
***********************************************************************
  TIM interrupt service routine
  used to initiate ATD samples (on Ch 0 and Ch 1)	 		  			 		  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
  int bUnU = 2;
  int i;
  int bg1_temp = bg1;
  int bg2_temp = bg2;
  int once_flag = 0;
    
  // clear TIM CH 7 interrupt flag 
 	TFLG1 = TFLG1 | 0x80;
    
  tim_count++;
  rec_count++;
  if(tempoChangeFlag == 1) 
  {
      tim_count = 0;
      tempoChangeFlag = 0;
  }
  if(tim_count == TEMPO/tempo) 
  {
      tim_count = 0;
      changeNoteFlag = 1;
      
    
  }
  if(rec_count == RECORD_PERIOD) 
  {
      rec_count = 0;
      recordButtons = 1;
      
    
  }
 

  if(mode != 2 || start == 0) {
    
  freq_mod0 = REST;
  freq_mod1 = REST;
  }
  
  //Mode Work/////////////////
  if(mode == 0 || mode == 1) 
  {
    
  while(bUnU > 0) 
  {   
      if(once_flag) break;      
      // Check BG2
      for(i = 0; i < 8; i++) 
      {
        if(bg2_temp % 2 == 1)
        {
            if(bUnU <=0 ) break;
            else if(bUnU == 2) 
            {
              freq_mod0 = bg2Notes[i];
              bUnU--;
            }
            else if(bUnU == 1) 
            {
              freq_mod1 = bg2Notes[i];
              bUnU--;
              break;
            }
            
        } 
        bg2_temp = (bg2_temp >> 1);
      }
      
      // Break if two notes reached
      if(bUnU <= 0) break;
      
      // Check BG1
      for(i = 0; i < 5; i++) 
      {
        if(bg1_temp % 2 == 1)
        {
            if(bUnU <=0 ) break;
            else if(bUnU == 2) 
            {
              freq_mod0 = bg1Notes[i];
              bUnU--;
            }
            else if(bUnU == 1) 
            {
              freq_mod1 = bg1Notes[i];
              bUnU--;
              break;
            }
            
        } 
        bg1_temp = (bg1_temp >> 1);
      }
  
      once_flag = 1;
   }
   
   //RecordMode
   if(mode == 1) 
   {
   
   
      if(start)
			{
				if(recordButtons)
				{
					recordButtons = 0;
					if(currentSongIndex < MAX_SONG_LENGTH)
					{
						recordNotes(freq_mod0, freq_mod1);
						currentSongIndex++;
						if(currentSongIndex == MAX_SONG_LENGTH)
						{
							start = 0;
							currentSongIndex = 0;
							
						}
					}
					else
					{
						start = 0;
					}
				}
			}
   
   
   }
   
   //////////////
 
   
   
  }
  // End of mode 0 0r 1
  
  
  
  // Playback Mode
	else if(mode == 2)
	{
		if(start == 1)
		{
			if(changeNoteFlag == 1)
			{
				changeNoteFlag = 0;
				currentSongIndex++;
				i = getSpeaker0Note();
				if(currentSongIndex == MAX_SONG_LENGTH || i == 0) 
				{
					start = 0;
					currentSongIndex = 0;
					freq_mod0 = REST;
					freq_mod1 = REST;
				}
				else
				{
					freq_mod0 = getSpeaker0Note();
					freq_mod1 = getSpeaker1Note();
				}

			}
		}
	}
 
 
   // Actual note play
   count0 += pitch;
   count0 = count0 % (freq_mod0);
   if ((count0 < freq_mod0/2) && (freq_mod0 != REST)) PWMDTY0 = 2; 
   else  PWMDTY0 = -2; 
 
   
   count1 += pitch;
   count1 = count1 % (freq_mod1);
   if ((count1 < freq_mod1/2) && (freq_mod1 != REST)) PWMDTY1 = 2; 
   else  PWMDTY1 = -2; 
   
   

  
}


void recordNotes(int freq_mod0, int freq_mod1)
{
	if(option == 0)
	{
		SONG1_B1[currentSongIndex] = freq_mod0;
		SONG1_B2[currentSongIndex] = freq_mod1;
	}
	
	else if(option == 1)
	{
		SONG2_B1[currentSongIndex] = freq_mod0;
		SONG2_B2[currentSongIndex] = freq_mod1;
	}

}

void fancyDisplay() 
{
  //Free Play   P:_
  //___  ___    T:_
  
  //Record      P:_
  //___ ___     S:_
  
  //Recording   P:_
  //___ ___ L:_ S:_
  
  //Play    T:_ P:_
  //___ ___     S:_

  //Playin  T:_ P:_
  //___ ___ L:_ S:_
  
  char fPL1[] = "Free Play   P:_\0";
  char fPL2[] = "___  ___    T:_\0";
  
  char rSL1[] = "Record      P:_\0";
  char rSL2[] = "___ ___     S:_\0";

  char pSL1[] = "Play    T:_ P:_\0";
  char pSL2[] = "___ ___     S:_\0";
  
  if(mode == 0) 
  {
      fPL1[14] = (char)(pitch) + (int)'0';
      fPL2[14] = (char)(tempo) + (int)'0';
      assignNote(0, fPL2, freq_mod0);
      assignNote(4, fPL2, freq_mod1);
      chgline(LINE1);
      pmsglcd(fPL1);
      chgline(LINE2);
      pmsglcd(fPL2);
  } 
  else if(mode == 1) 
  {
      rSL1[14] = (char)(pitch) + (int)'0';
      rSL2[14] = (char)(option) + (int)'0';
      
      assignNote(0, rSL2, freq_mod0);
      assignNote(4, rSL2, freq_mod1);


      if(start == 1) 
      {
          rSL1[6] = 'i'; rSL1[7] = 'n'; rSL1[8] = 'g';
          rSL2[8] = 'L'; rSL2[9] = ':'; rSL2[10] = (char)(currentSongIndex / 21) + (int)'0';
          
      }
      chgline(LINE1);
      pmsglcd(rSL1);
      chgline(LINE2);
      pmsglcd(rSL2);
  }
    else if(mode == 2) 
  {
      pSL1[10] = (char)(tempo) + (int)'0';
      pSL1[14] = (char)(pitch) + (int)'0';
      pSL2[14] = (char)(option) + (int)'0';
      
      
      assignNote(0, pSL2, freq_mod0);
      assignNote(4, pSL2, freq_mod1);


      if(start == 1) 
      {
          pSL1[4] = 'i'; pSL1[5] = 'n';  pSL1[6] = ' ';
          pSL2[8] = 'L'; pSL2[9] = ':'; pSL2[10]= (char)(currentSongIndex / 21) + (int)'0';
          
      }
      chgline(LINE1);
      pmsglcd(pSL1);
      chgline(LINE2);
      pmsglcd(pSL2);
  }

  

}
void assignLEDS() 
{  
    PTM_PTM0 = 0;
    PTM_PTM1 = 0;
    PTM_PTM3 = 0;

    if(freq_mod0 != REST || freq_mod1 != REST) 
    {
        PTM_PTM0 = 1;
    }
    if(start == 1) 
    {
        if(mode == 1)   PTM_PTM1 = 1;
        else if(mode == 2) PTM_PTM3 = 1;
    }
}
    
  

void assignNote(int index, char *str, int freq) 
{
  if(freq == C1) 
  {
     str[index] = 'C';
     str[index+1] = '1';
     str[index+2] = ' ';                                                      
  } 
  else if(freq == D1b) 
  {
     str[index] = 'D';
     str[index+1] = '1';
     str[index+2] = 'b';
  }
  else if(freq == D1) 
  {
     str[index] = 'D';
     str[index+1] = '1';
     str[index+2] = ' ';
  }
  else if(freq == E1b) 
  {
     str[index] = 'E';
     str[index+1] = '1';
     str[index+2] = 'b';
  }
  else if(freq == E1) 
  {
     str[index] = 'E';
     str[index+1] = '1';
     str[index+2] = ' ';
  }
  else if(freq == F1) 
  {
     str[index] = 'F';
     str[index+1] = '1';
     str[index+2] = ' ';
  }
  else if(freq == G1b) 
  {
     str[index] = 'G';
     str[index+1] = '1';
     str[index+2] = 'b';
  }
    else if(freq == G1) 
  {
     str[index] = 'G';
     str[index+1] = '1';
     str[index+2] = ' ';
  }
  else if(freq == A1b) 
  {
     str[index] = 'A';
     str[index+1] = '1';
     str[index+2] = 'b';
  }
  else if(freq == A1) 
  {
     str[index] = 'A';
     str[index+1] = '1';
     str[index+2] = ' ';
  }
  else if(freq == B1b) 
  {
     str[index] = 'B';
     str[index+1] = '1';
     str[index+2] = 'b';
  }
  else if(freq == B1) 
  {
     str[index] = 'B';
     str[index+1] = '1';
     str[index+2] = ' ';
  }
  else if(freq == C2) 
  {
     str[index] = 'C';
     str[index+1] = '2';
     str[index+2] = ' ';
  }
  else if(freq == REST) 
  {
     str[index] = ' ';
     str[index+1] = ' ';
     str[index+2] = ' ';
  }
}

 
void displayButtons() 
{

	char line1_string[] = "________1PTMSO12\0";
	char line2_string[] = "________2_______\0";
	
	int bit;
	int bg1_temp = bg1;
	int bg2_temp = bg2;
	
	
	
	
	
	line2_string[9] = (char)(pitch) + (int)'0';
	line2_string[10] = (char)(tempo) + (int)'0';
	line2_string[11] = (char)(mode) + (int)'0';
	line2_string[12] = (char)(start) + (int)'0';
	line2_string[13] = (char)(option) + (int)'0';
	line2_string[14] = (char)(freq_mod0 % 10) + (int)'0';
	line2_string[15] = (char)(freq_mod1 % 10) + (int)'0';

  

  
  

  for(i = 0; i < 8; i++) 
  {
    bit = bg1_temp % 2;
    bg1_temp = bg1_temp >> 1;
    //line1_string[i] = '1';
    line1_string[i] = (char)(bit) + (int)'0';
    bit = bg2_temp % 2;
    bg2_temp = bg2_temp >> 1;
    line2_string[i] = (char)(bit) + (int)'0';
  }
	
	chgline(LINE1);
	pmsglcd(line1_string);
	chgline(LINE2);
 	pmsglcd(line2_string);


}

void getButtons() 
{

        
    bg1 = 0;
    bg2 = 0;

    // CEBar = 5, StrobeBar = 6, CLK = 7
    
    // Strobe 
    PTT_PTT6 = 0;
    PTT_PTT6 = 1;
    
    // Clock High
    //PTT_PTT7 = 1;
    // Enable Chip
    PTT_PTT5 = 0;

    
    
    // Group 1
    // Read bits
    for(i = 0; i < 8; i++) 
    { 
      // Read bits
      bg1 = bg1 * 2;
      bg1 += PORTAD0_PTAD2;
      //bg1 += 1;
      //Clock
      PTT_PTT7 = 0;
      PTT_PTT7 = 1;
    }
    
    // Group 2
    // Read bits
    for(i = 0; i < 8; i++) 
    { 
      // Read bits
      bg2 = bg2 * 2;
      bg2 += PORTAD0_PTAD2;
      //bg2 += 1;
      //Clock
      PTT_PTT7 = 0;
      PTT_PTT7 = 1;
    }
    
    
    // Disable chip
    PTT_PTT5 = 1;


 
}


void itos(int ind, char *disp, long num) 
{
  int i, digit, pow10 = 10000;
  for(i = 4; i >=0; i--) 
  {
      
      digit = num / pow10;
      num = num % pow10;
      pow10 = pow10 /10 ;
      disp[ind + (4 - i)] = (char)digit + (int)'0';
  }
  
}


/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout(char x)

{
 
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out 
  	int j = 0;

	while(!SPISR_SPTEF){}
	SPIDR = x;
	for(j = 0; j < 30; j++)
	{
		asm
		{
			nop;
			nop;
			nop;
			nop;
			nop;
		}
	} 
}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{
  int j;
	for(j = 0; j < 4800; j++)
	{
		asm
		{
			nop;
			nop;
			nop;
			nop;
			nop;
		}
	} 
}

/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/

void send_byte(char x)
{
     // shift out character
     // pulse LCD clock line low->high->low
     // wait 2 ms for LCD to process data
  shiftout(x);
	PTT_PTT4 = 1;
	PTT_PTT4 = 0;
	PTT_PTT4 = 1;
	lcdwait();
}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/

void send_i(char x)
{
        // set the register select line low (instruction data)
        // send byte
  PTT_PTT2 = 0;
	send_byte(x);
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline(char x)
{
  send_i(CURMOV);
	send_i(x);
	lcdwait();
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
  PTT_PTT2 = 1;
	send_byte(x);
}

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
  int i = 0;
	while(str[i] != '\0')
	{
		print_c(str[i]);
		i++;
	}
}


/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 (for debugging only)
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/

char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}