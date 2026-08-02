#include "clock.hpp"

#include "stm32h7xx_hal.h"

namespace {

[[noreturn]] void halt() {
  while (true) {
    __NOP();
  }
}

}

namespace clock {

void init() {
  SCB_EnableICache();
  SCB_EnableDCache();

  if (HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY) != HAL_OK) halt();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  RCC_OscInitTypeDef osc{};
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = RCC_HSE_ON;  // X1 is a crystal not an oscillator.
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc.PLL.PLLM = 5;                     // 25 MHz / 5 = 5 MHz reference
  osc.PLL.PLLN = 160;                   // 5 x 160 = 800 MHz VCO
  osc.PLL.PLLP = 2;                     // 400 MHz SYSCLK
  osc.PLL.PLLQ = 8;                     // 100 MHz SDMMC kernel clock
  osc.PLL.PLLR = 2;
  osc.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;  // 4-8 MHz reference band
  osc.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;  // 192-836 MHz
  osc.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) halt();

  RCC_ClkInitTypeDef clk{};
  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.SYSCLKDivider = RCC_SYSCLK_DIV1;  // 400 MHz core, the VOS1 ceiling
  clk.AHBCLKDivider = RCC_HCLK_DIV2;    // 200 MHz AXI/AHB, also a VOS1 ceiling
  clk.APB3CLKDivider = RCC_APB3_DIV2;   // 100 MHz APB, likewise
  clk.APB1CLKDivider = RCC_APB1_DIV2;
  clk.APB2CLKDivider = RCC_APB2_DIV2;
  clk.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) halt();

  MODIFY_REG(FLASH->ACR, FLASH_ACR_WRHIGHFREQ, FLASH_ACR_WRHIGHFREQ_1);

  RCC_PeriphCLKInitTypeDef periph{};
  periph.PeriphClockSelection = RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_USART1;
  periph.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;  // pll1_q_ck at 100 MHz
  periph.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&periph) != HAL_OK) halt();
}

}
