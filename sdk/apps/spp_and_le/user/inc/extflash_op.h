#ifndef __EXFLASH_OP_H__
#define __EXFLASH_OP_H__


#define FLASH_SECTOR        (4096)

#define DETECT_30S_OK_LEN   (30*256)
#define DETECT_1S_OK_LEN    (256)


typedef struct  __HISTORY_DATA
{
    u8 head;
    u8 file;
    u8 buffer[250];

    u8 unix_time1_h;
    u8 unix_time1_l;
    u8 unix_time0_h;
    u8 unix_time0_l;
}HISTORY_DATA;


//一些全局数据的结构体
typedef struct  __LOZ_EXFLASH_VAR{
    u8 history_flag;          //是否/卡主一些图形，在上传历史数据，在上传历史数据的时候，优先级高，卡主1.实时数据的传输，2.再继续传输历史数据，3.采集心电数据，4，显示时间及其心跳
    u8 temp_file;               //临时文件管理标志位
}LOZ_EXFLASH_VAR;



void clear_30s_buffer(void);

//记录历史文件
void write_history_file(void);
void read_history_file(void);
//记录历史数据索引偏移
void write_history_index(void);
void read_history_index(void);
//记录历史全部数据偏移
void write_history_offset(void);
void read_history_offset(void);
//记录历史数据每个头文件的地址偏移
void write_history_head_file_offset(void);
void read_history_head_file_offset(void);

//查询最后未上传的文件数量
u8 remain_history_file(void);


//块区擦除功能
void erase_block(void);
//片区擦除功能
void erase_chip(void);

void history_data_read_handle(void);
void delete_history_read(void);
void check_size_flash(void);


extern HISTORY_DATA history_data;
extern LOZ_EXFLASH_VAR loz_exflash_var;
extern HISTORY_DATA last_history_data;     //读取历史数据用来存储当前读取的历史数据
extern HISTORY_DATA cur_history_data;      //读取历史数据用来存储上一次的历史数据
#endif
