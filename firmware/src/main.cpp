#include "board.hpp"
#include "clock.hpp"
#include "logging.hpp"
#include "sd.hpp"
#include "stm32h7xx_hal.h"
#include "tusb.h"

int main() {
  HAL_Init();
  clock::init();
  board::init();
  logging::init();
  logging::printf("stor: boot, SYSCLK %lu Hz", static_cast<unsigned long>(SystemCoreClock));

  board::reset_phy();
  if (!board::phy_clock_alive()) {
    logging::fatal("no ULPI clock on PA5; check U3 reset (PD3) and the 24 MHz X3");
  }

  board::enable_usb_clocks();

  const tud_configure_dwc2_t usb_cfg = {
      .bm_double_buffered = 1u << 1,
      .vbus_sensing = false,
  };
  tud_configure(0, TUD_CFGID_DWC2, &usb_cfg);

  tusb_init();
  sd::poll_presence();

  uint32_t last_poll = HAL_GetTick();
  while (true) {
    tud_task();
    if (HAL_GetTick() - last_poll >= 100) { // 100 ms
      last_poll = HAL_GetTick();
      sd::poll_presence();
    }
  }
}

extern "C" {

void SysTick_Handler() { HAL_IncTick(); }

void OTG_HS_IRQHandler() { tud_int_handler(0); }

}
