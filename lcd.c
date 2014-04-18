/*Author: Supratim Das
*Institute: Calcutta Institute of Engineering and Management
*version: 20110823
*description: 16x2 lcd driver header
*/

/*This driver api is a part of Remote GSM based power management and intrusion detection system. Final year B.tech project by the
*author
*/

/*this is header file contains the lcd API. this header contains the definition of the data and control port to which lcd is connected. This header should
*only be included after the inclusion of <avr/io.h> and <util/delay.h> in the main program. This header also contains the definition of various lcd cmds
*and functions necessary for updating the lcd display. Please go through the comment section of each functions for more information about that function
*/
//check the inclusion of necessary headers for proper operation
#ifndef _LCD_C
#define _LCD_C


#ifndef F_CPU
#define F_CPU 12000000l
#endif

#ifndef USE_WATCHDOG
#define USE_WATCHDOG 0
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "lcd.h"


//port and pin definition of the lcd connections

#define RS	PB2	//register select
#define	RW	PB1	//read/write
#define	EN	PB0	//enable
#define LCD_DATA_PORT	PORTC
#define LCD_DATA_DDR	DDRC
#define	LCD_CONTROL_PORT	PORTB
#define	LCD_CONTROL_DDR	DDRB
#define	LCD_ENABLE	LCD_CONTROL_PORT|=(1<<EN)	
#define	LCD_DISABLE	LCD_CONTROL_PORT&=~(1<<EN)

//LCD command codes

#define	CLRSCR	0x01
#define	HOME	0x02
#define	DEC_CUR	0x04
#define	INC_CUR	0x06
#define	SHIFT_DISP_RIGHT	0x05
#define	SHIFT_DISP_LEFT		0x07
#define	DISP_OFF_CUR_OFF	0x08
#define	DISP_OFF_CUR_ON		0x0a
#define	DISP_ON_CUR_OFF		0x0c
#define	DISP_ON_CUR_BLINK	0x0e
#define	SHIFT_CUR_LEFT		0x10
#define	SHIFT_CUR_RIGHT		0x14
#define	SHIFT_ENT_DISP_LEFT	0x18
#define	SHIFT_ENT_DISP_RIGHT	0x1c
#define	MOV_CUR_1_LINE_BEG	0x80
#define	MOV_CUR_2_LINE_BEG	0xc0
#define	LINEX2	0x38

#define MIRRORED_OUTPUT 1

#define MIRROR_BYTE(x)	(x<<1&0x10)|(x>>1&0x08)|(x<<3&0x20)|(x>>3&0x04)|(x<<5&0x40)|(x>>5&0x02)|(x<<7&0x80)|(x>>7&0x01)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this function takes as input the specific lcd command and executes them. for the various lcd commands please refer to the LCD command code section at
*the beginning of this document
*/
void lcdCmd(char cmd)
{
	LCD_CONTROL_PORT&=~(1<<RS);	//select command register
	#if MIRRORED_OUTPUT
	cmd=MIRROR_BYTE(cmd);
	#endif
	LCD_DATA_PORT=cmd;	//output command code on lcd port
	LCD_ENABLE;	//enable lcd
	_delay_us(80);	//delay to latch inputs
	LCD_DISABLE;	//disable lcd
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this function takes as input a character data and writes the character to the lcd display
*/
void lcdData(char data)
{
	LCD_CONTROL_PORT|=(1<<RS);	//select data register
	#if MIRRORED_OUTPUT
	data=MIRROR_BYTE(data);
	#endif
	LCD_DATA_PORT=data;
	LCD_ENABLE;
	_delay_us(80);	//delay to latch inputs
	LCD_DISABLE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this function contains the lcd initialisation code and must be called before any operation can be performed on the lcd.
*/
void lcdInit()	//lcd initialisation
{
#if USE_WATCHDOG
	wdt_reset();
	wdt_disable();
#endif
	LCD_DATA_DDR=0xff;	//initialise lcd output port
	LCD_DATA_PORT=0x00;
	LCD_CONTROL_DDR=0x0f;	//initialise lcd control port
	LCD_CONTROL_PORT=0x00;
	LCD_CONTROL_PORT&=~(1<<RW);
	_delay_ms(100);
	lcdCmd(LINEX2); // 8 data lines
	lcdCmd(INC_CUR); // cursor setting
	lcdCmd(DISP_ON_CUR_BLINK); // display ON
	lcdCmd(CLRSCR); // clear LCD memory
	_delay_ms(500);
	lcdData(' ');	//write a blank
	lcdCmd(MOV_CUR_1_LINE_BEG);	//move back to first line first character
	_delay_ms(1000);
#if USE_WATCHDOG
	wdt_enable(WDTO_2S);
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this is a higher level lcd function that writes into the lcd which is very similar to lcd_data() but additionally takes care of wrapping up the display
*as well as scrolling the diisplay at display boundaries
*/
void lcdWriteChar(lcdline* display,char ch)
{
	unsigned char line=0,i;
	if(display[0].i>=16)	//check if line 1 is full
	{
		line=2;	//if full set line 2 as working line
	}
	else
	{
		line=1;	//else remain in line 1
	}
	if(ch=='\r')
	{
		//display[line-1].i=0;
		return;
	}
	if(ch=='\n')
	{
		while(display[line-1].i<16)
		{
			display[line-1].ch[display[line-1].i]=' ';
			display[line-1].i++;
		}
		return;
	}
	switch(line)
	{
		case 1:	//if the current working line is 1
			if(display[0].i==0)	//check if first character of the line
			{
				lcdCmd(MOV_CUR_1_LINE_BEG);	//move cursor to the beginning of second line
			}
			display[0].ch[display[0].i]=ch;	//assing character data to appropriate location in the buffer
			lcdData(display[0].ch[display[0].i]);	//display the data
			display[0].i++;	//update the line offset
			break;
		case 2:	//if the current working line is 2
			if(display[1].i==0)	//check if first character of the line
			{
				lcdCmd(MOV_CUR_2_LINE_BEG);	//move cursor to the beginning of second line
			}
			if(display[1].i>=16)	//if line 2 is full
			{
				lcdCmd(MOV_CUR_1_LINE_BEG);	//move cursor to the beginning of line 2
				lcdCmd(DISP_OFF_CUR_OFF);
				for(i=0;i<16;i++)	//copy line 2 buffer to line 1 buffer and scroll line 2 to line 1
				{
					display[0].ch[i]=display[1].ch[i];
					lcdData(display[0].ch[i]);
				}
				lcdCmd(MOV_CUR_2_LINE_BEG);	//the following lines of code clears the line 2
				for(i=0;i<16;i++)
				{
					lcdData(' ');
				}
				display[1].i=0;
				lcdCmd(DISP_ON_CUR_BLINK);
				lcdCmd(MOV_CUR_2_LINE_BEG);
			}
			display[1].ch[display[1].i]=ch;
			lcdData(display[1].ch[display[1].i]);
			display[1].i++;
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this function writes a string to the lcd. Its important to consider the lcd character space. To big a string will be scrolled, so the programmer should
only use this for short strings to display
*/
void lcdWriteString(lcdline* display,char *string)
{
	unsigned char len=0;
	while(string[len]!='\0')
	{
		lcdWriteChar(display,string[len]);
		len++;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*lcd clear screen*/
void lcdClearScreen(lcdline buff[])
{
	lcdCmd(CLRSCR);
	buff[0].i=0;
	buff[1].i=0;
#if USE_WATCHDOG
	wdt_reset();
#endif
	_delay_ms(100);
#if USE_WATCHDOG
	wdt_reset();
#endif

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
