#ifndef __MEMFOX_H__
#define __MEMFOX_H__

#include <stdint.h>
#define UNUSED_UNIT 0XCF
#define DEFAULT_CELL_END 0XDF
#define DEFAULT_CELL_STA 0XAFAFAFAF
#define DEFAULT_CELL_LEN 512
//#define NULL 0x00
#define CELL_TAIL_SIZE 4

//malloc ctrl def
typedef struct{
    uint32_t cell_head_flag;
    uint32_t cell_tail_size;
    uint32_t mem_size;
    uint32_t cell_szie;
    //uint32_t *mem_p;
    uint32_t cell_tail_flag;
}Cell_head;

//fuctions
uint8_t memfox_init(uint8_t *BankStart,uint32_t BankSize);
void* memfox_malloc(uint32_t size);
void memfox_free(void *ptr);
uint32_t memfox_GetBankUsed(void);
uint32_t memfox_GetBankTotal(void);
//debug
uint32_t memfox_GetListIndex(void);
uint32_t memfox_GetLiseLen(void);
void memfox_printlist(void);
#endif