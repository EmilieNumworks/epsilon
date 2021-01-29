#ifndef ION_DEVICE_KERNEL_BOOT_ISR_H
#define ION_DEVICE_KERNEL_BOOT_ISR_H

#include <boot/isr.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void isr_systick();
void keyboard_handler();
void tim2_handler();
void svcall_handler_as();
void svcall_handler(unsigned svcNumber, void * args[]);
void pendsv_handler_as();
void pendsv_handler(uint8_t * frameAddress, uint32_t excReturn);

#ifdef __cplusplus
}
#endif

#endif
