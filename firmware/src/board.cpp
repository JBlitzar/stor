#include "board.hpp"

#include "stm32h7xx_hal.h"

namespace {


GPIO_TypeDef* const kLedActivityPort = GPIOA;
constexpr uint16_t kLedActivityPin = GPIO_PIN_6;
GPIO_TypeDef* const kLedErrorPort = GPIOA;
constexpr uint16_t kLedErrorPin = GPIO_PIN_7;

constexpr uint16_t kPhyResetPin = GPIO_PIN_3;   // PD3, active high
constexpr uint16_t kCardDetectPin = GPIO_PIN_4; // PD4
constexpr uint16_t kUlpiClockPin = GPIO_PIN_5;  // PA5

struct PinCfg {
  GPIO_TypeDef* port;
  uint16_t pin;
  uint32_t mode;
  uint32_t pull;
  uint32_t alternate;
};

const PinCfg kPins[] = {
    // ULPI: CK, D0-D7, STP, DIR, NXT
    {GPIOA, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOA, GPIO_PIN_3, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOB, GPIO_PIN_0, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOB, GPIO_PIN_1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOB, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOB, GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOB, GPIO_PIN_12, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOB, GPIO_PIN_13, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOB, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOC, GPIO_PIN_0, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOC, GPIO_PIN_2, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    {GPIOC, GPIO_PIN_3, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG1_HS},
    // SDMMC1: D0-D3, CK, CMD
    {GPIOC, GPIO_PIN_8, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1},
    {GPIOC, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1},
    {GPIOC, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1},
    {GPIOC, GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1},
    {GPIOC, GPIO_PIN_12, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1},
    {GPIOD, GPIO_PIN_2, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_SDMMC1},
    // USART1 console
    {GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF7_USART1},
    {GPIOA, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF7_USART1},
    // PHY reset, driven low (released) once configured
    {GPIOD, kPhyResetPin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0},
    // Card detect
    {GPIOD, kCardDetectPin, GPIO_MODE_INPUT, GPIO_PULLUP, 0},
    // LEDs: PA6 white (activity), PA7 red (error)
    {GPIOA, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0},
    {GPIOA, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0},
};

void apply(const PinCfg& cfg) {
  GPIO_InitTypeDef init{};
  init.Pin = cfg.pin;
  init.Mode = cfg.mode;
  init.Pull = cfg.pull;
  init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  init.Alternate = cfg.alternate;
  HAL_GPIO_Init(cfg.port, &init);
}

}

namespace board {

void init() {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();

  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_CLOSE);
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_CLOSE);

  for (const PinCfg& cfg : kPins) {
    apply(cfg);
  }
}

void reset_phy() {
  HAL_GPIO_WritePin(GPIOD, kPhyResetPin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOD, kPhyResetPin, GPIO_PIN_RESET);
  HAL_Delay(10);
}

bool phy_clock_alive() {
  GPIO_InitTypeDef probe{};
  probe.Pin = kUlpiClockPin;
  probe.Mode = GPIO_MODE_INPUT;
  probe.Pull = GPIO_NOPULL;
  probe.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &probe);

  bool saw_high = false;
  bool saw_low = false;
  for (uint32_t i = 0; i < 100000 && !(saw_high && saw_low); ++i) {
    if (HAL_GPIO_ReadPin(GPIOA, kUlpiClockPin) == GPIO_PIN_SET) {
      saw_high = true;
    } else {
      saw_low = true;
    }
  }

  apply(kPins[0]);
  return saw_high && saw_low;
}

void enable_usb_clocks() {
  __HAL_RCC_USB1_OTG_HS_CLK_ENABLE();
  __HAL_RCC_USB1_OTG_HS_ULPI_CLK_ENABLE();
}

bool card_present() {
  const bool low = HAL_GPIO_ReadPin(GPIOD, kCardDetectPin) == GPIO_PIN_RESET;
  return kCardDetectActiveLow ? low : !low;
}

void led(Led which, bool on) {
  GPIO_TypeDef* port = (which == Led::Activity) ? kLedActivityPort : kLedErrorPort;
  const uint16_t pin = (which == Led::Activity) ? kLedActivityPin : kLedErrorPin;
  HAL_GPIO_WritePin(port, pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

}
