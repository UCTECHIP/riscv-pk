/* See LICENSE for license details. */

ENTRY(_start)

SECTIONS {
  . = -0x41000000;

  .text.init : { *(.text.init) }
}
