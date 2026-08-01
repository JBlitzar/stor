#pragma once

#include <cstdint>

namespace board {

constexpr bool kCardDetectActiveLow = true;

enum class Led { Activity, Error };

void init();

void reset_phy();

bool phy_clock_alive();

void enable_usb_clocks();

bool card_present();

void led(Led which, bool on);

}
