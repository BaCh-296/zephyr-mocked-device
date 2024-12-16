/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

int main(void)
{
	LOG_INF("Zephyr Example Application %s\n", APP_VERSION_STRING);

	int ret;
	const struct device *sensor;
	struct sensor_value last_val = {0}, val;

	sensor = DEVICE_DT_GET(DT_NODELABEL(example_sensor));

	if (!device_is_ready(sensor))
	{
		LOG_ERR("Sensor not ready");
		return 0;
	}

	while (1)
	{

		ret = sensor_sample_fetch(sensor);
		if (ret < 0)
		{
			LOG_ERR("Could not fetch sample (%d)", ret);
			return 0;
		}

		ret = sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &val);
		if (ret < 0)
		{
			LOG_ERR("Could not get sample (%d)", ret);
			return 0;
		}

		if (last_val.val1 != val.val1)
		{
			LOG_INF("Temperature: %d",
					last_val.val1);
		}

		last_val = val;

		k_sleep(K_MSEC(2000));
	};

	return 0;
};
