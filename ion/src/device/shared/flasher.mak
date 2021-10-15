include ion/src/device/shared/usb/Makefile

ion_device_flasher_src += $(addprefix ion/src/device/shared/boot/, \
  isr.c \
  rt0.cpp \
  rt0_n0110.cpp \
)

ion_device_flasher_src += $(addprefix ion/src/device/shared/drivers/, \
  backlight.cpp \
  backlight_basic.cpp \
  base64.cpp \
  board_frequency_stm32f.cpp:-n0120 \
  board_frequency_stm32h.cpp:+n0120 \
  board_peripherals_clocks_n0110.cpp:+n0110 \
  board_peripherals_clocks_n0120.cpp:+n0120 \
  board_privileged.cpp \
  board_privileged_n0110.cpp:+n0110 \
  board_privileged_n0120.cpp:+n0120 \
  board_unprotected.cpp \
  display.cpp \
  external_flash.cpp \
  external_flash_qspi_n0110.cpp:+n0110 \
  external_flash_qspi_n0120.cpp:+n0120 \
  internal_flash.cpp \
  internal_flash_otp_n0110.cpp:+n0110 \
  internal_flash_otp_n0120.cpp:+n0120 \
  reset.cpp \
  serial_number.cpp \
  timing.cpp \
  usb.cpp \
  usb_gpio_n0110.cpp:+n0110 \
  usb_gpio_n0120.cpp:+n0120 \
)

ion_device_flasher_src += $(ion_device_dfu_src)