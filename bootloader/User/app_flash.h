#ifndef APPFLASH
#define APPFLASH
#include <stdint.h>

//30KB 作为bootloader
//98KB 作为app
#define FLASH_USER_START_ADDR   (FLASH_BASE + FLASH_PAGE_SIZE * 240)             
#define FLASH_USER_END_ADDR     (FLASH_USER_START_ADDR + FLASH_PAGE_SIZE * 784)   

#define APP_FLASH_PAGE_SIZE     784


int8_t flashEarse(uint32_t flashaddr, uint16_t numofpage);
int8_t flashEarseByFileSize(uint32_t filesize);
int8_t flashWriteCode(uint32_t addr, uint8_t *code,uint16_t codelen);

#endif
