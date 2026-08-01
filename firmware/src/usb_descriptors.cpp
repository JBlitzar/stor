#include <cstring>

#include "stm32h7xx_hal.h"
#include "tusb.h"

namespace {

enum { kItfMsc = 0, kItfCount };

constexpr uint8_t kEpMscOut = 0x01;
constexpr uint8_t kEpMscIn = 0x81;
constexpr uint16_t kConfigLen = TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN;

const tusb_desc_device_t kDeviceDesc = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,  // per-interface
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x1209,
    .idProduct = 0x0001,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01,
};

const uint8_t kConfigDesc[] = {
    TUD_CONFIG_DESCRIPTOR(1, kItfCount, 0, kConfigLen, 0x00, 100),
    TUD_MSC_DESCRIPTOR(kItfMsc, 0, kEpMscOut, kEpMscIn, 512),
};

const char* const kStrings[] = {
    "JBlitzar",
    "stor",
};

uint16_t g_str_buf[33];

void serial_hex(char* out) {
  const uint32_t* uid = reinterpret_cast<const uint32_t*>(UID_BASE);
  static const char kHex[] = "0123456789ABCDEF";
  for (int w = 0; w < 3; ++w) {
    for (int n = 0; n < 8; ++n) {
      out[w * 8 + n] = kHex[(uid[w] >> (28 - n * 4)) & 0xF];
    }
  }
  out[24] = '\0';
}

}

extern "C" {

const uint8_t* tud_descriptor_device_cb() {
  return reinterpret_cast<const uint8_t*>(&kDeviceDesc);
}

const uint8_t* tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;
  return kConfigDesc;
}

const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;

  size_t count = 0;

  if (index == 0) {
    g_str_buf[1] = 0x0409;
    count = 1;
  } else if (index == 3) {
    char serial[25];
    serial_hex(serial);
    count = strlen(serial);
    for (size_t i = 0; i < count; ++i) {
      g_str_buf[1 + i] = serial[i];
    }
  } else {
    const size_t str_index = index - 1;
    if (str_index >= sizeof(kStrings) / sizeof(kStrings[0])) return nullptr;
    const char* str = kStrings[str_index];
    count = strlen(str);
    if (count > 31) count = 31;
    for (size_t i = 0; i < count; ++i) {
      g_str_buf[1 + i] = str[i];
    }
  }

  g_str_buf[0] = static_cast<uint16_t>((TUSB_DESC_STRING << 8) | (2 * count + 2));
  return g_str_buf;
}

}
