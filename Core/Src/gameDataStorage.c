#include "gameDataStorage.h"
#include "stm32f1xx.h"
#include <string.h>
#include "crc.h"

//#define FLASH_PAGE_SIZE 1024  // 1 KB for STM32F103
#define FLASH_BASE_ADDRESS 0x0800FC00 // Last page for a 64 KB Flash

static HAL_StatusTypeDef Flash_Write(uint32_t address, uint32_t *data, uint32_t size)
{
    HAL_FLASH_Unlock(); // Unlock Flash for write

    for (uint32_t i = 0; i < size / sizeof(uint32_t); i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i * sizeof(uint32_t), data[i]) != HAL_OK) {
            HAL_FLASH_Lock(); // Lock Flash after write
            return HAL_ERROR;
        }
    }

    HAL_FLASH_Lock(); // Lock Flash after write
    return HAL_OK;
}

static HAL_StatusTypeDef Flash_Erase(uint32_t pageAddress)
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
        return HAL_ERROR;
    }

    HAL_FLASH_Lock(); // Lock Flash after erase
    return HAL_OK;
}

static void Flash_Read(uint32_t address, void *data, uint32_t size)
{
    memcpy(data, (void *)address, size);
}

static uint32_t calculateChecksum(GameSettings *settings)
{
     uint32_t checksum = HAL_CRC_Calculate(&hcrc, (uint32_t *) settings,
                         (sizeof(GameSettings) - sizeof(settings->checksum)) / sizeof(uint32_t));
    return checksum;
}

static bool saveSettings(GameSettings *settings)
{
    settings->checksum = calculateChecksum(settings);

    HAL_StatusTypeDef res = Flash_Erase(FLASH_BASE_ADDRESS);
    if (HAL_OK != res) {
        return false;
    }
    res = Flash_Write(FLASH_BASE_ADDRESS, (uint32_t *)settings, sizeof(GameSettings));
    if (HAL_OK != res) {
        return false;
    }

    return true;
}

void gameDataStorageReset()
{
    GameSettings defaultSettings = {
        .version = DATA_STORAGE_VERSION,
        .level = 1,
        .speed = GAME_SPEED_SLOW,
        .mode = GAME_MODE_SINGLE,
        .sequence = GAME_SEQUENCE_STATIC,
    };
    saveSettings(&defaultSettings);
}

bool gameDataStorageRead(GameSettings *settings)
{
    Flash_Read(FLASH_BASE_ADDRESS, settings, sizeof(GameSettings));
    uint32_t checksum = calculateChecksum(settings);
    bool match = checksum == settings->checksum;
    return match;
}

bool gameDataStorageWrite(GameSettings *settings)
{
    return saveSettings(settings);
}
