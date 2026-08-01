#include "sd.hpp"

#include "board.hpp"
#include "logging.hpp"
#include "stm32h7xx_hal.h"

namespace {

SD_HandleTypeDef g_sd{};
bool g_ready = false;
bool g_last_present = false;

bool wait_idle(uint32_t timeout_ms) {
  const uint32_t start = HAL_GetTick();
  while (HAL_SD_GetCardState(&g_sd) != HAL_SD_CARD_TRANSFER) {
    if (HAL_GetTick() - start > timeout_ms) return false;
  }
  return true;
}

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
  // 100 MHz kernel clock / (2 x 1) = 50 MHz, the high-speed ceiling without
  // UHS-I. HAL drops to 400 kHz on its own for the identification phase.
  g_sd.Init.ClockDiv = 1;

  if (HAL_SD_Init(&g_sd) != HAL_OK) {
    logging::printf("sd: init failed (0x%lx)", static_cast<unsigned long>(g_sd.ErrorCode));
    return false;
  }
  if (HAL_SD_ConfigWideBusOperation(&g_sd, SDMMC_BUS_WIDE_4B) != HAL_OK) {
    logging::printf("sd: 4-bit mode failed (0x%lx)",
                static_cast<unsigned long>(g_sd.ErrorCode));
    return false;
  }

  HAL_SD_CardInfoTypeDef info{};
  HAL_SD_GetCardInfo(&g_sd, &info);
  logging::printf("sd: %lu blocks x %lu B = %lu MiB",
              static_cast<unsigned long>(info.LogBlockNbr),
              static_cast<unsigned long>(info.LogBlockSize),
              static_cast<unsigned long>(info.LogBlockNbr / 2048));

  g_ready = true;
  return true;
}

void deinit() {
  if (!g_ready) return;
  HAL_SD_DeInit(&g_sd);
  g_ready = false;
}

bool ready() { return g_ready; }

uint32_t block_count() {
  if (!g_ready) return 0;
  HAL_SD_CardInfoTypeDef info{};
  HAL_SD_GetCardInfo(&g_sd, &info);
  return info.LogBlockNbr;
}

bool read(uint8_t* dst, uint32_t lba, uint32_t blocks) {
  if (!g_ready) return false;
  if (HAL_SD_ReadBlocks(&g_sd, dst, lba, blocks, 5000) != HAL_OK) return false;
  return wait_idle(5000);
}

bool write(const uint8_t* src, uint32_t lba, uint32_t blocks) {
  if (!g_ready) return false;
  if (HAL_SD_WriteBlocks(&g_sd, const_cast<uint8_t*>(src), lba, blocks, 5000) != HAL_OK) {
    return false;
  }
  return wait_idle(5000);
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
