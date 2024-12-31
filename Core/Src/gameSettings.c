#include "gameSettings.h"
#include "flashStorage.h"
#include "crc.h"
#include "levels.h"

static uint32_t calculateChecksum(GameSettings *settings)
{
     uint32_t checksum = HAL_CRC_Calculate(&hcrc, (uint32_t *) settings,
                         (sizeof(GameSettings) - sizeof(settings->checksum)) / sizeof(uint32_t));
    return checksum;
}

static bool saveSettings(GameSettings *settings)
{
    settings->checksum = calculateChecksum(settings);

    bool res = flashStorageErase(FLASH_BASE_ADDRESS);
    if (!res) {
        return false;
    }
    res = flashStorageWrite(FLASH_BASE_ADDRESS, (uint32_t *) settings, sizeof(GameSettings));
    if (!res) {
        return false;
    }

    return true;
}

void gameSettingsReset()
{
    GameSettings defaultSettings = {
        .version = FLASH_STORAGE_VERSION,
        .level = LEVEL_1,
        .speed = GAME_SPEED_SLOW,
        .mode = GAME_MODE_SINGLE,
        .sequence = GAME_SEQUENCE_STATIC,
    };
    saveSettings(&defaultSettings);
}

bool gameSettingsRead(GameSettings *settings)
{
    flashStorageRead(FLASH_BASE_ADDRESS, settings, sizeof(GameSettings));
    if (settings->version != FLASH_STORAGE_VERSION) {
        return false;
    }
    uint32_t checksum = calculateChecksum(settings);
    bool match = checksum == settings->checksum;
    return match;
}

bool gameSettingsWrite(GameSettings *settings)
{
    return saveSettings(settings);
}
