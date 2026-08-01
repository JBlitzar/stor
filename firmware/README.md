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


## Console

USART1 on PA9 (TX) / PA10 (RX), 115200 8N1, 3.3 V.

```sh
pio device monitor
```
