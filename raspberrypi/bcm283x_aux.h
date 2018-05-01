#ifndef MICROPY_INCLUDED_RPI_BCM283X_AUX_H
#define MICROPY_INCLUDED_RPI_BCM283X_AUX_H

#include "bcm283x.h"

#define AUX_BASE    (0x215000 + IO_BASE)
#define AUX_REG(X)  ((X) + AUX_BASE)

#define AUX_IRQ     AUX_REG(0x00)
#define AUX_ENABLES AUX_REG(0x04)
#define AUX_MU      AUX_REG(0x40)
#define AUX_SPI0    AUX_REG(0x80)
#define AUX_SPI1    AUX_REG(0xC0)

typedef struct mini_uart_t {
    uint32_t IO;
    uint32_t IER;
    uint32_t IIR;
    uint32_t LCR;
    uint32_t MCR;
    uint32_t LSR;
    uint32_t MSR;
    uint32_t SCRATCH;
    uint32_t CNTL;
    uint32_t STAT;
    uint32_t BAUD;
} mini_uart_t;

typedef struct aux_spi_t {
    uint32_t CNTL0;
    uint32_t CNTL1;
    uint32_t STAT;
    uint32_t IO;
    uint32_t PEEK;
} aux_spi_t;

#endif // MICROPY_INCLUDED_RPI_BCM283X_AUX_H