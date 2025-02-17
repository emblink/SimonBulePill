#pragma once
#include <stdint.h>
#include <stdbool.h>

#define FLASH_STORAGE_VERSION 1

bool flashStorageWrite(uint32_t address, uint32_t *data, uint32_t size);
bool flashStorageErase(uint32_t pageAddress);
