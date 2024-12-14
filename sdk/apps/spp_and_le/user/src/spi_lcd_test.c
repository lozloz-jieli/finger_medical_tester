#include "system/app_core.h"
#include "system/includes.h"

#include "app_config.h"
#include "app_action.h"

#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "user_cfg.h"
#include "vm.h"
#include "btcontroller_modules.h"
#include "bt_common.h"
#include "3th_profile_api.h"
#include "le_common.h"
#include "rcsp_bluetooth.h"
#include "JL_rcsp_api.h"
#include "custom_cfg.h"
#include "btstack/btstack_event.h"
#include "gatt_common/le_gatt_common.h"
#include "lcd.h"
#include "app_main.h"




#if 1//LE_DEBUG_PRINT_EN
#define log_info(x, ...)  printf("[SPI_LCD_TEST]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

u8 the_pic_buff[160*8*2]={0};
void WriteOneDot(unsigned int color);
void  DispOnepic( char * ord,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor,unsigned int Xpic,unsigned int Ypic)	 // ord:0~95
{
   u16 i,j,k;
   unsigned char  *p;
   unsigned char dat;
//   unsigned int index;
   OLED_Set_Draw_Area(Xstart+1,Xstart+Xpic,Ystart+26,Ystart+(Ypic+25));

//   p = a_pic;
   p = ord;


   for(i=0;i<Xpic/8;i++)
    {
        u16 b_l=0;
        for(k=0;k<Ypic;k++)
        {
           dat=*p++;
           for(j=0;j<8;j++)
            {
                 if((dat<<j)&0x80)
                 {
//                    WriteOneDot(TextColor);
                    the_pic_buff[b_l++]=(TextColor>>8);
                    the_pic_buff[b_l++]=(TextColor);
                 }
                 else
                 {
//                    WriteOneDot(BackColor);
                    the_pic_buff[b_l++]=(BackColor>>8);
                    the_pic_buff[b_l++]=(BackColor);
                 }
             }
        }

        lcd_spi_dma_send_wait(the_pic_buff, b_l);

     }

     //OLED_Set_Draw_Area(0,COL-1,0+24,ROW+24-1);
}





void spi_lcd_test_printf(void)
{
    log_info("%s",__func__);
}


