#include "logging.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "stm32h7xx_hal.h"

namespace {

UART_HandleTypeDef g_uart{};
char g_buf[256];

void emit(const char* prefix, const char* fmt, va_list ap) {
  if (prefix != nullptr) {
    HAL_UART_Transmit(&g_uart, reinterpret_cast<const uint8_t*>(prefix),
                      static_cast<uint16_t>(strlen(prefix)), HAL_MAX_DELAY);
  }
  const int n = vsnprintf(g_buf, sizeof(g_buf), fmt, ap);
  if (n > 0) {
    const uint16_t len = static_cast<uint16_t>(n < static_cast<int>(sizeof(g_buf))
                                                   ? n
                                                   : sizeof(g_buf) - 1);
    HAL_UART_Transmit(&g_uart, reinterpret_cast<uint8_t*>(g_buf), len, HAL_MAX_DELAY);
  }
  HAL_UART_Transmit(&g_uart, reinterpret_cast<const uint8_t*>("\r\n"), 2, HAL_MAX_DELAY);
}

}

namespace logging {

void init() {
  g_uart.Instance = USART1;
  g_uart.Init.BaudRate = 115200;
  g_uart.Init.WordLength = UART_WORDLENGTH_8B;
  g_uart.Init.StopBits = UART_STOPBITS_1;
  g_uart.Init.Parity = UART_PARITY_NONE;
  g_uart.Init.Mode = UART_MODE_TX_RX;
  g_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  g_uart.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&g_uart);
}

void printf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  emit(nullptr, fmt, ap);
  va_end(ap);
}

void fatal(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  emit("FATAL: ", fmt, ap);
  va_end(ap);
  while (true) {
    __NOP();
  }
}

}
