/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <app_version.h>

LOG_MODULE_REGISTER(main);

static void timer_callback(struct k_timer *dummy);

static const struct device *sensor;

K_TIMER_DEFINE(my_timer, timer_callback, NULL);

static void timer_callback(struct k_timer *dummy)
{
	int ret;
	static struct sensor_value last_val = {0};
	struct sensor_value val;

	ret = sensor_sample_fetch(sensor);

	if (ret == -ENODATA)
	{
		LOG_INF("End of csv file.");
		k_timer_stop(&my_timer);
		return;
	}

	else if (ret < 0)
	{
		LOG_ERR("Could not fetch sample (%d)", ret);
		return;
	}

	ret = sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &val);

	if (ret == -ENODATA)
	{
		LOG_INF("No more sensor data.");
		k_timer_stop(&my_timer);
		return;
	}

	else if (ret < 0)
	{
		LOG_ERR("Could not get sample (%d)", ret);
		return;
	}

	if (last_val.val1 != val.val1)
	{
		LOG_INF("Temperature: %d", val.val1);
	}

	last_val = val;
}

int main()
{
	LOG_INF("Zephyr Example Application %s\n", APP_VERSION_STRING);

	sensor = DEVICE_DT_GET(DT_NODELABEL(example_sensor));

	while (!device_is_ready(sensor))
	{
		LOG_WRN("Sensor not ready");
		k_sleep(K_MSEC(100));
	}

	// Start the timer with the specified interval
	k_timer_start(&my_timer, K_NO_WAIT, K_MSEC(CONFIG_TIMER_INTERVAL_MS));

	return 0;
}
