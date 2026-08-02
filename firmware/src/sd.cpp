#include "sd.hpp"

#include "board.hpp"
#include "logging.hpp"
#include "stm32h7xx_hal.h"

namespace {

SD_HandleTypeDef g_sd{};
bool g_ready = false;
bool g_last_present = false;
uint32_t g_block_count = 0;

bool wait_idle(uint32_t timeout_ms) {
  const uint32_t start = HAL_GetTick();
  while (HAL_SD_GetCardState(&g_sd) != HAL_SD_CARD_TRANSFER) {
    if (HAL_GetTick() - start > timeout_ms) return false;
  }
  return true;
}

#if STOR_PERF_LOG
uint32_t g_perf_start = 0;
uint32_t g_perf_ticks = 0;
uint32_t g_perf_blocks = 0;

void perf_begin() { g_perf_start = HAL_GetTick(); }

void perf_end(uint32_t blocks) {
  g_perf_ticks += HAL_GetTick() - g_perf_start;
  g_perf_blocks += blocks;
  if (g_perf_ticks < 1000) return;
  logging::printf("sd: %lu KB/s",
              static_cast<unsigned long>(g_perf_blocks / 2 * 1000 / g_perf_ticks));
  g_perf_ticks = 0;
  g_perf_blocks = 0;
}
#else
void perf_begin() {}
void perf_end(uint32_t) {}
#endif

}

namespace sd {

bool init() {
  if (g_ready) return true;
  if (!board::card_present()) return false;

  __HAL_RCC_SDMMC1_CLK_ENABLE();

  g_sd.Instance = SDMMC1;
  g_sd.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  g_sd.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  g_sd.Init.BusWide = SDMMC_BUS_WIDE_4B;
  g_sd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  // 100 MHz kernel clock / (2 x 2) = 25 MHz, in spec for default speed. CMD6
  // has to happen at this clock before we can go to 50 MHz.
  g_sd.Init.ClockDiv = 2;

  if (HAL_SD_Init(&g_sd) != HAL_OK) {
    logging::printf("sd: init failed (0x%lx)", static_cast<unsigned long>(g_sd.ErrorCode));
    return false;
  }
  if (HAL_SD_ConfigWideBusOperation(&g_sd, SDMMC_BUS_WIDE_4B) != HAL_OK) {
    logging::printf("sd: 4-bit mode failed (0x%lx)",
                static_cast<unsigned long>(g_sd.ErrorCode));
    return false;
  }

  if (HAL_SD_ConfigSpeedBusOperation(&g_sd, SDMMC_SPEED_MODE_HIGH) == HAL_OK) {
    MODIFY_REG(g_sd.Instance->CLKCR, SDMMC_CLKCR_CLKDIV, 1U);
    g_sd.Init.ClockDiv = 1;
  } else {
    logging::printf("sd: high speed refused (0x%lx), staying at 25 MHz",
                static_cast<unsigned long>(g_sd.ErrorCode));
    g_sd.ErrorCode = HAL_SD_ERROR_NONE;
  }

  HAL_SD_CardInfoTypeDef info{};
  HAL_SD_GetCardInfo(&g_sd, &info);
  g_block_count = info.LogBlockNbr;
  logging::printf("sd: %lu blocks x %lu B = %lu MiB at %lu MHz",
              static_cast<unsigned long>(info.LogBlockNbr),
              static_cast<unsigned long>(info.LogBlockSize),
              static_cast<unsigned long>(info.LogBlockNbr / 2048),
              static_cast<unsigned long>(50 / g_sd.Init.ClockDiv));

  g_ready = true;
  return true;
}

void deinit() {
  if (!g_ready) return;
  HAL_SD_DeInit(&g_sd);
  g_ready = false;
  g_block_count = 0;
}

bool ready() { return g_ready; }

uint32_t block_count() { return g_block_count; }

bool read(uint8_t* dst, uint32_t lba, uint32_t blocks) {
  if (!g_ready) return false;
  perf_begin();
  const bool ok = HAL_SD_ReadBlocks(&g_sd, dst, lba, blocks, 5000) == HAL_OK;
  perf_end(blocks);
  return ok;
}

bool write(const uint8_t* src, uint32_t lba, uint32_t blocks) {
  if (!g_ready) return false;
  perf_begin();
  const bool ok = HAL_SD_WriteBlocks(&g_sd, const_cast<uint8_t*>(src), lba, blocks, 5000) ==
                      HAL_OK &&
                  wait_idle(5000);
  perf_end(blocks);
  return ok;
}

bool poll_presence() {
  const bool present = board::card_present();
  if (present == g_last_present) return false;
  g_last_present = present;

  if (present) {
    logging::printf("sd: card inserted");
    init();
  } else {
    logging::printf("sd: card removed");
    deinit();
  }
  return true;
}

}
