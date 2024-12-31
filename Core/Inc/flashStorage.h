#pragma once
#include <stdint.h>
#include <stdbool.h>

#define FLASH_STORAGE_VERSION 1
//#define FLASH_PAGE_SIZE 1024  // 1 KB for STM32F103
#define FLASH_BASE_ADDRESS 0x0800FC00 // Last page for a 64 KB Flash

bool flashStorageWrite(uint32_t address, uint32_t *data, uint32_t size);
bool flashStorageErase(uint32_t pageAddress);
void flashStorageRead(uint32_t address, void *data, uint32_t size);
