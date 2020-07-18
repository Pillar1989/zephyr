/*
 * Copyright (c) 2020 Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <device.h>
#include <zephyr.h>

#define LOG_LEVEL 11
#include <logging/log.h>
LOG_MODULE_REGISTER(greybus_test);

#include "bus.h"

extern int deferred_init(void);

void test_main(void) {

	int r;
	struct device *bus;
	struct bus_api *api;
	uint8_t *mnfb;
	size_t mnfb_size;
	size_t num_cports;

	r = deferred_init();
	if (r < 0) {
		LOG_ERR("deferred_init() failed: %d", r);
		goto out;
	}

	bus = device_get_binding("GREYBUS_0");
	if (NULL == bus) {
		LOG_ERR("failed to get GREYBUS_0 device");
		goto out;
	}

	api = (struct bus_api *) bus->driver_api;
	if (NULL == api) {
		LOG_ERR("failed to get GREYBUS_0 driver_api");
		goto out;
	}

	r = api->num_cports(bus);
	if (r < 0) {
		LOG_ERR("failed to get the number of cports");
		goto out;
	}
	num_cports = r;

	/* assume ownership of the dynamically allocated mnfb */
	r = api->gen_mnfb(bus, &mnfb, &mnfb_size);
	if (r < 0) {
		LOG_ERR("failed to get : %d", r);
		goto out;
	}

	/* ok to free the manifest intermediate representation now */ 
	api->fini(bus);

	LOG_INF("Greybus Manifest Binary (size: %u):", (unsigned) mnfb_size);
	for(size_t i = 0; i < mnfb_size; ++i) {
		if (0 == i % 4 && i != 0) {
			printk("\n");
		}
		printk("%02x ", mnfb[i]);
	}
	printk("\n");

	LOG_INF("there are %u cports\n", (unsigned)num_cports);

out:
    for(;;) {
    	k_sleep(K_MSEC(500));
    }

	/* finally, free the mnfb */
	free(mnfb);
	mnfb = NULL;
	mnfb_size = 0;
}
