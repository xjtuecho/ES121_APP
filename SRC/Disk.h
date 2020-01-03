/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
 File Name :      Disk.h
 Version :        1.7a
 Description:
 Author :         Ning
 Data:            2017/11/22
 History:
 *******************************************************************************/
#ifndef __DISK_H
#define __DISK_H

#include "USB_scsi.h"
#include "USB_regs.h"
#include "USB_conf.h"
#include "USB_bot.h"
#include "USB_mem.h"
#include "USB_lib.h"
#include "USB_pwr.h"
#include "stm32f10x.h"

#define SECTOR_SIZE    512
#define SECTOR_CNT     4096

#define FAT1_SECTOR    &gDisk_buff[0x000]
#define FAT2_SECTOR    &gDisk_buff[0x200]
#define ROOT_SECTOR    &gDisk_buff[0x400]
#define VOLUME_BASE    &gDisk_buff[0x416]
#define OTHER_FILES    &gDisk_buff[0x420]
#define FILE_SECTOR    &gDisk_buff[0x600]
#define Root           (uint8_t*)ROOT_SECTOR

#define APP_BASE       0x0800C000
#define MT_CONT_BASE   0x0800E800

extern uint8_t gCalib_flag;
extern uint32_t gMoto_timecnt;

#define HEX            0
#define BIN            2
#define SET            1

#define RDY            0
#define NOT            2
#define END            3
#define ERR            4

#define DATA_SEG       0x00
#define DATA_END       0x01
#define EXT_ADDR       0x04

#define TXFR_IDLE      0
#define TXFR_ONGOING   1

//#define FAT_DATA       0x00FFFFF8
#define VOLUME         0x40DD8D18  //0x3E645C29

#define BUFF           0             // 扇区数据缓冲区

#define V32_BASE       SECTOR_SIZE   // V32 总共 8*4=32 字节
#define W_ADDR         0
#define ADDR           1
#define H_ADDR         2
#define OFFSET         3
#define SEC_CNT        4
#define COUNT          5
#define RD_CNT         6
#define WR_CNT         7

#define VAR_BASE       V32_BASE + 32 // VAR 总共 9+17=26 字节
#define USB_ST         0
#define SEG_KIND       1
#define SEG_LEN        2
#define SEG_SUM        3
#define SEG_TMP        4
#define SEG_ST         5
#define DATA_CNT       6
#define F_TYPE         7
#define F_FLAG         8
#define SEG_DATA       9             // 通信包缓冲区 9~26 共17字节

uint8_t Cal_Val(uint8_t str[], uint8_t k);
void Disk_BuffInit(void);
uint8_t ReWrite_All(void);
uint8_t Config_Analysis(void);
void Disk_BuffInit(void);
void Disk_SecWrite(uint8_t* pBuffer, uint32_t DiskAddr);
void Disk_SecRead(uint8_t* pBuffer, uint32_t DiskAddr);
void Soft_Delay(void);

void Write_Memory(uint32_t w_offset, uint32_t w_length);
void Read_Memory(uint32_t r_offset, uint32_t r_length);
void Set_Ver(uint8_t str[], uint8_t i);
void Upper(uint8_t* str, uint16_t len);
uint8_t* SearchFile(uint8_t* pfilename, uint16_t* pfilelen, uint16_t* root_addr);
uint8_t ReWriteFlash(void);
uint32_t Read_MtCnt(void);
uint8_t Write_MtFlash(void);
uint8_t FLASH_Prog(uint32_t Address, uint16_t Data);
void FLASH_Erase(uint32_t Address);
uint8_t Setting_Analysis(uint8_t* p_file, uint16_t root_addr);

#endif
