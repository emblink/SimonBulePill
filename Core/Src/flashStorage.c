#include "flashStorage.h"
#include "stm32f1xx.h"

bool flashStorageWrite(uint32_t address, uint32_t *data, uint32_t size)
{
    HAL_FLASH_Unlock(); // Unlock Flash for write

    for (uint32_t i = 0; i < size / sizeof(uint32_t); i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i * sizeof(uint32_t), data[i]) != HAL_OK) {
            HAL_FLASH_Lock(); // Lock Flash after write
            return false;
        }
    }

    HAL_FLASH_Lock(); // Lock Flash after write
    return true;
}

bool flashStorageErase(uint32_t pageAddress)
{
    HAL_FLASH_Unlock(); // Unlock Flash for erase

    FLASH_EraseInitTypeDef eraseInitStruct = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .PageAddress = pageAddress,
        .NbPages = 1
    };

    uint32_t pageError = 0;
    if (HAL_FLASHEx_Erase(&eraseInitStruct, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }

    HAL_FLASH_Lock(); // Lock Flash after erase
    return true;
}

void flashStorageRead(uint32_t address, void *data, uint32_t size)
{
    uint8_t *src = (uint8_t *) address;
    uint8_t *dest = (uint8_t *) data;

    for (uint32_t i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}
