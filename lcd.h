#ifndef _LCD_16_2_H_
#define _LCD_16_2_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this is a datastructure that can be associated with one line of a 16x2 lcd. In the  main program this structure should be used as a buffer for the lcd
*and should be declared as lcdline buff[2]={0};
*/
struct line
{
	unsigned char ch[16];	//buffer for the 16 character space for one line of a 16x2 lcd
	unsigned char i;	//stores the offset of the line that is written
};

typedef struct line lcdline;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*this function contains the lcd initialisation code and must be called before any operation can be performed on the lcd.
*/
void lcdInit();	//lcd initialisation

/*this is a higher level lcd function that writes into the lcd which is very similar to lcd_data() but additionally takes care of wrapping up the display
*as well as scrolling the diisplay at display boundaries
*/
void lcdWriteChar(lcdline* display,char ch);

/*this function writes a string to the lcd. Its important to consider the lcd character space. To big a string will be scrolled, so the programmer should
only use this for short strings to display
*/
void lcdWriteString(lcdline* display,char *string);

/*lcd clear screen*/
void lcdClearScreen(lcdline buff[]);
#endif