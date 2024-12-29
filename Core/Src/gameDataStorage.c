#include "gameDataStorage.h"
#include "stm32f1xx.h"
#include <string.h>

//#define FLASH_PAGE_SIZE 1024  // 1 KB for STM32F103
#define FLASH_BASE_ADDRESS 0x0801F800 // Last page for a 128 KB Flash

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

void Flash_Read(uint32_t address, void *data, uint32_t size)
{
    memcpy(data, (void *)address, size);
}

static uint32_t calculateChecksum(GameSettings *settings)
{
    uint32_t checksum = 0;
    uint32_t *data = (uint32_t *)settings;

    for (size_t i = 0; i < (sizeof(GameSettings) / sizeof(uint32_t)) - 1; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

static void saveSettings(GameSettings *settings)
{
    settings->checksum = calculateChecksum(settings);

    Flash_Erase(FLASH_BASE_ADDRESS);
    Flash_Write(FLASH_BASE_ADDRESS, (uint32_t *)settings, sizeof(GameSettings));
}

static void resetToDefaults()
{
    GameSettings defaultSettings = {
        .level = 1,
        .speed = GAME_SPEED_SLOW,
        .mode = GAME_MODE_SINGLE,
        .sequence = GAME_SEQUENCE_STATIC
    };
    saveSettings(&defaultSettings);
}

bool gameDataStorageInit()
{
    return true;
}

void gameDataStorageReset()
{
    resetToDefaults();
}

bool gameDataStorageRead(GameSettings *settings)
{
    Flash_Read(FLASH_BASE_ADDRESS, settings, sizeof(GameSettings));
    return (calculateChecksum(settings) == settings->checksum);
}

bool gameDataStorageWrite(GameSettings settings)
{
    return false;
}
