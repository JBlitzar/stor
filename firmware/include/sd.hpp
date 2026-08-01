#pragma once

#include <cstdint>

namespace sd {

constexpr uint32_t kBlockSize = 512;

bool init();

void deinit();

bool ready();

uint32_t block_count();

bool read(uint8_t* dst, uint32_t lba, uint32_t blocks);

bool write(const uint8_t* src, uint32_t lba, uint32_t blocks);

bool poll_presence();

}
