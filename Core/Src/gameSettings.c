#include "gameSettings.h"
#include "flashStorage.h"
#include "crc.h"
#include "levels.h"

/* Declare the settings in the reserved area */
typedef union {
    GameSettings settings;
    uint8_t raw[FLASH_PAGE_SIZE];
} FlashGameSettings;

// Place gameSettings in the reserved .settings section
__attribute__((section(".settings"), used)) FlashGameSettings gameData;

static uint32_t calculateChecksum(GameSettings *settings)
{
     uint32_t checksum = HAL_CRC_Calculate(&hcrc, (uint32_t *) settings,
                         (sizeof(GameSettings) - sizeof(settings->checksum)) / sizeof(uint32_t));
    return checksum;
}

static bool saveSettings(GameSettings *settings)
{
    settings->checksum = calculateChecksum(settings);

    bool res = flashStorageErase((uint32_t) &gameData);
    if (!res) {
        return false;
    }
    res = flashStorageWrite((uint32_t) &gameData, (uint32_t *) settings, sizeof(GameSettings));
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
    *settings = gameData.settings;
    if (gameData.settings.version != FLASH_STORAGE_VERSION) {
        return false;
    }
    uint32_t checksum = calculateChecksum(settings);
    bool match = checksum == gameData.settings.checksum;
    return match;
}

bool gameSettingsWrite(GameSettings *settings)
{
    return saveSettings(settings);
}
