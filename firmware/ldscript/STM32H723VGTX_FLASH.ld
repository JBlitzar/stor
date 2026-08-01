/* STM32H723VGT6 - 1 MB flash, 564 KB RAM total.
 *
 * .data/.bss live in AXI SRAM rather than DTCM. Today's SD path is polled FIFO
 * reads, so placement is not yet load-bearing, but the SDMMC1 IDMA has no path
 * to DTCM and silently fails on 0x20000000 buffers. Defaulting every static to
 * AXI means switching to the IDMA variants needs no relayout. Stack stays in
 * DTCM, which the CPU reaches at zero wait states and no DMA engine needs.
 *
 * AXI SRAM is 320 KB only while option byte TCM_AXI_SHARED grants the shared
 * 192 KB block to AXI (the reset default). Reassigning it to ITCM shrinks this
 * region and this script must shrink with it.
 */

ENTRY(Reset_Handler)

MEMORY
{
  ITCMRAM (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
  FLASH   (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
  DTCMRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  RAM_AXI (xrw) : ORIGIN = 0x24000000, LENGTH = 320K
  SRAM1   (xrw) : ORIGIN = 0x30000000, LENGTH = 16K
  SRAM2   (xrw) : ORIGIN = 0x30004000, LENGTH = 16K
  SRAM4   (xrw) : ORIGIN = 0x38000000, LENGTH = 16K
  BKPSRAM (xrw) : ORIGIN = 0x38800000, LENGTH = 4K
}

_estack    = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);
_Min_Heap_Size  = 0;
_Min_Stack_Size = 0x2000;

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM       : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } >FLASH

  .preinit_array :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } >FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM_AXI AT> FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM_AXI

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + _Min_Heap_Size;
    . = ALIGN(8);
  } >RAM_AXI

  /* Stack lives at the top of DTCM; verify it fits. */
  ASSERT(_Min_Stack_Size <= LENGTH(DTCMRAM), "stack larger than DTCM")

  /DISCARD/ : { libc.a(*) libm.a(*) libgcc.a(*) }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
