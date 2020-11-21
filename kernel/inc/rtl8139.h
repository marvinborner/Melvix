// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef RTL8139_H
#define RTL8139_H

#include <def.h>

#define RX_BUF_SIZE 0x2000

#define RTL8139_VENDOR_ID 0x10ec
#define RTL8139_DEVICE_ID 0x8139

#define RTL_ROK (1 << 0)
#define RTL_TOK (1 << 2)

#define RTL_PORT_MAC 0x00
#define RTL_PORT_MAR 0x08
#define RTL_PORT_TXSTAT 0x10
#define RTL_PORT_TXBUF 0x20
#define RTL_PORT_RBSTART 0x30
#define RTL_PORT_CMD 0x37
#define RTL_PORT_RXPTR 0x38
#define RTL_PORT_RXADDR 0x3A
#define RTL_PORT_IMR 0x3C
#define RTL_PORT_ISR 0x3E
#define RTL_PORT_TCR 0x40
#define RTL_PORT_RCR 0x44
#define RTL_PORT_RXMISS 0x4C
#define RTL_PORT_CONFIG 0x52

int rtl8139_install(void);
int rtl8139_installed(void);
void rtl8139_send_packet(void *data, u32 len);
u8 *rtl8139_get_mac(void);

#endif
