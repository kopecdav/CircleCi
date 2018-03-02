/*
 * tun_dri.h
 *
 *  Created on: 9. 2. 2018
 *      Author: martin
 */

#ifndef MBED_OS_FEATURES_FEATURE_LWIP_LWIP_INTERFACE_LWIP_ETH_ARCH_TARGET_STM_TUN_DRI_H_
#define MBED_OS_FEATURES_FEATURE_LWIP_LWIP_INTERFACE_LWIP_ETH_ARCH_TARGET_STM_TUN_DRI_H_
#include "mbed_interface.h"
#define TUN_DRIVER_OFFSET 4	//offset that is used by stack for tunneling driver (IPv6 packets starts with this offset)

#define TUN_DEBUG_ENABLED 0

#if TUN_DEBUG_ENABLED
		#define TUN_DEBUG(...);			printf(__VA_ARGS__);
#else
		#define TUN_DEBUG(...);			{}
#endif

#if defined (__cplusplus)
extern "C" {
#endif
int8_t tun_phy_rx(uint8_t *data, uint16_t len);
int8_t arm_tun_phy_device_register(uint8_t *mac_ptr, void (*driver_status_cb)(uint8_t, int8_t));


#if defined (__cplusplus)
}
#endif
#endif /* MBED_OS_FEATURES_FEATURE_LWIP_LWIP_INTERFACE_LWIP_ETH_ARCH_TARGET_STM_TUN_DRI_H_ */
