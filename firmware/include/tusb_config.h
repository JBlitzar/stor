#pragma once

#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)
#define CFG_TUSB_DEBUG 0

#define CFG_TUD_ENABLED 1

#define CFG_TUD_MAX_SPEED OPT_MODE_HIGH_SPEED

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))

#define CFG_TUD_ENDPOINT0_SIZE 64

#define CFG_TUD_MSC 1

// The class driver's staging buffer, and the buffer handed to sd::read/write.
// Raising this is the first lever if throughput ever matters.
#define CFG_TUD_MSC_EP_BUFSIZE 4096
