# stor firmware

TinyUSB + ST HAL firmware. It's mostly boilerplate and plumbing.


## Build

```sh
pio run
```

## Flash

Either obtain a CP2102 dongle, or follow the instructions in [`fpga`](fpga/).

Hold BOOT0, tap reset, release
BOOT0, then:

```sh
pio run -t upload
```

### Flash over USB (untested)

Plug the board, then the
same BOOT0 sequence, then:

```sh
dfu-util -a 0 -s 0x08000000:leave -D .pio/build/stor/firmware.bin
```

This will probably work, but it's not tested. be prepared to potentially flash over uart/swd.

I'll make this the default `pio run -t upload` behavior after it's been tested and verified.


## Console

USART1 on PA9 (TX) / PA10 (RX), 115200 8N1, 3.3 V.

```sh
pio device monitor
```
