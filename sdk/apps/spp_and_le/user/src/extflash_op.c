#include "typedef.h"
#include "system/includes.h"
#include "generic/gpio.h"
#include "asm/clock.h"
#include "tone_player.h"
#include "norflash.h"
#include "extflash_op.h"
#include "app_main.h"

#undef _DEBUG_H_
#define LOG_TAG_CONST       EXFLASH
#define LOG_TAG             "[EXFLASH]"
#include "debug.h"
#define LOG_v(t)  log_tag_const_v_ ## t
#define LOG_i(t)  log_tag_const_i_ ## t
#define LOG_d(t)  log_tag_const_d_ ## t
#define LOG_w(t)  log_tag_const_w_ ## t
#define LOG_e(t)  log_tag_const_e_ ## t
#define LOG_c(t)  log_tag_const_c_ ## t
#define LOG_tag(tag, n) n(tag)
const char LOG_tag(LOG_TAG_CONST,LOG_v) AT(.LOG_TAG_CONST) = 0;
const char LOG_tag(LOG_TAG_CONST,LOG_i) AT(.LOG_TAG_CONST) = 1;
const char LOG_tag(LOG_TAG_CONST,LOG_d) AT(.LOG_TAG_CONST) = 1; //log_debug
const char LOG_tag(LOG_TAG_CONST,LOG_w) AT(.LOG_TAG_CONST) = 1;
const char LOG_tag(LOG_TAG_CONST,LOG_e) AT(.LOG_TAG_CONST) = 1;
const char LOG_tag(LOG_TAG_CONST,LOG_c) AT(.LOG_TAG_CONST) = 1;

#define NOR_FLASH_ROOT_PATH	     "storage/res_nor"
#define NOR_FLASH_RES_ROOT_PATH	 NOR_FLASH_ROOT_PATH"/C/"
#define RES_ROOT_PATH		     NOR_FLASH_RES_ROOT_PATH"res/"

#define DBUG_PRINTF_LEN         512

enum {
    EXTFLASH_TONE_NUM_0 = 0,
    EXTFLASH_TONE_NUM_1,
    EXTFLASH_TONE_NUM_2,
    EXTFLASH_TONE_NUM_3,
};

const char *tone_table[] = {
    [EXTFLASH_TONE_NUM_0] 			= RES_ROOT_PATH"0.*",
    [EXTFLASH_TONE_NUM_1] 			= RES_ROOT_PATH"1.*",
    [EXTFLASH_TONE_NUM_2] 			= RES_ROOT_PATH"2.*",
    [EXTFLASH_TONE_NUM_3] 			= RES_ROOT_PATH"3.*",
};

char *dev_name = "res_nor";
static void *dev_ptr = NULL;
struct imount *fmnt;

extern void mcu_send_history_app(u8 *buffer,u16 len);


//测试函数
static const u8 res[] = { //测试data.bin
0x26, 0xC8, 0x64, 0xE9, 0x20, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x00,
0x64, 0x61, 0x74, 0x61, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x77, 0xD4, 0x33, 0x23, 0xA0, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x02, 0xFF, 0x00, 0x00,
0x69, 0x6E, 0x64, 0x65, 0x78, 0x2E, 0x69, 0x64, 0x78, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x30, 0xE6, 0x87, 0x7C, 0xE0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xFF, 0x00, 0x00,
0x61, 0x2E, 0x74, 0x78, 0x74, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x3D, 0x62, 0xAC, 0x21, 0xF0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0xFF, 0x00, 0x00,
0x62, 0x62, 0x2E, 0x74, 0x78, 0x74, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x38, 0x6C, 0x87, 0xC0, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0xFF, 0x01, 0x00,
0x63, 0x63, 0x63, 0x2E, 0x74, 0x78, 0x74, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x54, 0x49, 0x44, 0x58, 0xCB, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00,
0x09, 0xC9, 0x0A, 0x01, 0x61, 0x2E, 0x74, 0x78, 0x74, 0x00, 0x10, 0x76, 0x0B, 0x02, 0x62, 0x62,
0x2E, 0x74, 0x78, 0x74, 0x00, 0x3B, 0x8F, 0x0C, 0x03, 0x63, 0x63, 0x63, 0x2E, 0x74, 0x78, 0x74,
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x62, 0x62, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x63, 0x63, 0x63, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

//初始化帧头和命令
HISTORY_DATA history_data={
    .head = 0x5A,
    .file = 0x00,                  //初始化默认为0
};

HISTORY_DATA test_history_data;      //测试的全局变量

HISTORY_DATA last_history_data;     //读取历史数据用来存储当前读取的历史数据
HISTORY_DATA cur_history_data;      //读取历史数据用来存储上一次的历史数据
LOZ_EXFLASH_VAR loz_exflash_var;

char read_buffer[18*16] = {0};


static u8 buf[4 * 1024];
static void exflash_test_sys_loop(void *priv)
{
    static u32 cnt = 0;
    cnt++;
    y_printf("%s[cnt:%d]", __func__, cnt);
    int ret = 0;
    u32 len = 0;
    static u32 offset = 0;
    memset(buf, 0x00, sizeof(buf));
    if(dev_ptr){
#if 0
    offset = 0;
    len = sizeof(res);
    ret = dev_ioctl(dev_ptr, IOCTL_ERASE_SECTOR, offset);
    log_info("%s[IOCTL_ERASE_SECTOR -> offset:%d ret:%d]", __func__, offset, ret);

    ret = dev_bulk_write(dev_ptr, res, offset, len);
    log_info("%s[dev_bulk_write -> offset:%d len:%d ret:%d]", __func__, offset, len, ret);

    ret = dev_bulk_read(dev_ptr, buf, offset, len);
    log_info("%s[dev_bulk_read -> offset:%d len:%d ret:%d]", __func__, offset, len, ret);
    log_debug_hexdump(buf, ret);
#endif

#if 0
        struct imount *fmnt = mount(dev_name, NOR_FLASH_ROOT_PATH, "nor_sdfile", 3, NULL);
        log_info("%s[mount(%s):0x%x]", __func__, NOR_FLASH_ROOT_PATH, fmnt);
        if(!fmnt){
            r_printf("%s mount(%s) fail", __func__, NOR_FLASH_ROOT_PATH);
            return;
        }else{
            r_printf("mount suceed");
        }
#endif
#if 0
#define FILE_A                   RES_ROOT_PATH"a.txt"
#define FILE_C                   RES_ROOT_PATH"ccc.txt"
        static FILE *fp = NULL;
        static char *fname = NULL;
        fname = FILE_A;
        fp = fopen(fname, "r");
        log_info("[name:%s -> fp:%d]", fname, fp);
        if(fp){
            fseek(fp, 0, SEEK_SET);
            len = sizeof(buf);
            ret = fread(fp, buf, len);
            log_debug_hexdump(buf, ret);
            fclose(fp);
        }else{
            r_printf("open a error");
        }
        fname = FILE_C;
        fp = fopen(fname, "r");
        log_info("[name:%s -> fp:%d]", fname, fp);
        if(fp){
            fseek(fp, 0, SEEK_SET);
            len = sizeof(buf);
            ret = fread(fp, buf, len);
            log_debug_hexdump(buf, ret);
            fclose(fp);
        }else{
            r_printf("open ccc error");
        }
        ret = unmount(NOR_FLASH_ROOT_PATH);
        log_info("%s[umount(%s):0x%x]", __func__, NOR_FLASH_ROOT_PATH, ret);
#endif

#if 0
        log_debug_hexdump(buf, DBUG_PRINTF_LEN);
        //read
        offset = 0x100000 - 4096;                               //1024*1024byte = 0x100000             1M-4096
        len = 512;
        ret = dev_bulk_read(dev_ptr, buf, offset, len);
        log_info("%s[dev_bulk_read -> offset:%d len:%d ret:%d]", __func__, offset, len, ret);
        log_debug_hexdump(buf, DBUG_PRINTF_LEN);
        //erase
        ret = dev_ioctl(dev_ptr, IOCTL_ERASE_SECTOR, offset);
        log_info("%s[IOCTL_ERASE_SECTOR -> offset:%d ret:%d]", __func__, offset, ret);
        //write
        cnt = buf[0] + 1;
        for(u32 i = 0;i < sizeof(buf);i++){
            buf[i] = i;
        }buf[0] = cnt;
        ret = dev_bulk_write(dev_ptr, buf, offset, len);
        log_info("%s[dev_bulk_write -> offset:%d len:%d ret:%d]", __func__, offset, len, ret);
        //不要调用dev_close()
#endif

#if 1


#endif

    }
}


u8 collect_30s_buffer[DETECT_30S_OK_LEN];
HISTORY_DATA collect_30s_buffer_data[30];
u8 collect_1s_buffer[DETECT_1S_OK_LEN];
u32 history_index;                                                              //一直刻录1s一次的数据记录
u32 temp_history_index;                                                          //临时刻录1s一次的数据记录
u32 exflash_offset = FLASH_SECTOR;                                                           //flash地址开始从4096开始    //这个的全部历史的数据偏移量每次都要从vm读出来
u32 exflash_offset_index = FLASH_SECTOR;                                                           //flash地址开始从4096开始，     //这个的历史数据的指针索引偏移量也每次都要从vm读出来
u32 last_file_offset_index = FLASH_SECTOR;                                                           //flash地址开始从4096开始，     //这个的历史数据文件地址的要从vm读出来

u8 test_buffer[8192];
/*
**********************************************************************
函数功能：清理30s之前的buffer
函数形参：
函数返回值：None
备注：
日期：2024年12月05日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void clear_30s_buffer(void)
{
    y_printf("%s",__func__);

    memset(collect_30s_buffer,0,sizeof(collect_30s_buffer));
    temp_history_index = 0;
}

/*
**********************************************************************
函数功能：记录历史数据的文件数量
函数形参：
函数返回值：None
备注：
日期：2024年12月05日
作者：lozloz
版本：V0.0
**********************************************************************
*/

void write_history_file(void)
{
    u16 ret;
    ret = syscfg_write(CFG_USER_HISTORY_FILE,&history_data.file,1);
    if(ret<0){
        r_printf("%s error",__func__);
    }
}

void read_history_file(void)
{
    u16 ret;
    ret = syscfg_read(CFG_USER_HISTORY_FILE,&history_data.file,1);
    if(ret<0){
        r_printf("%s error",__func__);
    }
    loz_exflash_var.temp_file = history_data.file;

    y_printf("-------------------history_data.file = %d",history_data.file);
}


/*
**********************************************************************
函数功能：记录历史数据的索引偏移量
函数形参：
函数返回值：None
备注：
日期：2024年12月05日
作者：lozloz
版本：V0.0
**********************************************************************
*/

void write_history_index(void)
{
    u16 ret;
    ret = syscfg_write(CFG_USER_HISTORY_INDEX,&exflash_offset_index,4);
    if(ret<0){
        r_printf("%s error",__func__);
    }
}

void read_history_index(void)
{
    u16 ret;
    ret = syscfg_read(CFG_USER_HISTORY_INDEX,&exflash_offset_index,4);
    if(ret<0){
        r_printf("%s error",__func__);
    }

    y_printf("-------------------exflash_offset_index = %d",exflash_offset_index);
}


/*
**********************************************************************
函数功能：记录外部存储flash总体偏移量
函数形参：
函数返回值：None
备注：
日期：2024年12月05日
作者：lozloz
版本：V0.0
**********************************************************************
*/

void write_history_offset(void)
{
    u16 ret;
    ret = syscfg_write(CFG_USER_HISTORY_OFFSET,&exflash_offset,4);
    if(ret<0){
        r_printf("%s error",__func__);
    }
}

void read_history_offset(void)
{
    u16 ret;
    ret = syscfg_read(CFG_USER_HISTORY_OFFSET,&exflash_offset,4);
    if(ret<0){
        r_printf("%s error",__func__);
    }

    y_printf("-----------------exflash_offset = 0x%x",exflash_offset);
}


/*
**********************************************************************
函数功能：记录外部存储flash文件头地址偏移量
函数形参：
函数返回值：None
备注：
日期：2024年12月05日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void write_history_head_file_offset(void)
{
    u16 ret;
    ret = syscfg_write(CFG_USER_LAST_HEAD_OFFSET,&last_file_offset_index,4);
    if(ret<0){
        r_printf("%s error",__func__);
    }
}

void read_history_head_file_offset(void)
{
    u16 ret;
    ret = syscfg_read(CFG_USER_LAST_HEAD_OFFSET,&last_file_offset_index,4);
    if(ret<0){
        r_printf("%s error",__func__);
    }

    if(exflash_offset_index != last_file_offset_index){
        exflash_offset_index = last_file_offset_index;
        write_history_index();
    }
    y_printf("-----------------last_file_offset_index = 0x%x",last_file_offset_index);
}


/*
**********************************************************************
函数功能:擦除块区
函数形参：
函数返回值：None
备注：
日期：2024年12月10日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void erase_block(void)
{
    int ret = 0;
    int offset = 0;
    ret = dev_ioctl(dev_ptr, IOCTL_ERASE_BLOCK, offset);
    log_info("%s[IOCTL_ERASE_BLOCK -> offset:%d ret:%d]", __func__, offset, ret);
}


/*
**********************************************************************
函数功能:擦除片区
函数形参：
函数返回值：None
备注：
日期：2024年12月10日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void erase_chip(void)
{
    int ret = 0;
    int offset = 0;
    ret = dev_ioctl(dev_ptr, IOCTL_ERASE_CHIP, offset);
    log_info("%s[IOCTL_ERASE_CHIP -> offset:%d ret:%d]", __func__, offset, ret);

//并且擦除所有的记忆
#if 1
    history_data.file = 0;
    exflash_offset = FLASH_SECTOR;
    exflash_offset_index = FLASH_SECTOR;
    last_file_offset_index = FLASH_SECTOR;
    write_history_file();
    write_history_head_file_offset();
    write_history_index();
    write_history_offset();

    loz_exflash_var.temp_file = 0;             //临时变量清零
    clear_30s_buffer();                     //临时偏移量清零
#endif
}

/*
**********************************************************************
函数功能:传输数据完成或者异常中断后处理的一些buffer和标志位
函数形参：
函数返回值：None
备注：
日期：2024年12月10日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void trans_data_ok_or_assert(void)
{
    temp_history_index = 0;         //30s临时地址偏移索引清零
}


/*
**********************************************************************
函数功能:处理心电历史数据
函数形参：
函数返回值：None
备注：
日期：2024年12月06日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void history_data_write_deal(u16 ecg_vol)
{
    static u16 history_cnt = 0;
    int ret = 0;
    u32 len = 0;
    static u32 offset = 0;

    if(app_var.real_flag){
        return;
    }


    history_data.buffer[history_cnt++]  = (ecg_vol>>8) & 0xFF; 
    history_data.buffer[history_cnt++]  = ecg_vol & 0xFF; 



    if(history_cnt == 250){                                     //注意临界值，不要越界了
        history_cnt = 0;    
        
#if 1
        // log_debug_hexdump(buf, DBUG_PRINTF_LEN);

        if(elec_heart.heart_second<=30){
            
            memcpy(&collect_30s_buffer_data[temp_history_index],&history_data,DETECT_1S_OK_LEN);
            collect_30s_buffer_data[temp_history_index].file = loz_exflash_var.temp_file+1;
            // r_printf("temp_history_index = %d,collect_30s_buffer_data[temp_history_index].file = %d",temp_history_index,collect_30s_buffer_data[temp_history_index].file);
            temp_history_index++;
            if(elec_heart.heart_second == 30){                                                                          //30s之前的数据特殊处理，没有30s,就不写入flash
                len = temp_history_index*DETECT_1S_OK_LEN;                                                              
                //write
                ret = dev_bulk_write(dev_ptr, &collect_30s_buffer_data, exflash_offset, len);
                log_info("<<<<<<<<<<<<<<<<<<<<<<<<<%s[dev_bulk_write -> exflash_offset:%d len:%d ret:%d]", __func__, exflash_offset, len, ret);
                exflash_offset += temp_history_index*DETECT_1S_OK_LEN;   
                r_printf("exflash_offset = %d",exflash_offset);                                              //从第二片扇区开始，记录偏移量
                loz_exflash_var.temp_file = history_data.file;                                              //临时文件数量存储重新赋值
                temp_history_index=0;
            }
        }
        else
        {
            len = DETECT_1S_OK_LEN;

            //write
            // put_buf(&history_data,sizeof(history_data));
            ret = dev_bulk_write(dev_ptr, &history_data, exflash_offset, sizeof(history_data));
            log_info("%s[dev_bulk_write -> exflash_offset:%d len:%d ret:%d]", __func__, exflash_offset, len, ret);

#if 0
            //read
            ret = dev_bulk_read(dev_ptr, &test_history_data, exflash_offset, len);
            log_info("++++++++++++++++++++++%s[dev_bulk_read -> exflash_offset:%d len:%d ret:%d]", __func__, exflash_offset, len, ret);
            log_debug_hexdump(&test_history_data, sizeof(HISTORY_DATA));
#endif
            exflash_offset += DETECT_1S_OK_LEN;                               //从第二片扇区开始，记录地址偏移量

        }
#endif
            write_history_offset();
    }

}

/*
**********************************************************************
函数功能:读取历史数据设置
函数形参：
函数返回值：None
备注：
日期：2024年12月06日
作者：lozloz
版本：V0.0
**********************************************************************
*/
u16 read_flash_timer_id;

void notify_send_history_data(void)
{
    struct sys_event e;
    e.type = SYS_KEY_EVENT;
    e.u.key.init = 1;
    e.u.key.type = 0;//区分按键类型
    e.u.key.event = 0;
    e.u.key.value = 0;

    e.arg  = (void *)DEVICE_EVENT_FROM_HISTORY;
    sys_event_notify(&e);
}

void history_data_read_deal(void)
{
    u32 ret;
    u32 len = 256;
    static u8 close_cnt;                    //显示
    y_printf("exflash_offset_index = %d,exflash_offset = %d",exflash_offset_index,exflash_offset);
#if 1  
    //read
    if(elec_heart.history_flag == 1){

        if(exflash_offset_index<exflash_offset){
            ret = dev_bulk_read(dev_ptr, &cur_history_data, exflash_offset_index, len);
            log_info("++++++++++++++++++++++%s[dev_bulk_read -> exflash_offset_index:%d len:%d ret:%d]", __func__, exflash_offset_index, len, ret);
            // exflash_offset_index += 256;
            // write_history_index();
    //当文件头文件不同步的时候记录这个头文件的地址
            if(last_history_data.file != cur_history_data.file){
                g_printf("---------------------------------------not same file---------------------------------------------------");
                r_printf("last_history_data.file = %d,cur_history_data.file = %d",last_history_data.file,cur_history_data.file);
                last_file_offset_index = exflash_offset_index;
                write_history_head_file_offset();                                                  //记录此刻的文件头地址
                ack_file_num_ok(last_history_data.file);                                        //主动告诉app有一个文件流传输全部完成了
                memcpy(&last_history_data,&cur_history_data,sizeof(HISTORY_DATA));
                exflash_offset_index += 256;                                                          //历史数据索引偏移
                write_history_index();                    
                if(last_history_data.file>1){
                    elec_heart.history_flag = 0;                                                        //等云服务处理完后继续下发下发数据
                    return;
                }
            }
            log_debug_hexdump(&cur_history_data, sizeof(HISTORY_DATA));
            notify_send_history_data();
            memcpy(&last_history_data,&cur_history_data,sizeof(HISTORY_DATA));
            exflash_offset_index += 256;                                                          //历史数据索引偏移
            write_history_index();            
            loz_exflash_var.history_flag = 1;
            // if()
        }else{                   //机制：如果是读满了（read all data ok），就正片flash进行擦除数据处理
            if(!close_cnt){
                syn_data_all_ok_mode();
                ack_file_num_ok(last_history_data.file);                                        //主动告诉app最后一个文件流传输全部完成了
            }

            close_cnt++;
            if(close_cnt == 10){
                close_cnt = 0;
                clear_screen();
                erase_chip();
                //停止读取数据
                trans_history_over_flag();
                loz_exflash_var.history_flag = 0;
                elec_heart.history_flag = 0;
                delete_history_read();
            }

        }
    }
    
#endif    
}


void history_data_read_handle(void)
{
    log_info("%s",__func__);
    if(read_flash_timer_id == 0){
        read_flash_timer_id = sys_timer_add(NULL,history_data_read_deal,100);
    }
}

void delete_history_read(void)
{
    if(read_flash_timer_id){
        sys_timer_del(read_flash_timer_id);
        read_flash_timer_id = 0;
    }
}

/*
**********************************************************************
函数功能:读取历史数据剩余文件操作
函数形参：
函数返回值：None
备注：
日期：2024年12月06日
作者：lozloz
版本：V0.0
**********************************************************************
*/
HISTORY_DATA check_final_file;
HISTORY_DATA check_index_file;
u8 remain_history_file(void)
{
    u8 remain_file;
    u32 ret;
    u16 len = 256;
    ret = dev_bulk_read(dev_ptr, &check_final_file, exflash_offset-256, len);
    ret = dev_bulk_read(dev_ptr, &check_index_file, exflash_offset_index, len);

    r_printf("exflash_offset = %u,exflash_offset_index = %u",exflash_offset,exflash_offset_index);
    y_printf("check_final_file.file = %u,check_index_file.file = %u",check_final_file.file,check_index_file.file);
    
    if(check_final_file.file == 0xff && check_index_file.file == 0xff){
        remain_file = 0;
        g_printf("no file");
    }else{
        remain_file = check_final_file.file- check_index_file.file+1;
    }

    return remain_file;

}


/*
**********************************************************************
函数功能:读取历史数据剩余文件操作
函数形参：
函数返回值：None
备注：
日期：2024年12月06日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void check_size_flash(void)
{
    u32 ret;
    u16 len = 8192;

    // ret = dev_bulk_read(dev_ptr, &test_buffer, exflash_offset_index, len);
    ret = dev_bulk_read(dev_ptr, &test_buffer, 0, len);
    
    put_buf(test_buffer,sizeof(test_buffer));
}


void test_one(void)
{
#if 1
    static int cnt= 0;
    int ret;
    u32 len = 0;
    static u32 offset = 0;
    printf("%s",__func__);

    log_debug_hexdump(buf, 256);
    //read
    // offset = 0x100000 - 4096;
    offset = 0;
    len = 512;
    ret = dev_bulk_read(dev_ptr, buf, offset, len);
    log_info("%s[dev_bulk_read -> offset:%d len:%d ret:%d]", __func__, offset, len, ret);
    log_debug_hexdump(buf, 256);
    //erase
    ret = dev_ioctl(dev_ptr, IOCTL_ERASE_SECTOR, offset);
    log_info("%s[IOCTL_ERASE_SECTOR -> offset:%d ret:%d]", __func__, offset, ret);

#if 0
    //read
    ret = dev_bulk_read(dev_ptr, buf, offset, len);
    log_info("after erase %s[dev_bulk_read -> offset:%d len:%d ret:%d]", __func__, offset, len, ret);
    log_debug_hexdump(buf, 16);
    ret = dev_ioctl(dev_ptr, IOCTL_ERASE_SECTOR, offset);
    log_info("%s[IOCTL_ERASE_SECTOR -> offset:%d ret:%d]", __func__, offset, ret);    
#endif

    //write
    cnt = buf[0] + 1;
    for(u32 i = 0;i < sizeof(buf);i++){
        buf[i] = i;
    }buf[0] = cnt;
    ret = dev_bulk_write(dev_ptr, buf, offset, len);
    log_info("%s[dev_bulk_write -> offset:%d len:%d ret:%d]", __func__, offset, len, ret);
    //不要调用dev_close()
#endif    
}


void key_event_deal(u8 key_event,u8 key_value)
{
    log_info("%s",__func__);
    if(key_event == KEY_EVENT_CLICK)
    {
        // norfs_dev_ops.write(dev_name,res,sizeof(res),1024);
        // norfs_dev_ops.read(dev_name,read_buffer,sizeof(read_buffer),1024);
        test_one();
    }
}



void exflash_imount(void)
{
    r_printf("%s", __func__);

    // return;
#if 0
    fmnt = mount(dev_name, NOR_FLASH_ROOT_PATH, "nor_sdfile", 1, NULL);
    log_info("%s[mount(%s):0x%x]", __func__, NOR_FLASH_ROOT_PATH, fmnt);
    printf("-------------------------------------------------------------------");
    if(!fmnt){
        log_error("%s mount(%s) fail", __func__, NOR_FLASH_ROOT_PATH);
        return;
    }
#endif    

#if 1
    dev_ptr = dev_open(dev_name, NULL);
    y_printf("-------------------------------------------------------------------");

    log_info("%s[dev:0x%x]", __func__, dev_ptr);
    if (!dev_ptr) {
        log_error("%s dev_open fail", __func__);
        return;
    }
#endif

    // sys_timer_add(NULL, exflash_test_sys_loop, 1000 * 1);

}

void exflash_flash_tone_play(int tone_num)
{
    log_info("%s", __func__);
    tone_play_by_path(tone_table[tone_num], 1);    
}

