#include <cstring>

#include "board.hpp"
#include "sd.hpp"
#include "tusb.h"

extern "C" {

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16],
                        uint8_t product_rev[4]) {
  (void)lun;
  memset(vendor_id, ' ', 8);
  memset(product_id, ' ', 16);
  memset(product_rev, ' ', 4);
  memcpy(vendor_id, "JBlitzar", 8);
  memcpy(product_id, "stor microSD", 12);
  memcpy(product_rev, "1.0 ", 4);
}

bool tud_msc_test_unit_ready_cb(uint8_t lun) {
  if (!sd::ready()) {
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
    return false;
  }
  return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
  (void)lun;
  *block_count = sd::block_count();
  *block_size = sd::kBlockSize;
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start,
                           bool load_eject) {
  (void)lun;
  (void)power_condition;
  if (load_eject && !start) {
    sd::deinit();
  }
  return true;
}

bool tud_msc_is_writable_cb(uint8_t lun) {
  (void)lun;
  return sd::ready();
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer,
                          uint32_t bufsize) {
  (void)lun;
  if ((offset % sd::kBlockSize) != 0 || (bufsize % sd::kBlockSize) != 0) return -1;

  board::led(board::Led::Activity, true);
  const bool ok = sd::read(static_cast<uint8_t*>(buffer), lba + offset / sd::kBlockSize,
                           bufsize / sd::kBlockSize);
  board::led(board::Led::Activity, false);

  return ok ? static_cast<int32_t>(bufsize) : -1;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer,
                           uint32_t bufsize) {
  (void)lun;
  if ((offset % sd::kBlockSize) != 0 || (bufsize % sd::kBlockSize) != 0) return -1;

  board::led(board::Led::Activity, true);
  const bool ok = sd::write(buffer, lba + offset / sd::kBlockSize,
                            bufsize / sd::kBlockSize);
  board::led(board::Led::Activity, false);

  return ok ? static_cast<int32_t>(bufsize) : -1;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer,
                        uint16_t bufsize) {
  (void)buffer;
  (void)bufsize;

  if (scsi_cmd[0] == 0x35) return 0;

  tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
  return -1;
}

}
