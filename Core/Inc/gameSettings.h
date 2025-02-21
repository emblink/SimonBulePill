#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GAME_SPEED_SLOW = 0U,
    GAME_SPEED_MEDIUM,
    GAME_SPEED_HIGH,
    GAME_SPEED_COUNT
} GameSpeed;

typedef enum {
    GAME_MODE_SINGLE = 0U,
    GAME_MODE_PVP,
    GAME_MODE_MUSIC,
    GAME_MODE_COUNT,
} GameMode;

typedef enum {
    GAME_SEQUENCE_STATIC = 0U,
    GAME_SEQUENCE_RANDOM,
    GAME_SEQUENCE_COUNT,
} GameSequence;

#pragma pack(push, 1)
typedef struct {
    uint8_t version;
    uint8_t level;
    uint8_t speed;
    uint8_t mode;
    uint8_t sequence;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
    uint32_t checksum;
} GameSettings;
#pragma pack(pop)

bool gameSettingsRead(GameSettings *settings);
bool gameSettingsWrite(GameSettings *settings);
void gameSettingsReset();