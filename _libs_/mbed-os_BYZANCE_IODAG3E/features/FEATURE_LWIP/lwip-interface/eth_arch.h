/* EthernetInterface.h */
/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// Architecture specific Ethernet interface
// Must be implemented by each target

#ifndef ETHARCH_H_
#define ETHARCH_H_

#include "lwip/netif.h"
#define USE_NAT64

#ifdef __cplusplus
extern "C" {
#endif

#if DEVICE_EMAC
err_t emac_lwip_if_init(struct netif *netif);

#else /* DEVICE_EMAC */
void eth_arch_enable_interrupts(void);
void eth_arch_disable_interrupts(void);
err_t eth_arch_enetif_init(struct netif *netif);
#endif
#if defined(USE_NAT64)
int8_t arm_tun_phy_device_register(uint8_t *mac_ptr, void (*driver_status_cb)(uint8_t, int8_t));
#endif

#ifdef __cplusplus
}
#endif

#endif // #ifndef ETHARCHINTERFACE_H_
