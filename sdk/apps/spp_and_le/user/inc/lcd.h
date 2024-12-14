// #include "c8051F340.h"
// #include <main.h>

#ifndef __LCD_h__
#define __LCD_h__

#ifdef DEBUG
	#define DEBUG_X  50
	#define DEBUG_Y  15
	//extern unsigned int code debug_code[X][Y];
	extern unsigned int code debug_code[50][15];

	#define END 0xffff
#endif

#define ROW  160			//��ʾ���С�����      行     row                   X
#define COL  80             //列                      column                         Y

#define X_OFFSET		26
#define Y_OFFSET		1

// extern unsigned int code pic_eval[];
// extern unsigned char code ascii[];

//---------------------------------------------------------------------
// void SPI_WriteComm(unsigned int i);
// void SPI_WriteData(unsigned int i);
void SPI_WriteComm(unsigned char i);
void SPI_WriteData(unsigned char i);

void WriteDispData(unsigned char DataH,unsigned char DataL);
void SPI_WriteData_Da(unsigned char i);
void LCD_Init(void);
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend);
void DispBand(void);
void DispColor(unsigned int color);
void DispFrame(void);
void DispPicFromSD(unsigned char PicNum);

void DispScaleHor1(void);
void DispScaleVer(void);
void DispScaleVer_Red(void);
void DispScaleVer_Green(void);
void DispScaleVer_Blue(void);
void DispScaleVer_Gray(void);
void DispGrayHor16(void);
void DispGrayHor32(void);
void DispScaleHor2(void);
void DispSnow(void);
void DispBlock(void);

void WriteOneDot(unsigned int color);
unsigned char ToOrd(unsigned char ch);
void DispOneChar(unsigned char ord,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);	 // ord:0~95
void DispStr(unsigned char *str,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);
void DispInt(unsigned int i,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);

unsigned int ReadData(void);
void DispRegValue(unsigned int RegIndex,unsigned char ParNum);

void Debug(void);

void PutPixel(unsigned int x,unsigned int y,unsigned int color);
void DrawGird(unsigned int color);

void lcd_spi_init(void);
void Display_test1_demo(void);
void lcd_blockwrite(int xs, int xe, int ys, int ye);
void Display_row(unsigned int color);
void DispFrame_test(void);
void DrawPoint(int x, int y, int color);
void lcd_spi_dma_send_byte(char dat);


#endif
